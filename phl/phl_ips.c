/******************************************************************************
 *
 * Copyright(c) 2019 - 2020 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#include "phl_headers.h"
#include "phl_ps_api.h"
#include "phl_ips.h"
#include "test/phl_ps_dbg_cmd.h"

#ifdef CONFIG_PS_IPS

enum phl_ips_state {
	IPS_ST_DISABLE,
	IPS_ST_READY_TO_ENABLE,
	IPS_ST_ENABLE,
};

struct ips_obj {
	_os_mutex mux;
	struct phl_info_t *phl_info;

	/* cmd_q for saving requested command
	 * IPS enter: cmd q is empty
	 * IPS leave: cmd q is not empty
	 */
	struct list_head cmd_q;
	u32 cmd_cnt;

	/* IPS state */
	enum phl_ips_mode mode;

	/* IPS cap into power manager */
	u32 cap_token;

	/* start time when prepare to enter IPS */
	u32 start_time;

	/* req = 0: cmd q is empty
	 * req = 1: cmd q is not empty
	 */
	u8 req : 1;

	/* IPS state */
	u8 state : 2;
	u8 rsvd : 5;
};

struct ips_cmd_entry {
	struct list_head list;
	void (*cb)(void *phl, void *hdl, void *ctx, enum rtw_phl_status stat);
	void *ctx;
	u32 token;
	u8 enq : 1;
	u8 rsvd : 7;
	char type_s[50];
};
static void
_phl_ips_show_all_cmds(struct ips_obj *ips)
{
	struct ips_cmd_entry *pos = NULL;
	void *d = phl_to_drvpriv(ips->phl_info);

	_os_mutex_lock(d, &ips->mux);

	phl_list_for_loop(pos, struct ips_cmd_entry, &ips->cmd_q, list) {

		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
			"[IPS] %s Cmd => token: %u type : %s \n", __func__, pos->token, pos->type_s);
	}

	_os_mutex_unlock(d, &ips->mux);

}
static void
_phl_ips_dbg_dump_cmd_q(struct ips_obj *ips, u32 *used,
	char input[][MAX_ARGV], u32 input_num, char *output, u32 out_len)
{
	struct ips_cmd_entry *cmd = NULL;
	void *d = phl_to_drvpriv(ips->phl_info);
	u32 count = 0;

	PS_CNSL(out_len, *used, output + *used, out_len - *used,
		"Current existing commands:\n");

	_os_mutex_lock(d, &ips->mux);

	phl_list_for_loop(cmd, struct ips_cmd_entry, &ips->cmd_q, list) {

		PS_CNSL(out_len, *used, output + *used, out_len - *used,
			"Cmd => seq: %-11d type : %s \n",
			(int)cmd->token, cmd->type_s);
		count++;
	}
	_os_mutex_unlock(d, &ips->mux);

	if (0 == count) {

		PS_CNSL(out_len, *used, output + *used, out_len - *used, "Empty \n");
	}

}

static void
_phl_ips_cmd_exec_cb(struct ips_obj *ips, struct ips_cmd_entry *cmd, u32 status)
{
	cmd->cb(ips->phl_info, cmd, cmd->ctx, status);
}

static void
_phl_ips_insert_cmd(struct ips_obj *ips, struct ips_cmd_entry *cmd)
{
	void *d = phl_to_drvpriv(ips->phl_info);

	_os_mutex_lock(d, &ips->mux);
	list_add(&cmd->list, &ips->cmd_q);
	ips->cmd_cnt++;
	_os_mutex_unlock(d, &ips->mux);

	cmd->enq = true;
}

static u8
_phl_ips_remove_cmd(struct ips_obj *ips, u32 token)
{
	struct ips_cmd_entry *pos = NULL;
	void *d = phl_to_drvpriv(ips->phl_info);
	u8 find = false;

	_os_mutex_lock(d, &ips->mux);

	phl_list_for_loop(pos, struct ips_cmd_entry, &ips->cmd_q, list) {
		if (pos->token == token) {
			PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
				"[IPS] %s: find matched cmd! type (%s)\n", __func__, pos->type_s);
			list_del(&pos->list);
			pos->enq = false;
			ips->cmd_cnt--;
			_os_kmem_free(d, pos, sizeof(*pos));
			find = true;
			break;
		}
	}

	_os_mutex_unlock(d, &ips->mux);

	return find;
}

static void
_phl_ips_judge_pwr_req(struct ips_obj *ips)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	void *d = phl_to_drvpriv(ips->phl_info);
	u8 new_req;

	_os_mutex_lock(d, &ips->mux);

	if (list_empty(&ips->cmd_q)) {
		new_req = false;
	} else {
		new_req = true;
	}

	_os_mutex_unlock(d, &ips->mux);

	if (ips->req != new_req) {

		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
			"[IPS] %s: ips req %d -> %d\n", __func__,
			ips->req, new_req);
		ips->req = new_req;
	}
}

static void
_phl_ips_review_cmd(struct ips_obj *ips, struct ips_cmd_entry *cmd,
			enum rtw_phl_status status)
{
	if (cmd->cb) {
		_phl_ips_cmd_exec_cb(ips, cmd, status);
	}

	_phl_ips_insert_cmd(ips, cmd);
}

static void
_phl_ips_init_obj(struct ips_obj *ips, struct phl_info_t *phl_info)
{
	void *d = phl_to_drvpriv(phl_info);

	/* init obj local use variable */
	ips->phl_info = phl_info;
	ips->mode = IPS_MODE_PWR_DOWN;
	ips->state = IPS_ST_DISABLE;
	ips->cmd_cnt = 0;
	ips->req = 0;
	ips->start_time = _os_get_cur_time_ms();

	INIT_LIST_HEAD(&ips->cmd_q);

	_os_mutex_init(d, &ips->mux);
}

static void
_phl_ips_deinit_obj(struct ips_obj *ips, void *d)
{
	struct ips_cmd_entry *cmd_pos = NULL;
	struct ips_cmd_entry *cmd_n = NULL;

	_os_mutex_lock(d, &ips->mux);

	phl_list_for_loop_safe(cmd_pos, cmd_n, struct ips_cmd_entry,
				&ips->cmd_q, list) {

		list_del(&cmd_pos->list);
		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_, "[IPS] del cmd %p\n", cmd_pos);
		_os_kmem_free(d, cmd_pos, sizeof(*cmd_pos));
	}

	_os_mutex_unlock(d, &ips->mux);
	_os_mutex_deinit(d, &ips->mux);
}

static void
_phl_ips_dump_obj(void *fsm, char *s, int *sz)
{
	/* nothing to do for now */
}

/* Create inactive power save object
 * @fsm: FSM main structure which created by phl_ips_new_fsm()
 * @phl_info: private data structure to invoke hal/phl function
 *
 * return
 * ips_obj: structure of command object (Do NOT expose)
 */
static struct ips_obj *
_phl_ips_new_obj(struct phl_info_t *phl_info)
{
	struct ips_obj *ips;
	void *d = phl_to_drvpriv(phl_info);

	ips = (struct ips_obj *)_os_kmem_alloc(d, sizeof(*ips));
	if (ips == NULL) {
		PHL_ERR("ips: malloc obj fail.\n");
		return NULL;
	}

	_os_mem_set(d, ips, 0, sizeof(*ips));

	return ips;
}

/* Free ips object */
/* @ips_obj: local created ips object
 */
static void _phl_ips_free_obj(struct ips_obj *ips, void *d)
{
	/* free ips_obj */
	_os_kmem_free(d, ips, sizeof(*ips));
}

/* For EXTERNAL application to initialize inactive power save
 * @ips_obj: local created ips object
 * @phl_info: private data structure to invoke hal/phl function
 *
 * return: rtw_phl_status
 */
enum rtw_phl_status
phl_ips_init(void **ips_obj, struct phl_info_t *phl_info)
{
	*ips_obj = _phl_ips_new_obj(phl_info);
	if (*ips_obj == NULL) {
		return RTW_PHL_STATUS_FAILURE;
	}

	_phl_ips_init_obj(*ips_obj, phl_info);

	return RTW_PHL_STATUS_SUCCESS;
}

/* For EXTERNAL application to deinitialize inactive power save
 *  * @ips_obj: local created ips object
 *
 */
void phl_ips_deinit(void *ips_obj)
{
	struct ips_obj *ips = (struct ips_obj *)ips_obj;
	void *d;

	if (ips == NULL)
		return;

	d = phl_to_drvpriv(ips->phl_info);
	_phl_ips_deinit_obj(ips, d);
	_phl_ips_free_obj(ips, d);
	ips = NULL;
}

void phl_ips_periodic_chk(void *ips_obj)
{
	struct ips_obj *ips = (struct ips_obj *)ips_obj;
	struct rtw_phl_com_t *phl_com = ips->phl_info->phl_com;
	struct rtw_ps_cap_t *ps_cap = &phl_com->dev_sw_cap.ps_cap;
	u32 dif_time = _os_get_cur_time_ms() - ips->start_time;

	if(ips->state == IPS_ST_READY_TO_ENABLE &&
		dif_time > ps_cap->ips_defer_time) {

		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_, "IPS enter !!!\n");
		phl_ps_issue_pm_cap(ips->phl_info, &ips->cap_token,
					HWPS_CAP_PWROFF);
		ips->state = IPS_ST_ENABLE;
	}
}

/* For EXTERNAL application to issue ips command (expose)
 * @ips_obj: local created ips object
 * @ntfy: parameter for issue cmd
 *
 * return: rtw_phl_status
 */
enum rtw_phl_status
phl_ips_issue_cmd(void *ips_obj, struct ps_ntfy *ntfy)
{
	struct ips_obj *ips = (struct ips_obj *)ips_obj;
	void *d = phl_to_drvpriv(ips->phl_info);
	struct ips_cmd_entry *cmd = NULL;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;

	do {
		cmd = _os_kmem_alloc(d, sizeof(*cmd));
		if (cmd == NULL) {
			PHL_ERR("%s: alloc cmd fail.\n", __func__);
			pstatus = RTW_PHL_STATUS_RESOURCE;
			break;
		}

		INIT_LIST_HEAD(&cmd->list);
		cmd->cb = ntfy->cb;
		cmd->ctx = ntfy->ctx;
		cmd->token = ntfy->token;
		_os_mem_cpy(d, cmd->type_s, ntfy->type_s, 50);

		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
			"[IPS] %s: token (%d), type (%s), current status (%s) \n",
			__func__, ntfy->token, cmd->type_s,
			ips->req == 1 ? "already left IPS" : "ready to leave IPS");

		ips->req = true;

		if (ips->state == IPS_ST_ENABLE) {
			PHL_TRACE(COMP_PHL_PS, _PHL_INFO_, "IPS leave !!!\n");
			phl_ps_cancel_pm_cap(ips->phl_info, ips->cap_token);
		}
		ips->state = IPS_ST_DISABLE;

		_phl_ips_review_cmd(ips, cmd, RTW_PHL_STATUS_SUCCESS);

	} while (0);

	phl_ps_ntfy_completion(ips->phl_info, ntfy);

	return pstatus;
}

/* For EXTERNAL application to cancel ips command (expose)
 * @ips_obj: local created ips object
 * @ntfy: parameter for cancel cmd
 *
 */
void phl_ips_cancel_cmd(void *ips_obj, struct ps_ntfy *ntfy)
{
	struct ips_obj *ips = (struct ips_obj *)ips_obj;
	struct rtw_phl_com_t *phl_com = ips->phl_info->phl_com;
	struct rtw_ps_cap_t *ps_cap = &phl_com->dev_sw_cap.ps_cap;
	u32 token = ntfy->token;

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[IPS] %s: token (%d)\n", __func__, ntfy->token);

	do {
		if (token == IPS_RSVD_TOKEN) {
			PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
				"[IPS] Reserved token for first trigger.\n");
		} else if (_phl_ips_remove_cmd(ips, token) == false) {
			PHL_ERR("%s: cannot find token(%d)!\n", __func__, token);
			break;
		}

		_phl_ips_judge_pwr_req(ips);

		if (ps_cap->ips_en == 0) {
			PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
				"[IPS] IPS is disabled (%d).\n", ps_cap->ips_en);
			break;
		}

		if (!ips->req && ips->state == IPS_ST_DISABLE) {
			ips->start_time = _os_get_cur_time_ms();
			ips->state = IPS_ST_READY_TO_ENABLE;
		}

	} while (0);

	_phl_ips_show_all_cmds(ips);

	phl_ps_ntfy_completion(ips->phl_info, ntfy);
}
void phl_ips_dbg_dump_obj(void *ips_obj, u32 *used, char input[][MAX_ARGV],
			u32 input_num, char *output, u32 out_len)
{
	struct ips_obj *ips = (struct ips_obj *)ips_obj;

	PS_CNSL(out_len, *used, output + *used, out_len - *used,
		"========== PHL IPS Info ==========\n");

	_phl_ips_dbg_dump_cmd_q(ips, used, input, input_num,
					output, out_len);
}

#endif /* CONFIG_PS_IPS */

