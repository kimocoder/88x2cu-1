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
#include "phl_fsm.h"
#include "phl_ps_fsm.h"
#include "phl_pm.h"
#include "phl_ips.h"
#include "phl_lps.h"
#include "test/phl_ps_dbg_cmd.h"

#ifdef CONFIG_FSM

#ifdef CONFIG_PS

#define PS_PTCL_NOT_DEFINE 0xFF

#define PS_PTMR_PERIOD 100 /* ms */

struct ps_cmd_entry {
	struct list_head list;
	void (*cb)(void *phl, void *hdl, void *ctx, enum rtw_phl_status stat);
	void *ctx;
	u32 token;
	u8 enq : 1;
	u8 rsvd : 7;
	char type_s[50];
};

enum PS_STATE_ST {
	PS_ST_IDLE,
	PS_ST_SERVICE
};

static int _phl_ps_idle_st_hdl(void *obj, u16 event, void *param);
static int _phl_ps_service_st_hdl(void *obj, u16 event, void *param);

/* STATE table */
static struct fsm_state_ent ps_state_tbl[] = {
	ST_ENT(PS_ST_IDLE, _phl_ps_idle_st_hdl),
	ST_ENT(PS_ST_SERVICE, _phl_ps_service_st_hdl)
};

/* EVENT table */
static struct fsm_event_ent ps_event_tbl[] = {
	EV_ENT(PS_EV_REQ_NOTIFY),
	EV_ENT(PS_EV_PERIODIC_ALARM),
	EV_ENT(PS_EV_PERIODIC_CHK),
	EV_ENT(PS_EV_ROLE_NOTIFY),
	EV_ENT(PS_EV_ROLE_TYPE_NOTIFY),
	EV_ENT(PS_EV_ISSUE_CMD),
	EV_ENT(PS_EV_CANCEL_CMD),
	EV_ENT(PM_EV_ISSUE_CMD),
	EV_ENT(PM_EV_CANCEL_CMD),
	EV_ENT(PM_EV_SET_RADIO),
	EV_ENT(PM_EV_ISSUE_CAP),
	EV_ENT(PM_EV_CANCEL_CAP),
	EV_ENT(IPS_EV_ISSUE_CMD),
	EV_ENT(IPS_EV_CANCEL_CMD),
	EV_ENT(LPS_EV_ISSUE_CMD),
	EV_ENT(LPS_EV_CANCEL_CMD),
	EV_ENT(LPS_EV_ADD_ROLE),
	EV_ENT(LPS_EV_DEL_ROLE),
	EV_ENT(PS_EV_MAX) /* EV_MAX for fsm safety checking */
};

static void
_phl_ps_show_all_cmds(struct ps_obj *ps)
{
	struct ps_cmd_entry *pos = NULL;
	void *d = phl_to_drvpriv(ps->phl_info);

	_os_mutex_lock(d, &ps->mux);

	phl_list_for_loop(pos, struct ps_cmd_entry, &ps->cmd_q, list) {

		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
			"[PS] %s Cmd => token: %u type : %s \n", __func__, pos->token, pos->type_s);
	}

	_os_mutex_unlock(d, &ps->mux);

}
static void
_phl_ps_dbg_dump_cmd_q(struct ps_obj *ps, u32 *used,
	char input[][MAX_ARGV], u32 input_num, char *output, u32 out_len)
{
	struct ps_cmd_entry *cmd = NULL;
	void *d = phl_to_drvpriv(ps->phl_info);
	u32 count = 0;

	PS_CNSL(out_len, *used, output + *used, out_len - *used,
		"Current existing commands:\n");

	_os_mutex_lock(d, &ps->mux);

	phl_list_for_loop(cmd, struct ps_cmd_entry, &ps->cmd_q, list) {

		PS_CNSL(out_len, *used, output + *used, out_len - *used,
			"Cmd => seq: %-11d type : %s \n",
			(int)cmd->token, cmd->type_s);

		count++;
	}
	_os_mutex_unlock(d, &ps->mux);

	if (0 == count) {

		PS_CNSL(out_len, *used, output + *used, out_len - *used, "Empty \n");
	}
}



static void _phl_ps_ptmr_stop(struct ps_obj *ps)
{
	PHL_INFO("[PS], %s(): stop ptmr!!\n", __func__);

	ps->ptmr_stop = true;
	_os_cancel_timer(phl_to_drvpriv(ps->phl_info), &ps->ptmr);
}

static void _phl_ps_ptmr_start(struct ps_obj *ps)
{
	PHL_INFO("[PS], %s(): start ptmr!!\n", __func__);

	ps->ptmr_stop = false;
	_os_set_timer(phl_to_drvpriv(ps->phl_info), &ps->ptmr, PS_PTMR_PERIOD);
}

static void _phl_ps_ptmr_cb(void *ctx)
{
	struct ps_obj *ps = NULL;

	if (!ctx)
		return;
	else
		ps = (struct ps_obj *)ctx;

	if (!ps->ptmr_init || ps->ptmr_stop)
		return;

	/* PHL_INFO("[PS], Timer \n"); */
	phl_ps_send_msg(ps->phl_info, NULL, 0, PS_EV_PERIODIC_CHK);
	_os_set_timer(phl_to_drvpriv(ps->phl_info), &ps->ptmr, PS_PTMR_PERIOD);
}

static void _phl_ps_ptmr_init(struct ps_obj *ps)
{
	_os_init_timer(phl_to_drvpriv(ps->phl_info), &ps->ptmr,
			_phl_ps_ptmr_cb, ps, NULL);
	ps->ptmr_init = true;
	ps->ptmr_stop = true;
}

static void _phl_ps_ptmr_deinit(struct ps_obj *ps)
{
	ps->ptmr_stop = true;

	if (ps->ptmr_init) {
		_phl_ps_ptmr_stop(ps);
		PHL_INFO("[PS], %s(): deinit ptmr!!\n", __func__);
		_os_release_timer(phl_to_drvpriv(ps->phl_info), &ps->ptmr);
		ps->ptmr_init = false;
	}
}

static void
_phl_ps_cmd_exec_cb(struct ps_obj *ps, struct ps_cmd_entry *cmd, u32 status)
{
	cmd->cb(ps->phl_info, cmd, cmd->ctx, status);
}

static void
_phl_ps_role_init_info(struct ps_obj *ps)
{
	u8 idx;

	for(idx = 0; idx < MAX_WIFI_ROLE_NUMBER; idx++)
	{
		ps->role_info[idx].rstate = PHL_ROLE_STOP;
		ps->role_info[idx].ps_req = 0;
		ps->role_info[idx].ps_token = 0;
	}
}

static void
_phl_ps_ptcl_cancel_cmd(struct ps_obj *ps, u8 pctl, u32 token)
{
	struct ps_ntfy ntfy = {0};

	ntfy.free_ntfy = false;
	ntfy.token = token;
	ntfy.wait_done = false;
	if(pctl == PS_PTCL_IPS) {
		phl_ips_cancel_cmd(ps->ips_obj, &ntfy);
	} else if(pctl == PS_PTCL_LPS) {
		phl_lps_cancel_cmd(ps->lps_obj, &ntfy);
	} else {
		PHL_ERR("[PS] pctl (%d) has not function to cancel.\n", pctl);
	}
}

static void
_phl_ps_ptcl_reset_info(struct ps_obj *ps)
{
	u8 idx;
	struct ps_ptcl_info *ptcl;

	ps->leave_all_ptcl = false;

	for(idx = 0; idx < PS_PTCL_NUM; idx++)
	{
		ptcl = &ps->ptcl_info[idx];

		if(ptcl->status == RTW_PHL_STATUS_SUCCESS)
			_phl_ps_ptcl_cancel_cmd(ps, idx, ptcl->token);

		ptcl->token = 0;
		ptcl->status = RTW_PHL_STATUS_FAILURE;
	}
}

static void
_phl_ps_ptcl_init_info(struct ps_obj *ps)
{
	u8 idx;

	for(idx = 0; idx < PS_PTCL_NUM; idx++)
	{
		ps->ptcl_info[idx].token = 0;
		ps->ptcl_info[idx].status = RTW_PHL_STATUS_FAILURE;
	}
}

static enum rtw_phl_status
_phl_ps_leave_all_ps(struct ps_obj *ps)
{
	struct ps_ntfy ntfy = {0};
	void *d = phl_to_drvpriv(ps->phl_info);
	struct ps_ptcl_info *ptcl;

	if(ps->leave_all_ptcl) {
		return RTW_PHL_STATUS_SUCCESS;
	}

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_, "[PS] Leave all ps !!!\n");

	_os_mem_set(d, &ntfy, 0, sizeof(ntfy));
	ptcl = &ps->ptcl_info[PS_PTCL_IPS];
	ntfy.free_ntfy = false;
	ntfy.token = phl_ps_get_token(ps);
	ntfy.wait_done = false;
	ptcl->token = ntfy.token;

	rtw_phl_ps_type_convert_to_string(PHL_PS_NTFY_TYPE_PS_MODULE, ntfy.type_s);
	ptcl->status = phl_ips_issue_cmd(ps->ips_obj, &ntfy);
	if(ptcl->status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("[PS] IPS leave fail.\n");
		return RTW_PHL_STATUS_FAILURE;
	}

	_os_mem_set(d, &ntfy, 0, sizeof(ntfy));
	ptcl = &ps->ptcl_info[PS_PTCL_LPS];
	ntfy.free_ntfy = false;
	ntfy.token = phl_ps_get_token(ps);
	ntfy.wait_done = false;
	ptcl->token = ntfy.token;

	rtw_phl_ps_type_convert_to_string(PHL_PS_NTFY_TYPE_PS_MODULE, ntfy.type_s);
	ptcl->status = phl_lps_issue_cmd(ps->lps_obj, &ntfy);
	if(ptcl->status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("[PS] LPS leave fail.\n");
		return RTW_PHL_STATUS_FAILURE;
	}

	ps->leave_all_ptcl = true;

	return RTW_PHL_STATUS_SUCCESS;
}

static void
_phl_ps_insert_cmd(struct ps_obj *ps, struct ps_cmd_entry *cmd)
{
	void *d = phl_to_drvpriv(ps->phl_info);

	_os_mutex_lock(d, &ps->mux);
	list_add(&cmd->list, &ps->cmd_q);
	ps->cmd_cnt++;
	_os_mutex_unlock(d, &ps->mux);

	cmd->enq = true;
}

static u8
_phl_ps_remove_cmd(struct ps_obj *ps, u32 token)
{
	struct ps_cmd_entry *pos = NULL;
	void *d = phl_to_drvpriv(ps->phl_info);
	u8 find = false;

	_os_mutex_lock(d, &ps->mux);

	phl_list_for_loop(pos, struct ps_cmd_entry, &ps->cmd_q, list) {
		if (pos->token == token) {
			PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
				"[PS] %s: find matched cmd! type (%s) \n", __func__, pos->type_s);
			list_del(&pos->list);
			pos->enq = false;
			ps->cmd_cnt--;
			_os_kmem_free(d, pos, sizeof(*pos));
			find = true;
			break;
		}
	}

	_os_mutex_unlock(d, &ps->mux);

	return find;
}

static void
_phl_ps_judge_pwr_req(struct ps_obj *ps)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	void *d = phl_to_drvpriv(ps->phl_info);
	u8 new_req;

	_os_mutex_lock(d, &ps->mux);

	if (list_empty(&ps->cmd_q)) {
		new_req = false;
	} else {
		new_req = true;
	}

	_os_mutex_unlock(d, &ps->mux);

	if (ps->req != new_req) {

		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
			"[PS] %s: ps req %d -> %d\n", __func__,
			ps->req, new_req);
		ps->req = new_req;
	}
}

static void
_phl_ps_review_cmd(struct ps_obj *ps, struct ps_cmd_entry *cmd,
			enum rtw_phl_status status)
{
	void *d = phl_to_drvpriv(ps->phl_info);

	if (cmd->cb) {
		_phl_ps_cmd_exec_cb(ps, cmd, status);
	}

	if(status == RTW_PHL_STATUS_SUCCESS) {
		_phl_ps_insert_cmd(ps, cmd);
	} else {
		_os_kmem_free(d, cmd, sizeof(*cmd));
		_phl_ps_judge_pwr_req(ps);
		_phl_ps_ptcl_reset_info(ps);
	}
}

static int
_phl_ps_req_enqueue(struct ps_obj *ps, struct fsm_msg *msg)
{
	void *d = phl_to_drvpriv(ps->phl_info);

	_os_spinlock(d, &ps->req_q_lock, _bh, NULL);
	list_add(&msg->list, &ps->req_q);
	ps->req_num++;
	_os_spinunlock(d, &ps->req_q_lock, _bh, NULL);

	return 0;
}

static struct fsm_msg *
_phl_ps_req_dequeue(struct ps_obj *ps)
{
	void *d = phl_to_drvpriv(ps->phl_info);
	struct fsm_msg *msg;

	if (list_empty(&ps->req_q))
		return NULL;

	_os_spinlock(d, &ps->req_q_lock, _bh, NULL);
	msg = list_last_entry(&ps->req_q, struct fsm_msg, list);
	list_del(&msg->list);
	ps->req_num--;
	_os_spinunlock(d, &ps->req_q_lock, _bh, NULL);

	return msg;
}

static u8
_phl_ps_req_proc(struct ps_obj *ps)
{
	struct fsm_msg *msg;

	/* Check pending ps request
	 * Dequeue extra_queue and enqueue back to msg_queue
	 */
	msg = _phl_ps_req_dequeue(ps);
	if (msg != NULL) {
		phl_fsm_sent_msg(ps->fsm_obj, msg);
		return true;
	}
	return false;
}

enum rtw_phl_status
_phl_ps_hdl_periodic_alarm(struct ps_obj *ps)
{
	struct phl_info_t *phl = ps->phl_info;

	return phl_ps_send_msg(phl, NULL, 0, PS_EV_PERIODIC_CHK);
}

static void
_phl_ps_hdl_serv_periodic_chk(struct ps_obj *ps)
{
	/* PHL_INFO("[PS], periodic_chk \n"); */
	phl_pm_periodic_chk(ps->pm_obj);
	phl_ips_periodic_chk(ps->ips_obj);
	phl_lps_periodic_chk(ps->lps_obj);
}

static enum rtw_phl_status
_phl_ps_hdl_serv_issue_cmd(struct ps_obj *ps, void *param)
{
	void *d = phl_to_drvpriv(ps->phl_info);
	struct ps_cmd_entry *cmd = NULL;
	u8 pwr_req = false;
	struct ps_ntfy *ntfy = (struct ps_ntfy *)param;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;

	do {
		cmd = _os_kmem_alloc(d, sizeof(*cmd));
		if (cmd == NULL) {
			PHL_ERR("[PS] %s: alloc cmd fail.\n", __func__);
			pstatus = RTW_PHL_STATUS_RESOURCE;
			break;
		}

		INIT_LIST_HEAD(&cmd->list);
		cmd->cb = ntfy->cb;
		cmd->ctx = ntfy->ctx;
		cmd->token = ntfy->token;
		_os_mem_cpy(d, cmd->type_s, ntfy->type_s, 50);

		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
			"[PS] %s: token (%d), type (%s), req %d -> 1\n", __func__,
			ntfy->token, cmd->type_s, ps->req);
		ps->req = true;

		pstatus = _phl_ps_leave_all_ps(ps);
		_phl_ps_review_cmd(ps, cmd, pstatus);

	} while (0);

	phl_ps_ntfy_completion(ps->phl_info, ntfy);

	return pstatus;
}

static void
_phl_ps_hdl_serv_cancel_cmd(struct ps_obj *ps, void *param)
{
	struct ps_ntfy *ntfy = (struct ps_ntfy *)param;
	u32 token = ntfy->token;

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[PS] %s: token (%d)\n", __func__, ntfy->token);

	do {
		if (_phl_ps_remove_cmd(ps, token) == false) {
			PHL_ERR("[PS] %s: cannot find token(%d)!\n", __func__, token);
			break;
		}

		_phl_ps_judge_pwr_req(ps);

		if (!ps->req && ps->leave_all_ptcl) {
			_phl_ps_ptcl_reset_info(ps);
		}

	} while (0);

	_phl_ps_show_all_cmds(ps);

	phl_ps_ntfy_completion(ps->phl_info, ntfy);
}

static u8
_phl_ps_ips_is_allowed(struct ps_obj *ps)
{
	struct ps_role_info *rinfo;
	u8 idx;

	for(idx = 0; idx < MAX_WIFI_ROLE_NUMBER; idx++) {
		rinfo = &ps->role_info[idx];
		if(rinfo->rstate == PHL_ROLE_MSTS_STA_CONN_START ||
			rinfo->rstate == PHL_ROLE_MSTS_STA_CONN_END ||
			rinfo->rstate == PHL_ROLE_MSTS_AP_START)
			return false;
	}

	return true;
}

static u8
_phl_ps_lps_chk_state_chg_cb(void * phl, u16 macid, u8 original_state)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct rtw_ps_cap_t *ps_cap = &phl_com->dev_sw_cap.ps_cap;
	struct rtw_stats *phl_stats = &phl_info->phl_com->phl_stats;
	struct rtw_phl_stainfo_t *sta;
	struct rtw_hal_stainfo_t *hal_sta;
	u8 new_state = original_state;
	u8 rssi;

	sta = rtw_phl_get_stainfo_by_macid(phl_info, macid);
	if (sta == NULL)
		return false;

	hal_sta = sta->hal_sta;
	if (hal_sta == NULL)
		return false;

	rssi = PHL_TRANS_2_RSSI(hal_sta->rssi_stat.rssi);

	if (original_state == LPS_ROLE_ST_IDLE) {
		if (rssi > ps_cap->lps_rssi_enter_threshold &&
			phl_stats->tx_traffic.lvl == RTW_TFC_IDLE &&
			phl_stats->rx_traffic.lvl == RTW_TFC_IDLE) {
			new_state = LPS_ROLE_ST_LPS;
		} else {
			new_state = LPS_ROLE_ST_IDLE;
		}
	} else if (original_state == LPS_ROLE_ST_LPS) {
		if (rssi < ps_cap->lps_rssi_leave_threshold ||
			phl_stats->tx_traffic.lvl != RTW_TFC_IDLE ||
			phl_stats->rx_traffic.lvl != RTW_TFC_IDLE) {
			new_state = LPS_ROLE_ST_IDLE;
		} else {
			new_state = LPS_ROLE_ST_LPS;
		}
	} else {
		new_state = original_state;
	}

	if (new_state != original_state) {
		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
			"[PS] %s: state: %d -> %d, Tx (%d), Rx (%d), RSSI: %d\n",
			__func__, original_state, new_state,
			phl_stats->tx_traffic.lvl,
			phl_stats->rx_traffic.lvl, rssi);
	}

	return new_state;
}

enum rtw_phl_status
_phl_ps_issue_cmd(struct phl_info_t *phl, u32 *token, enum PHL_PS_NOTIFY_TYPE type)
{
	struct ps_obj *ps = phl->ps_obj;
	struct ps_ntfy ntfy = {0};
	void *d = phl_to_drvpriv(phl);

	_os_mem_set(d, &ntfy, 0, sizeof(ntfy));

	ntfy.free_ntfy = false;
	ntfy.token = phl_ps_get_token(ps);
	ntfy.wait_done = false;
	*token = ntfy.token;
	rtw_phl_ps_type_convert_to_string(type, ntfy.type_s);

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[PS] New ps cmd, token = %d.\n", ntfy.token);

	return _phl_ps_hdl_serv_issue_cmd(ps, &ntfy);
}

void _phl_ps_cancel_cmd(struct phl_info_t *phl, u32 token)
{
	struct ps_obj *ps = phl->ps_obj;
	struct ps_ntfy ntfy = {0};

	ntfy.free_ntfy = false;
	ntfy.token = token;
	ntfy.wait_done = false;

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[PS] Cancel ps cmd, token = %d\n", token);

	_phl_ps_hdl_serv_cancel_cmd(ps, &ntfy);
}

enum rtw_phl_status
_phl_ps_ips_issue_cmd(struct phl_info_t *phl, u32 *token, enum PHL_PS_NOTIFY_TYPE type)
{
	struct ps_obj *ps = phl->ps_obj;
	struct ps_ntfy ntfy = {0};

	ntfy.free_ntfy = false;
	ntfy.token = phl_ps_get_token(ps);
	ntfy.wait_done = false;
	*token = ntfy.token;
	rtw_phl_ps_type_convert_to_string(type, ntfy.type_s);

	return phl_ips_issue_cmd(ps->ips_obj, &ntfy);
}

void _phl_ps_ips_cancel_cmd(struct phl_info_t *phl, u32 token)
{
	struct ps_obj *ps = phl->ps_obj;
	struct ps_ntfy ntfy = {0};

	ntfy.free_ntfy = false;
	ntfy.token = token;
	ntfy.wait_done = false;
	phl_ips_cancel_cmd(ps->ips_obj, &ntfy);
}

static void
_phl_ps_lps_add_role(struct ps_obj *ps, u16 macid)
{
	struct ps_ntfy ntfy = {0};
	struct lps_role_info_param *role = &ntfy.u.lps_role;
	struct rtw_phl_com_t *phl_com = ps->phl_info->phl_com;
	struct rtw_ps_cap_t *ps_cap = &phl_com->dev_sw_cap.ps_cap;

	role->cap = HWPS_CAP_PWRON;
	if(ps_cap->lps_cap & HWPS_CAP_PWR_GATED) {
		role->cap |= HWPS_CAP_RF_OFF | HWPS_CAP_CLK_GATED
				| HWPS_CAP_PWR_GATED;

	} else if(ps_cap->lps_cap & HWPS_CAP_CLK_GATED) {
		role->cap = HWPS_CAP_RF_OFF | HWPS_CAP_CLK_GATED;

	} else if(ps_cap->lps_cap & HWPS_CAP_RF_OFF) {
		role->cap = HWPS_CAP_RF_OFF;
	}

	role->macid = macid;
	role->listen_bcn_mode = ps_cap->lps_listen_bcn_mode;
	role->awake_interval = ps_cap->lps_awake_interval;
	role->smart_ps_mode = ps_cap->lps_smart_ps_mode;
	role->chk_lps_state_chg_cb = _phl_ps_lps_chk_state_chg_cb;

	phl_lps_add_role(ps->lps_obj, &ntfy);
}

static void
_phl_ps_lps_del_role(struct ps_obj *ps, u16 macid)
{
	struct ps_ntfy ntfy = {0};

	ntfy.u.lps_role.macid = (u8)macid;
	phl_lps_del_role(ps->lps_obj, &ntfy);
}

static void
_phl_ps_hdl_role_sta_conn_end(struct ps_obj *ps, u16 macid,
	struct ps_role_info *rinfo)
{
#ifdef RTW_WKARD_LPS_P2P_ROLE_TYPE
	if (rinfo->rtype != PHL_RTYPE_P2P_GC) {
		_phl_ps_lps_add_role(ps, macid);
		if(rinfo->ps_req) {
			_phl_ps_cancel_cmd(ps->phl_info, rinfo->ps_token);
			rinfo->ps_req = false;
		}
	}
#else
	_phl_ps_lps_add_role(ps, macid);
	if(rinfo->ps_req) {
		_phl_ps_cancel_cmd(ps->phl_info, rinfo->ps_token);
		rinfo->ps_req = false;
	}
#endif
}

static void
_phl_ps_hdl_role_sta_dis_conn(struct ps_obj *ps, u16 macid,
	struct ps_role_info *rinfo)
{
#ifdef RTW_WKARD_LPS_P2P_ROLE_TYPE
	if (rinfo->rtype != PHL_RTYPE_P2P_GC) {
		_phl_ps_lps_del_role(ps, macid);
	}
#endif
	if(rinfo->ps_req) {
		_phl_ps_cancel_cmd(ps->phl_info, rinfo->ps_token);
		rinfo->ps_req = false;
	}
}

static void
_phl_ps_hdl_serv_role_notify(struct ps_obj *ps, void *param)
{
	void *d = phl_to_drvpriv(ps->phl_info);
	struct ps_ntfy *ntfy = (struct ps_ntfy *)param;
	struct ps_role_info_param *role = &ntfy->u.ps_role;
	struct ps_role_info *rinfo;

	rinfo = &ps->role_info[role->role_id];
	rinfo->rstate = role->rstate;

	if(!_phl_ps_ips_is_allowed(ps)) {
		if(!ps->ips_req) {
			_phl_ps_ips_issue_cmd(ps->phl_info, &ps->ips_token, PHL_PS_NTFY_TYPE_ROLE);
			ps->ips_req = true;
		}
	}

	if(role->rstate == PHL_ROLE_MSTS_STA_CONN_START) {
		if(!rinfo->ps_req) {
			_phl_ps_issue_cmd(ps->phl_info, &rinfo->ps_token, PHL_PS_NTFY_TYPE_ROLE);
			rinfo->ps_req = true;
		}
	} else if(role->rstate == PHL_ROLE_MSTS_STA_CONN_END) {
		_phl_ps_hdl_role_sta_conn_end(ps, role->macid, rinfo);
	} else if(role->rstate == PHL_ROLE_MSTS_STA_DIS_CONN) {
		_phl_ps_hdl_role_sta_dis_conn(ps, role->macid, rinfo);
	} else if(role->rstate == PHL_ROLE_MSTS_AP_START) {
		if(!rinfo->ps_req) {
			_phl_ps_issue_cmd(ps->phl_info, &rinfo->ps_token, PHL_PS_NTFY_TYPE_ROLE);
			rinfo->ps_req = true;
		}
	} else if(role->rstate == PHL_ROLE_MSTS_AP_STOP) {
		if(rinfo->ps_req) {
			_phl_ps_cancel_cmd(ps->phl_info, rinfo->ps_token);
			rinfo->ps_req = false;
		}
	}

	if(_phl_ps_ips_is_allowed(ps)) {
		if(ps->ips_req) {
			_phl_ps_ips_cancel_cmd(ps->phl_info, ps->ips_token);
			ps->ips_req = false;
		}
	}

	phl_ps_ntfy_completion(ps->phl_info, ntfy);
}

#ifdef RTW_WKARD_LPS_P2P_ROLE_TYPE
static void
_phl_ps_hdl_serv_role_type_notify(struct ps_obj *ps, void *param)
{
	void *d = phl_to_drvpriv(ps->phl_info);
	struct ps_ntfy *ntfy = (struct ps_ntfy *)param;
	struct ps_role_info_param *role = &ntfy->u.ps_role;
	struct ps_role_info *rinfo;

	rinfo = &ps->role_info[role->role_id];
	rinfo->rtype = role->rtype;

	phl_ps_ntfy_completion(ps->phl_info, ntfy);
}
#endif

static void _phl_ps_cap_init(struct ps_obj *ps)
{
	struct rtw_phl_com_t *phl_com = ps->phl_info->phl_com;
	struct rtw_ps_cap_t *ps_cap = &phl_com->dev_sw_cap.ps_cap;

	ps_cap->ips_en = false;
	ps_cap->ips_cap = 0;
	ps_cap->ips_defer_time = 100;

	ps_cap->lps_en = false;
	ps_cap->lps_cap = 0;
	ps_cap->lps_awake_interval = 0;
	ps_cap->lps_listen_bcn_mode = RTW_LPS_RLBM_MIN;
	ps_cap->lps_smart_ps_mode = RTW_LPS_TRX_PWR0;
	ps_cap->lps_defer_time = 8000;
	ps_cap->lps_rssi_enter_threshold = 40;
	ps_cap->lps_rssi_leave_threshold = 35;

	ps_cap->lps_wow_en = false;
	ps_cap->lps_wow_cap = 0;
	ps_cap->lps_wow_awake_interval = 0;
	ps_cap->lps_wow_listen_bcn_mode = RTW_LPS_RLBM_MAX;
	ps_cap->lps_wow_smart_ps_mode = RTW_LPS_TRX_PWR0;
}

static void
_phl_ps_init_obj(struct ps_obj *ps)
{
	void *d = phl_to_drvpriv(ps->phl_info);

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_, "[PS] %s\n", __func__);

	ps->cmd_cnt = 0;
	ps->ntfy_cnt = 0;
	ps->cur_seq = 1;
	ps->req = 0;
	ps->leave_all_ptcl = false;
	ps->ser_ongoing = false;

	_phl_ps_role_init_info(ps);
	_phl_ps_ptcl_init_info(ps);

	INIT_LIST_HEAD(&ps->req_q);
	INIT_LIST_HEAD(&ps->cmd_q);
	INIT_LIST_HEAD(&ps->ntfy_q);

	_os_mutex_init(d, &ps->mux);
	_os_spinlock_init(d, &ps->ntfy_lock);
	_os_spinlock_init(d, &ps->req_q_lock);

	_phl_ps_cap_init(ps);
	_phl_ps_ptmr_init(ps);
}

static void
_phl_ps_deinit_obj(struct ps_obj *ps)
{
	void *d = phl_to_drvpriv(ps->phl_info);
	struct ps_cmd_entry *cmd_pos = NULL;
	struct ps_cmd_entry *cmd_n = NULL;
	struct ps_ntfy *ntfy_pos = NULL;
	struct ps_ntfy *ntfy_n = NULL;
	struct fsm_msg *msg;

	_phl_ps_ptmr_deinit(ps);

	_os_mutex_lock(d, &ps->mux);

	phl_list_for_loop_safe(cmd_pos, cmd_n, struct ps_cmd_entry,
				&ps->cmd_q, list) {

		list_del(&cmd_pos->list);
		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_, "[PS] del cmd %p\n",
			cmd_pos);
		_os_kmem_free(d, cmd_pos, sizeof(*cmd_pos));
	}

	_os_mutex_unlock(d, &ps->mux);

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_, "[PS] ntfy cnt: %d\n",
		ps->ntfy_cnt);

	_os_spinlock(d, &ps->ntfy_lock, _bh, NULL);

	phl_list_for_loop_safe(ntfy_pos, ntfy_n, struct ps_ntfy,
				&ps->ntfy_q, list) {

		list_del(&ntfy_pos->list);
		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_, "[PS] del ntfy %p, token: %d\n",
			ntfy_pos, ntfy_pos->token);
		_os_kmem_free(d, ntfy_pos, sizeof(*ntfy_pos));
		ps->ntfy_cnt--;
	}

	_os_spinunlock(d, &ps->ntfy_lock, _bh, NULL);

	while ((msg = _phl_ps_req_dequeue(ps)) != NULL) {
		_os_kmem_free(d, (void *)msg, sizeof(*msg));
	}

	_os_mutex_deinit(d, &ps->mux);
	_os_spinlock_free(d, &ps->ntfy_lock);
	_os_spinlock_free(d, &ps->req_q_lock);
}

static void
_phl_ps_dump_obj(void *fsm, char *s, int *sz)
{
	/* nothing to do for now */
}

static void
_phl_ps_dump_fsm(void *fsm, char *s, int *sz)
{
	/* nothing to do for now */
}

/*
 * legacy power save state handler
 */

/*
 * ps idle handler
 */
static int
_phl_ps_idle_st_hdl(void *obj, u16 event, void *param)
{
	struct ps_obj *ps = (struct ps_obj *)obj;

	switch (event) {
	case FSM_EV_SWITCH_IN:
		break;

	case FSM_EV_STATE_IN:
	case PS_EV_REQ_NOTIFY:
		if(_phl_ps_req_proc(ps))
			phl_fsm_state_goto(ps->fsm_obj, PS_ST_SERVICE);
		break;

	case PS_EV_PERIODIC_ALARM:
		_phl_ps_hdl_periodic_alarm(ps);
		break;

	case FSM_EV_STATE_OUT:
		break;

	case FSM_EV_SWITCH_OUT:
		break;

	default:
		break;
	}
	return 0;
}

static int
_phl_ps_service_st_hdl(void *obj, u16 event, void *param)
{
	struct ps_obj *ps = (struct ps_obj *)obj;
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	enum PS_STATE_ST next_state = PS_ST_IDLE;
	int rtn = FSM_KEEP_PARAM;

	switch (event) {
	case FSM_EV_STATE_IN:
		break;

	case PS_EV_PERIODIC_ALARM:
		_phl_ps_hdl_periodic_alarm(ps);
		break;

	case PS_EV_PERIODIC_CHK:
		_phl_ps_hdl_serv_periodic_chk(ps);
		phl_fsm_state_goto(ps->fsm_obj, PS_ST_IDLE);
		break;

	case PS_EV_ROLE_NOTIFY:
		_phl_ps_hdl_serv_role_notify(ps, param);
		phl_fsm_state_goto(ps->fsm_obj, PS_ST_IDLE);
		break;

#ifdef RTW_WKARD_LPS_P2P_ROLE_TYPE
	case PS_EV_ROLE_TYPE_NOTIFY:
		_phl_ps_hdl_serv_role_type_notify(ps, param);
		phl_fsm_state_goto(ps->fsm_obj, PS_ST_IDLE);
		break;
#endif

	case PS_EV_ISSUE_CMD:
		_phl_ps_hdl_serv_issue_cmd(ps, param);
		phl_fsm_state_goto(ps->fsm_obj, PS_ST_IDLE);
		break;

	case PS_EV_CANCEL_CMD:
		_phl_ps_hdl_serv_cancel_cmd(ps, param);
		phl_fsm_state_goto(ps->fsm_obj, PS_ST_IDLE);
		break;

	case PM_EV_ISSUE_CMD:
		phl_pm_issue_pwr_cmd(ps->pm_obj, param);
		phl_fsm_state_goto(ps->fsm_obj, PS_ST_IDLE);
		break;

	case PM_EV_CANCEL_CMD:
		phl_pm_cancel_pwr_cmd(ps->pm_obj, param);
		phl_fsm_state_goto(ps->fsm_obj, PS_ST_IDLE);
		break;

	case PM_EV_SET_RADIO:
		phl_pm_set_radio_state(ps->pm_obj, param);
		phl_fsm_state_goto(ps->fsm_obj, PS_ST_IDLE);
		break;

#if 0 /* For Test usage */

	case PM_EV_ISSUE_CAP:
		phl_pm_issue_pwr_cap(ps->pm_obj, param);
		phl_fsm_state_goto(ps->fsm_obj, PS_ST_IDLE);
		break;

	case PM_EV_CANCEL_CAP:
		phl_pm_cancel_pwr_cap(ps->pm_obj, param);
		phl_fsm_state_goto(ps->fsm_obj, PS_ST_IDLE);
		break;

#endif /* PS_TEST */

	case IPS_EV_ISSUE_CMD:
		phl_ips_issue_cmd(ps->ips_obj, param);
		phl_fsm_state_goto(ps->fsm_obj, PS_ST_IDLE);
		break;

	case IPS_EV_CANCEL_CMD:
		phl_ips_cancel_cmd(ps->ips_obj, param);
		phl_fsm_state_goto(ps->fsm_obj, PS_ST_IDLE);
		break;

	case LPS_EV_ISSUE_CMD:
		phl_lps_issue_cmd(ps->lps_obj, param);
		phl_fsm_state_goto(ps->fsm_obj, PS_ST_IDLE);
		break;

	case LPS_EV_CANCEL_CMD:
		phl_lps_cancel_cmd(ps->lps_obj, param);
		phl_fsm_state_goto(ps->fsm_obj, PS_ST_IDLE);
		break;

	case FSM_EV_CANCEL:
		phl_fsm_state_goto(ps->fsm_obj, PS_ST_IDLE);
		break;

	case FSM_EV_STATE_OUT:
		break;

	default:
		break;
	}
	return rtn;
}

/* Stop power save (expose) */
/* @ps: power save to be cancelled
 */
static enum rtw_phl_status
_phl_ps_cancel(struct ps_obj *ps)
{
#ifdef PHL_INCLUDE_FSM
	return phl_fsm_cancel_obj(ps->fsm_obj);
#else
	return RTW_PHL_STATUS_SUCCESS;
#endif /* PHL_INCLUDE_FSM */
}

/* Create a power save FSM
 * @root: FSM root structure
 * @phl_info: private data structure to invoke hal/phl function
 *
 * return
 * fsm_main: FSM main structure (Do NOT expose)
 */
static struct fsm_main *
_phl_ps_new_fsm(struct fsm_root *root, struct phl_info_t *phl_info)
{
	void *d = phl_to_drvpriv(phl_info);
	struct fsm_main *fsm = NULL;
	struct rtw_phl_fsm_tb tb;

	_os_mem_set(d, &tb, 0, sizeof(tb));
	tb.max_state = sizeof(ps_state_tbl)/sizeof(ps_state_tbl[0]);
	tb.max_event = sizeof(ps_event_tbl)/sizeof(ps_event_tbl[0]);
	tb.state_tbl = ps_state_tbl;
	tb.evt_tbl = ps_event_tbl;
	tb.dump_obj = _phl_ps_dump_obj;
	tb.dump_fsm = _phl_ps_dump_fsm;
	tb.dbg_level = FSM_DBG_INFO;
	tb.evt_level = FSM_DBG_INFO;

	fsm = phl_fsm_init_fsm(root, "ps", phl_info, &tb);

	return fsm;
}

/* Destory ps fsm */
/* @fsm: see fsm_main
 */
static void _phl_ps_destory_fsm(struct fsm_main *fsm)
{
	if (fsm == NULL)
		return;

	/* deinit fsm local variable if has */

	/* call FSM Framework to deinit fsm */
	phl_fsm_deinit_fsm(fsm);
}

/* Create power save object
 * @fsm: FSM main structure which created by phl_pm_new_fsm()
 * @phl_info: private data structure to invoke hal/phl function
 *
 * return
 * ps_obj: structure of command object (Do NOT expose)
 */
static struct ps_obj *
_phl_ps_new_obj(struct fsm_main *fsm, struct phl_info_t *phl_info)
{
	struct fsm_obj *obj;
	struct ps_obj *ps;

	ps = phl_fsm_new_obj(fsm, (void **)&obj, sizeof(*ps));

	if (ps == NULL || obj == NULL) {
		/* TODO free fsm; currently will be freed in deinit process */
		FSM_ERR(fsm, "ps: malloc obj fail\n");
		return NULL;
	}
	ps->fsm = fsm;
	ps->fsm_obj = obj;
	ps->phl_info = phl_info;

	/* init obj local use variable */
	_phl_ps_init_obj(ps);

	return ps;
}

/* Destory ps object */
/* @ps: local created ps object
 */
static void _phl_ps_destory_obj(struct ps_obj *ps)
{
	void *d;

	if (ps == NULL)
		return;

	d = phl_to_drvpriv(ps->phl_info);

	/* deinit and free all local variables */
	_phl_ps_deinit_obj(ps);

	/* inform FSM framewory to recycle fsm_obj */
	phl_fsm_destory_obj(ps->fsm_obj);
}

static enum rtw_phl_status
_phl_ps_init_submodule(struct phl_info_t *phl_info)
{
	struct ps_obj *ps = (struct ps_obj *)phl_info->ps_obj;
	enum rtw_phl_status status;

	status = phl_pm_init(&ps->pm_obj, phl_info);
	if (status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("pm init failed\n");
		goto pm_fail;
	}

	status = phl_ips_init(&ps->ips_obj, phl_info);
	if (status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("ips init failed\n");
		goto ips_fail;
	}

	status = phl_lps_init(&ps->lps_obj, phl_info);
	if (status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("lps init failed\n");
		goto lps_fail;
	}

	return RTW_PHL_STATUS_SUCCESS;

lps_fail:
	phl_ips_deinit(ps->pm_obj);
ips_fail:
	phl_pm_deinit(ps->pm_obj);
pm_fail:
	return RTW_PHL_STATUS_FAILURE;
}

static void
_phl_ps_deinit_submodule(struct ps_obj *ps)
{
	phl_lps_deinit(ps->lps_obj);
	phl_ips_deinit(ps->ips_obj);
	phl_pm_deinit(ps->pm_obj);
}

/* For EXTERNAL application to initialize power save
 * @phl_info: private data structure to invoke hal/phl function
 *
 * return: rtw_phl_status
 */
enum rtw_phl_status
phl_ps_init(struct phl_info_t *phl_info)
{
	enum rtw_phl_status status;

	if (phl_info->ps_fsm != NULL)
		return RTW_PHL_STATUS_FAILURE;

	phl_info->ps_fsm = _phl_ps_new_fsm(phl_info->fsm_root, phl_info);
	if (phl_info->ps_fsm == NULL)
		return RTW_PHL_STATUS_FAILURE;

	if (phl_info->ps_obj != NULL)
		goto obj_fail;

	phl_info->ps_obj = _phl_ps_new_obj(phl_info->ps_fsm, phl_info);
	if (phl_info->ps_obj == NULL)
		goto obj_fail;

	status = _phl_ps_init_submodule(phl_info);
	if (status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_ps_init_submodule failed\n");
		goto submodule_fail;
	}

	return RTW_PHL_STATUS_SUCCESS;

submodule_fail:
	_phl_ps_destory_obj(phl_info->ps_obj);
	phl_info->ps_obj = NULL;
obj_fail:
	phl_fsm_deinit_fsm(phl_info->ps_fsm);
	phl_info->ps_fsm = NULL;
	return RTW_PHL_STATUS_FAILURE;
}

/* For EXTERNAL application to deinitialize power save
 * @phl_info: private data structure to invoke hal/phl function
 *
 */
void phl_ps_deinit(struct phl_info_t *phl_info)
{
	_phl_ps_deinit_submodule(phl_info->ps_obj);

	_phl_ps_destory_obj(phl_info->ps_obj);
	phl_info->ps_obj = NULL;
	_phl_ps_destory_fsm(phl_info->ps_fsm);
	phl_info->ps_fsm = NULL;
}

/* For EXTERNAL application to start power save module
 * @phl_info: private data structure to invoke hal/phl function
 *
 * return: rtw_phl_status
 */
enum rtw_phl_status
phl_ps_fsm_start(struct phl_info_t *phl_info)
{
	struct ps_obj *ps = (struct ps_obj *)phl_info->ps_obj;

	phl_pm_start(ps->pm_obj);
	phl_fsm_start_fsm(phl_info->ps_fsm);
	_phl_ps_ptmr_start(ps);

	return RTW_PHL_STATUS_SUCCESS;
}

/* For EXTERNAL application to stop power save module
 * @phl_info: private data structure to invoke hal/phl function
 *
 */
void phl_ps_fsm_stop(struct phl_info_t *phl_info)
{
	struct ps_obj *ps = (struct ps_obj *)phl_info->ps_obj;

	_phl_ps_ptmr_stop(ps);
	phl_pm_stop(ps->pm_obj);
	phl_fsm_stop_fsm(phl_info->ps_fsm);
}

/* For PS INTERNAL application to send msg to FSM
 * @phl_info: private data structure to invoke hal/phl function
 * @param: param for event
 * @sz: size of param
 * @event: FSM event
 *
 */
enum rtw_phl_status
phl_ps_send_msg(struct phl_info_t *phl, void *param, u32 sz, u16 event)
{
	struct ps_obj *ps = phl->ps_obj;
	struct fsm_msg *msg;

	/* NEW message to start scan */
	msg = phl_fsm_new_msg(ps->fsm_obj, event);
	if (msg == NULL) {
		PHL_ERR("[PS] %s: alloc msg fail.\n", __func__);
		return RTW_PHL_STATUS_RESOURCE;
	}

	msg->param = param;
	msg->param_sz = sz;

	_phl_ps_req_enqueue(ps, msg);

	/* notify ps-obj to dequeue from extra queue */
	return phl_fsm_gen_msg(phl, ps->fsm_obj,
		NULL, 0, PS_EV_REQ_NOTIFY);
}

/* For PS INTERNAL application to get token
 * @ps_obj: local created ps object
 *
 */
u32 phl_ps_get_token(void *ps_obj)
{
	struct ps_obj *ps = (struct ps_obj *)ps_obj;
	void *d = phl_to_drvpriv(ps->phl_info);
	u32 token;

	_os_mutex_lock(d, &ps->mux);
	token = ps->cur_seq;
	ps->cur_seq++;
	if(ps->cur_seq == 0)
		ps->cur_seq = 1;
	_os_mutex_unlock(d, &ps->mux);

	return token;
}

/* For PS INTERNAL application to test power level (expose)
 * @phl: refer to phl_info_t
 * @test_comp: Requierd power level
 */
u8 phl_ps_test_pwr_level(struct phl_info_t *phl, u32 test_comp)
{
	struct ps_obj *ps = phl->ps_obj;

	return phl_pm_test_pwr_level(ps->pm_obj, test_comp);
}

/* For PS INTERNAL application to issue power command (expose)
 * @phl: refer to phl_info_t
 * @token: The token for this power command
 * @comp: The HW block this power command required
 */
enum rtw_phl_status
phl_ps_issue_pm_cmd(struct phl_info_t *phl, u32 *token, u32 comp,
		enum PHL_PS_NOTIFY_TYPE type)
{
	struct ps_obj *ps = phl->ps_obj;
	struct ps_ntfy ntfy = {0};
	void *d = phl_to_drvpriv(phl);

	_os_mem_set(d, &ntfy, 0, sizeof(ntfy));

	ntfy.free_ntfy = false;
	ntfy.token = phl_ps_get_token(ps);
	ntfy.wait_done = false;
	ntfy.u.pm_comp = comp;
	*token = ntfy.token;

	rtw_phl_ps_type_convert_to_string(type, ntfy.type_s);

	return phl_pm_issue_pwr_cmd(ps->pm_obj, &ntfy);
}

/* For PS INTERNAL application to cancel power command (expose)
 * @phl: refer to phl_info_t
 * @token: The token for this power command
 */
void phl_ps_cancel_pm_cmd(struct phl_info_t *phl, u32 token)
{
	struct ps_obj *ps = phl->ps_obj;
	struct ps_ntfy ntfy = {0};

	ntfy.free_ntfy = false;
	ntfy.token = token;
	ntfy.wait_done = false;
	phl_pm_cancel_pwr_cmd(ps->pm_obj, &ntfy);
}

/* For PS INTERNAL application to issue power saving capability (expose)
 * @phl: refer to phl_info_t
 * @token: The token for this power saving capbility
 * @cap: The HW state this power saving capbility claimed
 */
enum rtw_phl_status
phl_ps_issue_pm_cap(struct phl_info_t *phl, u32 *token, u32 cap)
{
	struct ps_obj *ps = phl->ps_obj;
	struct ps_ntfy ntfy = {0};

	ntfy.free_ntfy = false;
	ntfy.token = phl_ps_get_token(ps);
	ntfy.wait_done = false;
	ntfy.u.pm_cap = cap;
	*token = ntfy.token;

	return phl_pm_issue_pwr_cap(ps->pm_obj, &ntfy);
}


/* For PS INTERNAL application to cancel power saving capability (expose)
 * @phl: refer to phl_info_t
 * @token: The token for this power saving capbility
 */
void phl_ps_cancel_pm_cap(struct phl_info_t *phl, u32 token)
{
	struct ps_obj *ps = phl->ps_obj;
	struct ps_ntfy ntfy = {0};

	ntfy.free_ntfy = false;
	ntfy.token = token;
	ntfy.wait_done = false;
	phl_pm_cancel_pwr_cap(ps->pm_obj, &ntfy);
}

void phl_ps_ntfy_completion(struct phl_info_t *phl, struct ps_ntfy *ntfy)
{
	struct ps_obj *ps = phl->ps_obj;
	void *d = phl_to_drvpriv(phl);

	_os_spinlock(d, &ps->ntfy_lock, _bh, NULL);

	if (ntfy->wait_done && ntfy->done) {
		_os_event_set(d, ntfy->done);
	} else if (ntfy->free_ntfy) {
		list_del(&ntfy->list);
		_os_kmem_free(d, ntfy, sizeof(*ntfy));
		ps->ntfy_cnt--;
	}

	_os_spinunlock(d, &ps->ntfy_lock, _bh, NULL);
}

void phl_ps_dbg_dump_obj(void *ps_obj, u32 *used, char input[][MAX_ARGV],
			u32 input_num, char *output, u32 out_len)
{
	struct ps_obj *ps = (struct ps_obj *)ps_obj;
	struct rtw_phl_com_t *phl_com = ps->phl_info->phl_com;
	struct rtw_ps_cap_t *ps_cap = &phl_com->dev_sw_cap.ps_cap;
	u32 i;

	PS_CNSL(out_len, *used, output + *used, out_len - *used,
		"========== PHL PS Info ==========\n");

	PS_CNSL(out_len, *used, output + *used, out_len - *used,
		"PS option:\n");

	for (i = BIT0; i < RTW_PS_OPTION_MAX; i <<= 1) {
	PS_CNSL(out_len, *used, output + *used, out_len - *used,
		"%s: %d\n",
		phl_ps_id_to_str(PS_STR_OPTION, i),
		((ps_cap->ps_option & i)? 1: 0));
	}

	PS_CNSL(out_len, *used, output + *used, out_len - *used,
		"\n");
}

#endif /*CONFIG_PS */
#endif /*CONFIG_FSM*/

