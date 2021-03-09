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
#include "phl_lps.h"
#include "test/phl_ps_dbg_cmd.h"

#ifdef CONFIG_PS_LPS

struct lps_obj {
	_os_mutex mux;
	struct phl_info_t *phl_info;

	/* cmd_q for saving requested command
	 * LPS enter: cmd q is empty
	 * LPS leave: cmd q is not empty
	 */
	struct list_head cmd_q;
	u32 cmd_cnt;

	/* role_q for active role */
	struct list_head role_q;
	u32 role_cnt;

	/* token for req power */
	u32 pwr_token;

	/* req = 0: cmd q is empty
	 * req = 1: cmd q is not empty
	 */
	u8 req : 1;

	/* req = 0: not req power from pm
	 * req = 1: req power from pm
	 */
	u8 pwr_req : 1;
	u8 rsvd : 6;
};

struct lps_cmd_entry {
	struct list_head list;
	void (*cb)(void *phl, void *hdl, void *ctx, enum rtw_phl_status stat);
	void *ctx;
	u32 token;
	u8 enq : 1;
	u8 rsvd : 7;
	char type_s[50];
};

struct lps_role_entry {
	struct list_head list;

	u16 macid;
	u32 cap;
	u8 awake_interval;

	/* Another reason to decide to enter or leave LPS */
	u8 (*chk_lps_state_chg_cb)(void *phl, u16 macid, u8 state);
	u8 listen_bcn_mode : 2;
	u8 smart_ps_mode : 2;
	u8 enq : 1;
	u8 state : 3;
	u32 cap_token;
	u32 null_token;

	/* connected time */
	u32 connected_time;
	bool time_arrived_en_lps;

	void (*cb)(void *phl, void *hdl, void *ctx, enum rtw_phl_status stat);
	void *ctx;
};

static void
_phl_lps_show_all_cmds(struct lps_obj *lps)
{
	struct lps_cmd_entry *pos = NULL;
	void *d = phl_to_drvpriv(lps->phl_info);

	_os_mutex_lock(d, &lps->mux);

	phl_list_for_loop(pos, struct lps_cmd_entry, &lps->cmd_q, list) {

		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
			"[LPS] %s Cmd => token: %u type : %s \n", __func__,
				pos->token, pos->type_s);
	}

	_os_mutex_unlock(d, &lps->mux);

}

static void
_phl_lps_dbg_dump_cmd_q(struct lps_obj *lps, u32 *used,
	char input[][MAX_ARGV], u32 input_num, char *output, u32 out_len)
{
	struct lps_cmd_entry *cmd = NULL;
	void *d = phl_to_drvpriv(lps->phl_info);
	u32 count = 0;

	PS_CNSL(out_len, *used, output + *used, out_len - *used,
		"Current existing commands: \n");

	_os_mutex_lock(d, &lps->mux);

	phl_list_for_loop(cmd, struct lps_cmd_entry, &lps->cmd_q, list) {

		PS_CNSL(out_len, *used, output + *used, out_len - *used,
			"Cmd => seq: %-11d type : %s \n",
			(int)cmd->token, cmd->type_s);
		count++;
	}

	_os_mutex_unlock(d, &lps->mux);


	if (0 == count) {

		PS_CNSL(out_len, *used, output + *used, out_len - *used, "Empty \n");
	}
}

static void
_phl_lps_dbg_dump_role(struct lps_obj *lps, struct lps_role_entry *role,
			u32 *used, char input[][MAX_ARGV], u32 input_num,
			char *output, u32 out_len)
{
	struct rtw_phl_stainfo_t *stainfo = NULL;
	struct rtw_hal_stainfo_t *hal_sta = NULL;
	struct rtw_chan_def *chan;

	PS_CNSL(out_len, *used, output + *used, out_len - *used,
		"%-8s macid: %d, cap: %x, ps mode: %d, RLBM: %d, awake interval: %d\n",
		"Role =>", role->macid, (unsigned int)role->cap, role->smart_ps_mode,
		role->listen_bcn_mode, role->awake_interval);

	stainfo = rtw_phl_get_stainfo_by_macid(lps->phl_info, role->macid);
	if (stainfo == NULL)
		return;

	hal_sta = stainfo->hal_sta;
	if (hal_sta == NULL)
		return;

	PS_CNSL(out_len, *used, output + *used, out_len - *used,
		"%-8s active: %d, rssi: %d, aid: %d, state: %s",
		"", stainfo->active,
		PHL_TRANS_2_RSSI(hal_sta->rssi_stat.rssi),
		stainfo->aid,
		phl_ps_id_to_str(PS_STR_LPS_STATE, (u32)role->state));

	chan = &stainfo->chandef;
	PS_CNSL(out_len, *used, output + *used, out_len - *used,
		", center ch: %d, bw: %d, band: %d\n",
		chan->center_ch, chan->bw, chan->band);
}

static void
_phl_lps_dbg_dump_all_role(struct lps_obj *lps, u32 *used,
	char input[][MAX_ARGV], u32 input_num, char *output, u32 out_len)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	void *d = phl_to_drvpriv(lps->phl_info);
	struct lps_role_entry *pos = NULL;
	struct lps_role_entry *next;

	_os_mutex_lock(d, &lps->mux);

	phl_list_for_loop_safe(pos, next, struct lps_role_entry,
				&lps->role_q, list) {

		_phl_lps_dbg_dump_role(lps, pos, used, input, input_num,
					output, out_len);
	}

	_os_mutex_unlock(d, &lps->mux);

}

static void
_phl_lps_cmd_exec_cb(struct lps_obj *lps, struct lps_cmd_entry *cmd, u32 status)
{
	cmd->cb(lps->phl_info, cmd, cmd->ctx, status);
}

static void
_phl_lps_insert_cmd(struct lps_obj *lps, struct lps_cmd_entry *cmd)
{
	void *d = phl_to_drvpriv(lps->phl_info);

	_os_mutex_lock(d, &lps->mux);
	list_add(&cmd->list, &lps->cmd_q);
	lps->cmd_cnt++;
	_os_mutex_unlock(d, &lps->mux);

	cmd->enq = true;
}

static u8
_phl_lps_remove_cmd(struct lps_obj *lps, u32 token)
{
	struct lps_cmd_entry *pos = NULL;
	void *d = phl_to_drvpriv(lps->phl_info);
	u8 find = false;

	_os_mutex_lock(d, &lps->mux);

	phl_list_for_loop(pos, struct lps_cmd_entry, &lps->cmd_q, list) {
		if (pos->token == token) {
			PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
				"[LPS] %s: find matched cmd! type (%s)\n",
				__func__, pos->type_s);
			list_del(&pos->list);
			pos->enq = false;
			lps->cmd_cnt--;
			_os_kmem_free(d, pos, sizeof(*pos));
			find = true;
			break;
		}
	}

	_os_mutex_unlock(d, &lps->mux);

	return find;
}

static u8
_phl_lps_is_role_exist(struct lps_obj *lps, u16 macid)
{
	struct lps_role_entry *pos = NULL;
	void *d = phl_to_drvpriv(lps->phl_info);

	_os_mutex_lock(d, &lps->mux);

	phl_list_for_loop(pos, struct lps_role_entry, &lps->role_q, list) {
		if (pos->macid == macid) {
			PHL_ERR("%s: the same mac id(%d) in role queue.\n",
				__func__, pos->macid);
			_os_mutex_unlock(d, &lps->mux);
			return true;
		}
	}

	_os_mutex_unlock(d, &lps->mux);

	return false;
}

static u8
_phl_lps_is_role_idle(struct lps_obj *lps)
{
	struct lps_role_entry *pos = NULL;
	void *d = phl_to_drvpriv(lps->phl_info);
	u8 is_idle = false;

	_os_mutex_lock(d, &lps->mux);

	phl_list_for_loop(pos, struct lps_role_entry, &lps->role_q, list) {
		if (pos->state == LPS_ROLE_ST_IDLE) {
			is_idle = true;
			break;
		}
	}

	_os_mutex_unlock(d, &lps->mux);

	return is_idle;
}

static void
_phl_lps_init_role_entry(struct lps_role_entry *role, struct ps_ntfy *ntfy)
{
	struct lps_role_info_param *role_info;

	role_info = &ntfy->u.lps_role;
	INIT_LIST_HEAD(&role->list);
	role->macid = role_info->macid;
	role->cap = role_info->cap;
	role->awake_interval = role_info->awake_interval;
	role->listen_bcn_mode = role_info->listen_bcn_mode;
	role->smart_ps_mode = role_info->smart_ps_mode;
	role->chk_lps_state_chg_cb = role_info->chk_lps_state_chg_cb;
	role->state = LPS_ROLE_ST_IDLE;
	role->connected_time = _os_get_cur_time_ms();
	role->time_arrived_en_lps = false;
}

static enum rtw_phl_status
_phl_lps_insert_role(struct lps_obj *lps, struct lps_role_entry *role)
{
	void *d = phl_to_drvpriv(lps->phl_info);

	_os_mutex_lock(d, &lps->mux);

	list_add(&role->list, &lps->role_q);
	lps->role_cnt++;
	role->enq = true;

	_os_mutex_unlock(d, &lps->mux);

	return RTW_PHL_STATUS_SUCCESS;
}

static void
_phl_lps_remove_role(struct lps_obj *lps, struct lps_role_entry *role)
{
	void *d = phl_to_drvpriv(lps->phl_info);

	if (role->cb) {
		role->cb(lps->phl_info, role, role->ctx, RTW_PHL_STATUS_SUCCESS);
	}

	_os_mutex_lock(d, &lps->mux);

	list_del(&role->list);
	lps->role_cnt--;
	role->enq = false;
	_os_kmem_free(d, role, sizeof(*role));

	_os_mutex_unlock(d, &lps->mux);

}

static void
_phl_lps_issue_pwr_req(struct lps_obj *lps)
{
	if(lps->pwr_req) {
		return;
	}

	phl_ps_issue_pm_cmd(lps->phl_info, &lps->pwr_token, PWRCMD_COMP_IO_RF,
				PHL_PS_NTFY_TYPE_LPS_MODULE);
	lps->pwr_req = true;
}

static void
_phl_lps_cancel_pwr_req(struct lps_obj *lps)
{
	/* no powr req currently */
	if(!lps->pwr_req) {
		return;
	}

	/* someone need pwr req */
	if(lps->req) {
		return;
	}

	if(_phl_lps_is_role_idle(lps)) {
		return;
	}

	phl_ps_cancel_pm_cmd(lps->phl_info, lps->pwr_token);
	lps->pwr_req = false;
}

static u8
_phl_lps_judge_pwr_req(struct lps_obj *lps)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	void *d = phl_to_drvpriv(lps->phl_info);
	u8 new_req;
	u8 pwr_req = false;

	_os_mutex_lock(d, &lps->mux);

	if (list_empty(&lps->cmd_q)) {
		new_req = false;
	} else {
		new_req = true;
	}

	_os_mutex_unlock(d, &lps->mux);

	if (lps->req != new_req) {

		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
			"[LPS] %s: %d -> %d\n", __func__, lps->req, new_req);
		pwr_req = true;
		lps->req = new_req;
	}

	return pwr_req;
}

static enum rtw_phl_status
_phl_lps_cfg_role(struct lps_obj *lps, struct lps_role_entry *role, u8 lps_en)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	struct rtw_hal_lps_info lps_info;
#ifdef RTW_WKARD_LPS_ROLE_CONFIG
	struct rtw_wifi_role_t *wrole = NULL;
	struct rtw_phl_stainfo_t *sta = NULL;

	sta = rtw_phl_get_stainfo_by_macid(lps->phl_info, role->macid);
	if (sta != NULL) {
		wrole = sta->wrole;
	} else
		PHL_ERR("%s sta is NULL\n", __func__);
#endif

	if(lps_en) {
		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
			"[LPS] Enter: macid = %d\n", role->macid);
		#ifdef RTW_WKARD_LPS_ROLE_CONFIG
		phl_role_suspend_unused_role(lps->phl_info, wrole);
		#endif
	} else {
		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
			"[LPS] Leave: macid = %d\n", role->macid);
		#ifdef RTW_WKARD_LPS_ROLE_CONFIG
		phl_role_recover_unused_role(lps->phl_info, wrole);
		#endif
	}

	lps_info.en_lps = lps_en;
	lps_info.macid = role->macid;
	lps_info.listen_bcn_mode = role->listen_bcn_mode;
	lps_info.awake_interval = role->awake_interval;
	lps_info.smart_ps_mode = role->smart_ps_mode;

	if (rtw_hal_ps_lps_cfg(lps->phl_info->hal, &lps_info) !=
		RTW_HAL_STATUS_SUCCESS) {
		PHL_ERR("%s: mac id(%d) cfg lps(%d) fail.\n", __func__,
			role->macid, lps_en);
		status = RTW_PHL_STATUS_FAILURE;
	}

	return status;
}

static void
_phl_lps_review_cmd(struct lps_obj *lps, struct lps_cmd_entry *cmd,
			enum rtw_phl_status status)
{
	if (cmd->cb) {
		_phl_lps_cmd_exec_cb(lps, cmd, status);
	}

	_phl_lps_insert_cmd(lps, cmd);
}

static u8
_phl_lps_delay_is_exceeded(struct lps_obj *lps, struct lps_role_entry *role)
{
	struct rtw_phl_com_t *phl_com = lps->phl_info->phl_com;
	struct rtw_ps_cap_t *ps_cap = &phl_com->dev_sw_cap.ps_cap;
	u32 cur_time = _os_get_cur_time_ms();
	u32 dif_time;

	if (role->time_arrived_en_lps == true)
		return true;

	if (cur_time >= role->connected_time) {
		dif_time = cur_time - role->connected_time;
	} else {
		dif_time = cur_time + (RTW_U32_MAX - role->connected_time);
	}

	if(dif_time > ps_cap->lps_defer_time) {
		role->time_arrived_en_lps = true;
		return true;
	} else {
		return false;
	}
}

static u8
_phl_lps_chk_condition(struct lps_obj *lps, struct lps_role_entry *role)
{
	struct rtw_phl_com_t *phl_com = lps->phl_info->phl_com;
	struct rtw_ps_cap_t *ps_cap = &phl_com->dev_sw_cap.ps_cap;
	u8 new_state = LPS_ROLE_ST_IDLE;

	do {
		if(ps_cap->lps_en == 0) {
			break;
		}

		if(lps->req) {
			break;
		}

		if(!_phl_lps_delay_is_exceeded(lps, role)) {
			break;
		}

		if(role->chk_lps_state_chg_cb) {
			new_state = role->chk_lps_state_chg_cb(lps->phl_info,
					role->macid, role->state);
		}

	} while (0);

	return new_state;
}

static enum rtw_phl_status
_phl_lps_update_role_state(struct lps_obj *lps, struct lps_role_entry *role)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	u8 new_state;

	new_state = _phl_lps_chk_condition(lps, role);
	if (new_state == role->state)
		return RTW_PHL_STATUS_SUCCESS;

	if (new_state == LPS_ROLE_ST_LPS) {

		if(!lps->pwr_req)
			_phl_lps_issue_pwr_req(lps);

		if (_phl_lps_cfg_role(lps, role, true)
			== RTW_PHL_STATUS_SUCCESS) {
			role->state = LPS_ROLE_ST_LPS;
		} else {
			status = RTW_PHL_STATUS_FAILURE;
		}

	} else if (new_state == LPS_ROLE_ST_IDLE) {

		if(!lps->pwr_req)
			_phl_lps_issue_pwr_req(lps);

		if (_phl_lps_cfg_role(lps, role, false)
			== RTW_PHL_STATUS_SUCCESS) {
			role->state = LPS_ROLE_ST_IDLE;
		} else {
			status = RTW_PHL_STATUS_FAILURE;
		}
	}

	return status;
}

static enum rtw_phl_status
_phl_lps_review_all_role(struct lps_obj *lps)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	void *d = phl_to_drvpriv(lps->phl_info);
	struct lps_role_entry *pos = NULL;
	struct lps_role_entry *next;

	_os_mutex_lock(d, &lps->mux);

	phl_list_for_loop_safe(pos, next, struct lps_role_entry,
				&lps->role_q, list) {

		status = _phl_lps_update_role_state(lps, pos);
		if (status != RTW_PHL_STATUS_SUCCESS) {
			break;
		}
	}

	_os_mutex_unlock(d, &lps->mux);

	return status;
}

static struct lps_role_entry *
_phl_lps_get_role(struct lps_obj *lps, u16 macid)
{
	struct lps_role_entry *pos = NULL;
	void *d = phl_to_drvpriv(lps->phl_info);
	u8 pwr_req = false;
	u8 find = false;

	_os_mutex_lock(d, &lps->mux);

	phl_list_for_loop(pos, struct lps_role_entry, &lps->role_q, list) {
		if (pos->macid == macid) {
			find = true;
			break;
		}
	}

	_os_mutex_unlock(d, &lps->mux);

	if (find) {
		return pos;
	} else {
		PHL_ERR("%s: mac id(%d) not found.\n", __func__, macid);
		return NULL;
	}
}

static void
_phl_lps_init_obj(struct lps_obj *lps, struct phl_info_t *phl_info)
{
	void *d = phl_to_drvpriv(phl_info);

	/* init obj local use variable */
	lps->phl_info = phl_info;
	lps->cmd_cnt = 0;
	lps->role_cnt = 0;
	lps->req = 0;
	lps->pwr_req = false;

	INIT_LIST_HEAD(&lps->cmd_q);
	INIT_LIST_HEAD(&lps->role_q);

	_os_mutex_init(d, &lps->mux);
}

static void
_phl_lps_deinit_obj(struct lps_obj *lps, void *d)
{
	struct lps_cmd_entry *cmd_pos = NULL;
	struct lps_cmd_entry *cmd_n = NULL;
	struct lps_role_entry *role_pos = NULL;
	struct lps_role_entry *role_n = NULL;

	_os_mutex_lock(d, &lps->mux);

	phl_list_for_loop_safe(cmd_pos, cmd_n, struct lps_cmd_entry,
				&lps->cmd_q, list) {

		list_del(&cmd_pos->list);
		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_, "[LPS] del cmd %p\n",
			cmd_pos);
		_os_kmem_free(d, cmd_pos, sizeof(*cmd_pos));
	}

	phl_list_for_loop_safe(role_pos, role_n, struct lps_role_entry,
				&lps->role_q, list) {

		list_del(&role_pos->list);
		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_, "[LPS] del role %p\n",
			role_pos);
		_os_kmem_free(d, role_pos, sizeof(*role_pos));
	}

	_os_mutex_unlock(d, &lps->mux);
	_os_mutex_deinit(d, &lps->mux);
}

/* Create legacy power save object
 * @fsm: FSM main structure which created by phl_lps_new_fsm()
 * @phl_info: private data structure to invoke hal/phl function
 *
 * return
 * lps_obj: structure of command object (Do NOT expose)
 */
static struct lps_obj *
_phl_lps_new_obj(struct phl_info_t *phl_info)
{
	struct lps_obj *lps;
	void *d = phl_to_drvpriv(phl_info);

	lps = (struct lps_obj *)_os_kmem_alloc(d, sizeof(*lps));
	if (lps == NULL) {
		PHL_ERR("lps: malloc obj fail.\n");
		return NULL;
	}

	_os_mem_set(d, lps, 0, sizeof(*lps));

	return lps;
}

/* Destory lps object */
/* @lps: local created lps object
 */
static void _phl_lps_free_obj(struct lps_obj *lps, void *d)
{
	/* free lps_obj */
	_os_kmem_free(d, lps, sizeof(*lps));
}

/* For EXTERNAL application to initialize legacy power save
 * @phl_info: private data structure to invoke hal/phl function
 *
 * return: rtw_phl_status
 */
enum rtw_phl_status
phl_lps_init(void **lps_obj, struct phl_info_t *phl_info)
{
	*lps_obj = _phl_lps_new_obj(phl_info);
	if (*lps_obj == NULL) {
		return RTW_PHL_STATUS_FAILURE;
	}

	_phl_lps_init_obj(*lps_obj, phl_info);

	return RTW_PHL_STATUS_SUCCESS;
}

/* For EXTERNAL application to deinitialize legacy power save
 * @phl_info: private data structure to invoke hal/phl function
 *
 */
void phl_lps_deinit(void *lps_obj)
{
	struct lps_obj *lps = (struct lps_obj *)lps_obj;
	void *d;

	if (lps == NULL)
		return;

	d = phl_to_drvpriv(lps->phl_info);
	_phl_lps_deinit_obj(lps, d);
	_phl_lps_free_obj(lps, d);
	lps = NULL;
}

void phl_lps_periodic_chk(void *lps_obj)
{
	struct lps_obj *lps = (struct lps_obj *)lps_obj;

	_phl_lps_review_all_role(lps);
	_phl_lps_cancel_pwr_req(lps);
}

u8 phl_lps_is_ongoing(void *lps_obj)
{
	struct lps_obj *lps = (struct lps_obj *)lps_obj;
	void *d = phl_to_drvpriv(lps->phl_info);
	struct lps_role_entry *pos = NULL;
	u8 is_lps_on = false;

	_os_mutex_lock(d, &lps->mux);

	phl_list_for_loop(pos, struct lps_role_entry, &lps->role_q, list) {

		if (pos->state == LPS_ROLE_ST_LPS) {
			is_lps_on = true;
			break;
		}
	}

	_os_mutex_unlock(d, &lps->mux);

	return is_lps_on;
}

/* For EXTERNAL application to issue lps command (expose)
 * @lps_obj: local created lps object
 * @ntfy: parameter for issue cmd
 *
 * return: rtw_phl_status
 */
enum rtw_phl_status
phl_lps_issue_cmd(void *lps_obj, struct ps_ntfy *ntfy)
{
	struct lps_obj *lps = (struct lps_obj *)lps_obj;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	void *d = phl_to_drvpriv(lps->phl_info);
	struct lps_cmd_entry *cmd = NULL;
	u8 pwr_req = false;

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
			"[LPS] %s: token (%d), type (%s), current status (%s)\n",
			__func__, ntfy->token, cmd->type_s,
			lps->req == 1 ? "already left LPS" : "ready to leave LPS");

		lps->req = true;

		_phl_lps_issue_pwr_req(lps);
		pstatus = _phl_lps_review_all_role(lps);
		_phl_lps_review_cmd(lps, cmd, pstatus);

	} while (0);

	phl_ps_ntfy_completion(lps->phl_info, ntfy);

	return pstatus;
}

/* For EXTERNAL application to cancel lps command (expose)
 * @lps_obj: local created lps object
 * @ntfy: parameter for cancel cmd
 *
 */
void phl_lps_cancel_cmd(void *lps_obj, struct ps_ntfy *ntfy)
{
	struct lps_obj *lps = (struct lps_obj *)lps_obj;

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[LPS] %s: token (%d)\n", __func__, ntfy->token);

	_phl_lps_remove_cmd(lps, ntfy->token);

	if(_phl_lps_judge_pwr_req(lps)) {
		/* power req 1 -> 0 */
		_phl_lps_review_all_role(lps);
		_phl_lps_cancel_pwr_req(lps);
	}

	_phl_lps_show_all_cmds(lps);

	phl_ps_ntfy_completion(lps->phl_info, ntfy);
}

/* For EXTERNAL application to add role to use legacy power save (expose)
 * @lps_obj: local created lps object
 * @ntfy: parameter for add role
 *
 * return: rtw_phl_status
 */
enum rtw_phl_status
phl_lps_add_role(void *lps_obj, struct ps_ntfy *ntfy)
{
	struct lps_obj *lps = (struct lps_obj *)lps_obj;
	struct lps_role_entry *role;
	void *d = phl_to_drvpriv(lps->phl_info);
	struct rtw_pkt_ofld_null_info null_info = {0};
	struct rtw_phl_stainfo_t *phl_sta = NULL;

	if (_phl_lps_is_role_exist(lps, ntfy->u.lps_role.macid)) {
		return RTW_PHL_STATUS_FAILURE;
	}

	role = _os_kmem_alloc(d, sizeof(*role));
	if (role == NULL) {
		PHL_ERR("%s: alloc role fail.\n", __func__);
		return RTW_PHL_STATUS_RESOURCE;
	}

	rtw_hal_cfg_fw_ps_log(lps->phl_info->hal, true);

	_phl_lps_issue_pwr_req(lps);

	phl_sta = rtw_phl_get_stainfo_by_macid(lps->phl_info, role->macid);

	_os_mem_cpy(d, &(null_info.a1[0]), &(phl_sta->mac_addr[0]),
		MAC_ADDRESS_LENGTH);

	_os_mem_cpy(d,&(null_info.a2[0]), &(phl_sta->wrole->mac_addr[0]),
			MAC_ADDRESS_LENGTH);

	_os_mem_cpy(d, &(null_info.a3[0]), &(phl_sta->mac_addr[0]),
			MAC_ADDRESS_LENGTH);

	RTW_PHL_PKT_OFLD_REQ(lps->phl_info, role->macid,
				PKT_TYPE_NULL_DATA, &role->null_token, &null_info);

	_phl_lps_init_role_entry(role, ntfy);
	_phl_lps_insert_role(lps, role);
	phl_ps_issue_pm_cap(lps->phl_info, &role->cap_token, role->cap);

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[LPS] Add role %p, macid = %d, mode = %d, interval = %d, smart_ps_mode = %d.\n",
		role, role->macid, role->listen_bcn_mode, role->awake_interval,
		role->smart_ps_mode);

	return RTW_PHL_STATUS_SUCCESS;
}

/* For EXTERNAL application to remove role to use legacy power save (expose)
 * @lps_obj: local created lps object
 * @ntfy: parameter for del role
 *
 * return: rtw_phl_status
 */
enum rtw_phl_status
phl_lps_del_role(void *lps_obj, struct ps_ntfy *ntfy)
{
	struct lps_obj *lps = (struct lps_obj *)lps_obj;
	struct lps_role_entry *role;
	void *d = phl_to_drvpriv(lps->phl_info);

	role = _phl_lps_get_role(lps, ntfy->u.lps_role.macid);
	if (role == NULL) {
		PHL_ERR("%s: get role (macid %d) fail.\n", __func__,
			ntfy->u.lps_role.macid);
		return RTW_PHL_STATUS_FAILURE;
	}

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[LPS] Del role %p, macid = %d\n", role, role->macid);

	role->cb = ntfy->cb;
	role->ctx = ntfy->ctx;

	_phl_lps_issue_pwr_req(lps);
	phl_ps_cancel_pm_cap(lps->phl_info, role->cap_token);

	if(role->state == LPS_ROLE_ST_LPS) {
		_phl_lps_cfg_role(lps, role, false);
	}

	phl_pkt_ofld_cancel(lps->phl_info, role->macid,
				PKT_TYPE_NULL_DATA, &role->null_token);

	_phl_lps_remove_role(lps, role);
	_phl_lps_cancel_pwr_req(lps);

	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status
phl_lps_wow_cfg(void *lps_obj, bool lps_en, u16 macid)
{
	struct lps_obj *lps = (struct lps_obj *)lps_obj;
	struct rtw_phl_com_t *phl_com = lps->phl_info->phl_com;
	struct rtw_ps_cap_t *ps_cap = &phl_com->dev_sw_cap.ps_cap;
	struct lps_role_entry role;

	role.macid = macid;
	role.listen_bcn_mode = ps_cap->lps_wow_listen_bcn_mode;
	role.awake_interval = ps_cap->lps_wow_awake_interval;
	role.smart_ps_mode = ps_cap->lps_wow_smart_ps_mode;

	return _phl_lps_cfg_role(lps, &role, lps_en);
}

void phl_lps_dbg_dump_obj(void *lps_obj, u32 *used, char input[][MAX_ARGV],
			u32 input_num, char *output, u32 out_len)
{
	struct lps_obj *lps = (struct lps_obj *)lps_obj;
	struct rtw_phl_com_t *phl_com = lps->phl_info->phl_com;
	struct rtw_ps_cap_t *ps_cap = &phl_com->dev_sw_cap.ps_cap;
	struct rtw_stats *phl_stats = &phl_com->phl_stats;

	PS_CNSL(out_len, *used, output + *used, out_len - *used,
		"========== PHL LPS Info ==========\n");

	PS_CNSL(out_len, *used, output + *used, out_len - *used,
		"RSSI threshold enter/leave: %d/ %d\n",
		ps_cap->lps_rssi_enter_threshold,
		ps_cap->lps_rssi_leave_threshold);

	PS_CNSL(out_len, *used, output + *used, out_len - *used,
		"tx/ rx lvl: %s/ %s\n",
		phl_ps_id_to_str(PS_STR_TFC_LVL, (u32)phl_stats->tx_traffic.lvl),
		phl_ps_id_to_str(PS_STR_TFC_LVL, (u32)phl_stats->rx_traffic.lvl));

	_phl_lps_dbg_dump_all_role(lps, used, input, input_num,
					output, out_len);
	_phl_lps_dbg_dump_cmd_q(lps, used, input, input_num,
					output, out_len);
}

void phl_lps_watchdog(void *lps_obj)
{
	struct lps_obj *lps = (struct lps_obj *)lps_obj;
	struct rtw_phl_com_t *phl_com = lps->phl_info->phl_com;
	struct rtw_ps_cap_t *ps_cap = &phl_com->dev_sw_cap.ps_cap;
	struct rtw_stats *phl_stats = &phl_com->phl_stats;
	void *d = phl_to_drvpriv(lps->phl_info);
	struct lps_role_entry *pos = NULL;
	struct lps_role_entry *next;
	struct rtw_phl_stainfo_t *stainfo = NULL;
	struct rtw_hal_stainfo_t *hal_sta = NULL;

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[LPS] RSSI threshold enter/leave: %u/%u, tx/rx lvl: %s/%s \n",
		ps_cap->lps_rssi_enter_threshold,
		ps_cap->lps_rssi_leave_threshold,
		phl_ps_id_to_str(PS_STR_TFC_LVL, (u32)phl_stats->tx_traffic.lvl),
		phl_ps_id_to_str(PS_STR_TFC_LVL, (u32)phl_stats->rx_traffic.lvl));


	_os_mutex_lock(d, &lps->mux);

	phl_list_for_loop_safe(pos, next, struct lps_role_entry,
				&lps->role_q, list) {

		stainfo = rtw_phl_get_stainfo_by_macid(lps->phl_info, pos->macid);

		if (NULL == stainfo)
			continue;

		hal_sta = stainfo->hal_sta;

		if (NULL == hal_sta)
			continue;

		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
			"[LPS] active: %u, rssi: %d, aid %d, state: %s \n",
			stainfo->active, PHL_TRANS_2_RSSI(hal_sta->rssi_stat.rssi),
			stainfo->aid,
			phl_ps_id_to_str(PS_STR_LPS_STATE, (u32)pos->state));
	}

	_os_mutex_unlock(d, &lps->mux);
}

#endif /* CONFIG_PS_LPS */
