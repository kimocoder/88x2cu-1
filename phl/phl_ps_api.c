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
#include "phl_lps.h"

static void
_phl_ps_free_ntfy(struct phl_info_t *phl_info, struct ps_ntfy *ntfy)
{
	struct ps_obj *ps = phl_info->ps_obj;
	void *d = phl_to_drvpriv(phl_info);

	_os_spinlock(d, &ps->ntfy_lock, _bh, NULL);
	list_del(&ntfy->list);
	_os_kmem_free(d, ntfy, sizeof(*ntfy));
	ps->ntfy_cnt--;
	_os_spinunlock(d, &ps->ntfy_lock, _bh, NULL);
}

static enum rtw_phl_status
_phl_ps_completion(struct phl_info_t *phl_info, struct ps_ntfy *ntfy,
			bool wait_done, _os_event *done)
{
	struct ps_obj *ps = (struct ps_obj *)phl_info->ps_obj;
	void *d = phl_to_drvpriv(phl_info);

	if (wait_done) {
		if(_os_event_wait(d, done, 1000) == 0) {
			_os_spinlock(d, &ps->ntfy_lock, _bh, NULL);
			ntfy->wait_done = false;
			ntfy->done = NULL;
			PHL_ERR("%s: wait timeout. seq: %u, type :%s\n",
				__func__, ntfy->token, ntfy->type_s);
			_os_spinunlock(d, &ps->ntfy_lock, _bh, NULL);
			return RTW_PHL_STATUS_FAILURE;
		} else {
			_phl_ps_free_ntfy(phl_info, ntfy);
			return RTW_PHL_STATUS_SUCCESS;
		}
	} else {
		return RTW_PHL_STATUS_PENDING;
	}
}

static struct ps_ntfy *
_phl_ps_new_ntfy(struct phl_info_t *phl_info)
{
	struct ps_obj *ps = phl_info->ps_obj;
	struct ps_ntfy *ntfy;
	void *d = phl_to_drvpriv(phl_info);

	ntfy = (struct ps_ntfy *)_os_kmem_alloc(d, sizeof(*ntfy));
	if (ntfy == NULL) {
		PHL_ERR("%s: alloc ntfy fail.\n", __func__);
		return NULL;
	}
	_os_mem_set(d, ntfy, 0, sizeof(*ntfy));

	INIT_LIST_HEAD(&ntfy->list);

	_os_spinlock(d, &ps->ntfy_lock, _bh, NULL);
	list_add(&ntfy->list, &ps->ntfy_q);
	ps->ntfy_cnt++;
	_os_spinunlock(d, &ps->ntfy_lock, _bh, NULL);

	return ntfy;
}

#ifdef CONFIG_PS

void
rtw_phl_ps_type_convert_to_string(enum PHL_PS_NOTIFY_TYPE type, char *buf)
{
	switch (type) {

	case PHL_PS_NTFY_TYPE_WATCHDOG:
		_os_strcpy(buf, "WATCHDOG");
		break;
	case PHL_PS_NTFY_TYPE_BATTERY_CHG:
		_os_strcpy(buf, "BATTERY_CHG");
		break;
	case PHL_PS_NTFY_TYPE_CREATE_MAC:
		_os_strcpy(buf, "CREATE_MAC");
		break;
	case PHL_PS_NTFY_TYPE_DELETE_MAC:
		_os_strcpy(buf, "DELETE_MAC");
		break;
	case PHL_PS_NTFY_TYPE_CHANGE_PORT:
		_os_strcpy(buf, "CHANGE_PORT");
		break;
	case PHL_PS_NTFY_TYPE_CONNECT:
		_os_strcpy(buf, "CONNECT");
		break;
	case PHL_PS_NTFY_TYPE_DISASSOCIATE:
		_os_strcpy(buf, "DISASSOCIATE");
		break;
	case PHL_PS_NTFY_TYPE_START_AP:
		_os_strcpy(buf, "START_AP");
		break;
	case PHL_PS_NTFY_TYPE_DRIVER_UNLOAD:
		_os_strcpy(buf, "DRIVER_UNLOAD");
		break;
	case PHL_PS_NTFY_TYPE_WOWLAN:
		_os_strcpy(buf, "WOWLAN");
		break;
	case PHL_PS_NTFY_TYPE_SCAN:
		_os_strcpy(buf, "SCAN");
		break;
	case PHL_PS_NTFY_TYPE_CMD_IO:
		_os_strcpy(buf, "CMD_IO");
		break;
	case PHL_PS_NTFY_TYPE_ROLE:
		_os_strcpy(buf, "ROLE");
		break;
	case PHL_PS_NTFY_TYPE_PS_MODULE:
		_os_strcpy(buf, "PS_MODULE");
		break;
	case PHL_PS_NTFY_TYPE_LPS_MODULE:
		_os_strcpy(buf, "LPS_MODULE");
		break;
	default:
		_os_strcpy(buf, "UNKNOWN");
		break;
	}
}

static void
_phl_ps_battery_chg_buf_done(void* priv, struct phl_msg* msg)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)priv;

	if(msg->inbuf && msg->inlen){
		_os_kmem_free(phl_to_drvpriv(phl_info), msg->inbuf, msg->inlen);
	}
}

/* For EXTERNAL application to set power mode by hub msg (expose)
 * @phl: refer to phl_info_t
 * @ips_allow: allow ips enter
 * @lps_allow: allow lps enter
 */
enum rtw_phl_status
rtw_phl_ps_send_battery_chg_hub_msg(void *phl, bool ips_allow, bool lps_allow)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *d = phl_to_drvpriv(phl_info);
	struct phl_msg msg = {0};
	struct phl_msg_attribute attr = {0};
	struct battery_chg_ntfy_info *binfo;
	enum rtw_phl_status pstatus;

	binfo = (struct battery_chg_ntfy_info *)_os_kmem_alloc(d, sizeof(*binfo));
	if (binfo == NULL) {
		PHL_ERR("%s: alloc ntfy fail.\n", __func__);
		return RTW_PHL_STATUS_RESOURCE;
	}

	binfo->ips_allow = ips_allow;
	binfo->lps_allow = lps_allow;

	msg.inbuf = (u8 *)binfo;
	msg.inlen = sizeof(*binfo);
	SET_MSG_MDL_ID_FIELD(msg.msg_id, PHL_MDL_POWER_MGNT);
	SET_MSG_EVT_ID_FIELD(msg.msg_id, MSG_EVT_BATTERY_CHG);
	msg.band_idx = HW_BAND_0;
	attr.completion.completion = _phl_ps_battery_chg_buf_done;
	attr.completion.priv = phl_info;

	pstatus = phl_msg_hub_send(phl_info, &attr, &msg);
	if(pstatus != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s: send msg_hub failed\n", __func__);
		_os_kmem_free(d, binfo, sizeof(struct battery_chg_ntfy_info));
	}
	return pstatus;
}

static void
_phl_ips_update_setting(struct phl_info_t *phl_info, bool ips_allow)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct rtw_ps_cap_t *ps_cap = &phl_com->dev_sw_cap.ps_cap;
	static u32 token = 0;

	if (ps_cap->ps_option & RTW_IPS_SKIP_BAT_CHG)
		return;

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[IPS] Update IPS: ips_allow: %d, token: %d\n",
		ips_allow, token);

	if(ips_allow) {
		if(token) {
			rtw_phl_ps_leave_ips_cancel(phl_info, token);
			token = 0;
		}
	} else {
		if(!token) {
			rtw_phl_ps_leave_ips_async(phl_info, &token, NULL, NULL,
				PHL_PS_NTFY_TYPE_BATTERY_CHG);
		}
	}
}

static void
_phl_lps_update_setting(struct phl_info_t *phl_info, bool lps_allow)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct rtw_ps_cap_t *ps_cap = &phl_com->dev_sw_cap.ps_cap;
	static u32 token = 0;

	if (ps_cap->ps_option & RTW_LPS_SKIP_BAT_CHG)
		return;

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[LPS] Update LPS: lps_allow: %d, token: %d\n",
		lps_allow, token);

	if(lps_allow) {
		if(token) {
			rtw_phl_ps_leave_lps_cancel(phl_info, token);
			token = 0;
		}
	} else {
		if(!token) {
			rtw_phl_ps_leave_lps_async(phl_info, &token, NULL, NULL,
				PHL_PS_NTFY_TYPE_BATTERY_CHG);
		}
	}
}

static enum rtw_phl_status
_phl_ps_battery_chg_notify(void *phl, void *param)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct battery_chg_ntfy_info *binfo;

	if (param == NULL)
		return RTW_PHL_STATUS_FAILURE;

	binfo = (struct battery_chg_ntfy_info *)param;

	_phl_ips_update_setting(phl_info, binfo->ips_allow);
	_phl_lps_update_setting(phl_info, binfo->lps_allow);

	return RTW_PHL_STATUS_SUCCESS;
}

static enum rtw_phl_status
_phl_ps_watchdog_notify(void *phl, bool start)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct ps_obj *ps = phl_info->ps_obj;
	u32 *token = &ps->token_list.token_watchdog;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;

	if(start) {
		if (*token == 0) {
			pstatus = phl_ps_req_pwr_sync(phl, token,
				PWRCMD_COMP_IO_RF, true,
				PHL_PS_NTFY_TYPE_WATCHDOG);
		}
	} else {

		if (*token) {
			pstatus = phl_ps_cancel_pwr_req(phl, *token);
			*token = 0;
		}
	}
	return pstatus;
}

static enum rtw_phl_status
_phl_ps_simple_watchdog_notify(void *phl, bool start)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct ps_obj *ps = phl_info->ps_obj;
	u32 *token = &ps->token_list.token_watchdog;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;

	if(start) {
		if (*token == 0) {
			pstatus = phl_ps_req_pwr_sync(phl, token,
				PWRCMD_COMP_IO_RF, false,
				PHL_PS_NTFY_TYPE_SIMPLE_WATCHDOG);
		}
	} else {

		if (*token) {
			pstatus = phl_ps_cancel_pwr_req(phl, *token);
			*token = 0;
		}
	}
	return pstatus;
}


static enum rtw_phl_status
_phl_ps_create_mac_notify(void *phl, bool start)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct ps_obj *ps = phl_info->ps_obj;
	u32 *token = &ps->token_list.token_create_mac;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	if(start) {
		pstatus = phl_ps_req_pwr_sync(phl, token, PWRCMD_COMP_IO_MAC,
						false, PHL_PS_NTFY_TYPE_CREATE_MAC);
	} else {
		if(*token)
			pstatus = phl_ps_cancel_pwr_req(phl, *token);
	}
	return pstatus;
}

static enum rtw_phl_status
_phl_ps_delete_mac_notify(void *phl, bool start)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct ps_obj *ps = phl_info->ps_obj;
	u32 *token = &ps->token_list.token_delete_mac;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	if(start) {
		pstatus = phl_ps_req_pwr_sync(phl, token, PWRCMD_COMP_IO_MAC,
						false, PHL_PS_NTFY_TYPE_DELETE_MAC);
	} else {
		if(*token)
			pstatus = phl_ps_cancel_pwr_req(phl, *token);
	}
	return pstatus;
}

static enum rtw_phl_status
_phl_ps_change_port_notify(void *phl, bool start)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct ps_obj *ps = phl_info->ps_obj;
	u32 *token = &ps->token_list.token_change_port;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	if(start) {
		pstatus = phl_ps_req_pwr_sync(phl, token, PWRCMD_COMP_IO_MAC,
						false, PHL_PS_NTFY_TYPE_CHANGE_PORT);
	} else {
		if(*token)
			pstatus = phl_ps_cancel_pwr_req(phl, *token);
	}
	return pstatus;
}

static enum rtw_phl_status
_phl_ps_connect_notify(void *phl, bool start)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct ps_obj *ps = phl_info->ps_obj;
	u32 *token = &ps->token_list.token_connect;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	if(start) {
		pstatus = phl_ps_leave_cmd_sync(phl, token, PHL_PS_NTFY_TYPE_CONNECT);
	} else {
		if(*token)
			pstatus = phl_ps_leave_cmd_cancel(phl, *token);
	}
	return pstatus;
}

static enum rtw_phl_status
_phl_ps_disassociate_notify(void *phl, bool start, void *param)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct ps_ntfy_info *ninfo;
	struct ps_obj *ps = phl_info->ps_obj;
	u32 *token = &ps->token_list.token_disassociate;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	if(start) {
		if (param == NULL)
			return RTW_PHL_STATUS_FAILURE;

		ninfo = (struct ps_ntfy_info *)param;
		if (ninfo->sync) {
			pstatus = phl_ps_leave_cmd_sync(phl, token, PHL_PS_NTFY_TYPE_DISASSOCIATE);
		} else {
			pstatus = phl_ps_leave_cmd_async(phl, token, ninfo->cb, ninfo->ctx,
                                                         	PHL_PS_NTFY_TYPE_DISASSOCIATE);
		}
	} else {
		if(*token)
			pstatus = phl_ps_leave_cmd_cancel(phl, *token);
	}
	return pstatus;
}

static enum rtw_phl_status
_phl_ps_start_ap_notify(void *phl, bool start)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct ps_obj *ps = phl_info->ps_obj;
	u32 *token = &ps->token_list.token_start_ap;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	if(start) {
		pstatus = phl_ps_leave_cmd_sync(phl, token, PHL_PS_NTFY_TYPE_START_AP);
	} else {
		if(*token)
			pstatus = phl_ps_leave_cmd_cancel(phl, *token);
	}
	return pstatus;
}

static enum rtw_phl_status
_phl_ps_driver_unload_notify(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct ps_obj *ps = phl_info->ps_obj;
	u32 token;

	/* leave all power saving */
	phl_ps_leave_cmd_sync(phl, &token, PHL_PS_NTFY_TYPE_DRIVER_UNLOAD);

	/* radio off => radio on */
	phl_pm_force_power_on(ps->pm_obj);

	return RTW_PHL_STATUS_SUCCESS;
}

/* For EXTERNAL application to notify ps module (expose)
 * @phl: refer to phl_info_t
 * @notify: notify id
 * @buf: input parameters
 */
enum rtw_phl_status
rtw_phl_ps_notify(void *phl, enum PHL_PS_NOTIFY notify, void *param)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	switch (notify) {
	case PHL_PS_NTFY_WATCHDOG_START:
		pstatus = _phl_ps_watchdog_notify(phl, true);
		break;
	case PHL_PS_NTFY_WATCHDOG_STOP:
		pstatus = _phl_ps_watchdog_notify(phl, false);
		break;
	case PHL_PS_NTFY_BATTERY_CHG:
		pstatus = _phl_ps_battery_chg_notify(phl, param);
		break;
	case PHL_PS_NTFY_CREATE_MAC_START:
		pstatus = _phl_ps_create_mac_notify(phl, true);
		break;
	case PHL_PS_NTFY_CREATE_MAC_END:
		pstatus = _phl_ps_create_mac_notify(phl, false);
		break;
	case PHL_PS_NTFY_DELETE_MAC_START:
		pstatus = _phl_ps_delete_mac_notify(phl, true);
		break;
	case PHL_PS_NTFY_DELETE_MAC_END:
		pstatus = _phl_ps_delete_mac_notify(phl, false);
		break;
	case PHL_PS_NTFY_CHANGE_PORT_START:
		pstatus = _phl_ps_change_port_notify(phl, true);
		break;
	case PHL_PS_NTFY_CHANGE_PORT_END:
		pstatus = _phl_ps_change_port_notify(phl, false);
		break;
	case PHL_PS_NTFY_CONNECT_START:
		pstatus = _phl_ps_connect_notify(phl, true);
		break;
	case PHL_PS_NTFY_CONNECT_STOP:
		pstatus = _phl_ps_connect_notify(phl, false);
		break;
	case PHL_PS_NTFY_DISASSOCIATE_START:
		pstatus = _phl_ps_disassociate_notify(phl, true, param);
		break;
	case PHL_PS_NTFY_DISASSOCIATE_STOP:
		pstatus = _phl_ps_disassociate_notify(phl, false, param);
		break;
	case PHL_PS_NTFY_START_AP_START:
		pstatus = _phl_ps_start_ap_notify(phl, true);
		break;
	case PHL_PS_NTFY_START_AP_END:
		pstatus = _phl_ps_start_ap_notify(phl, false);
		break;
	case PHL_PS_NTFY_DRIVER_UNLOAD:
		pstatus = _phl_ps_driver_unload_notify(phl);
		break;
	case PHL_PS_NTFY_SIMPLE_WATCHDOG_START:
		pstatus = _phl_ps_simple_watchdog_notify(phl, true);
		break;
	case PHL_PS_NTFY_SIMPLE_WATCHDOG_END:
		pstatus = _phl_ps_simple_watchdog_notify(phl, false);
		break;
	default:
		PHL_TRACE(COMP_PHL_PS, _PHL_DEBUG_,
			"%s: Unrecognize case: %d, please check it!\n",
			__func__, notify);
		break;
	}

	return pstatus;
}

/* For EXTERNAL application to notify role change (expose)
 * @phl: refer to phl_info_t
 * @role_id: role id
 * @rstate: role state
 */
enum rtw_phl_status
phl_ps_role_notify(void *phl, u8 *buf)
{
#ifdef PHL_INCLUDE_FSM
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct role_ntfy_info *rinfo = (struct role_ntfy_info *)buf;
	void *d = phl_to_drvpriv(phl_info);
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct ps_ntfy *ntfy;
	struct ps_role_info_param *ps_role;
	_os_event done;

	ntfy = _phl_ps_new_ntfy(phl_info);
	if (ntfy == NULL)
		return RTW_PHL_STATUS_FAILURE;

	_os_event_init(d, &done);

	ps_role = &ntfy->u.ps_role;
	ps_role->role_id = rinfo->role_id;
	ps_role->macid = rinfo->macid;
	ps_role->rstate = rinfo->rstate;
	ntfy->free_ntfy = true;
	ntfy->wait_done = true;
	ntfy->done = &done;

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[PS] Role change notify, role_id = %d, rstate = %d.\n",
		rinfo->role_id, rinfo->rstate);

	status = phl_ps_send_msg(phl, ntfy, sizeof(*ntfy), PS_EV_ROLE_NOTIFY);
	if(status != RTW_PHL_STATUS_SUCCESS) {
		_phl_ps_free_ntfy(phl_info, ntfy);
		return status;
	}

	return _phl_ps_completion(phl_info, ntfy, true, &done);
#else
	return RTW_PHL_STATUS_SUCCESS;
#endif /* PHL_INCLUDE_FSM */
}

/* For EXTERNAL application to notify ser status (expose)
 * @phl: refer to phl_info_t
 * @ser_start: ser is start or stop
 */
void phl_ps_ser_notify(void *phl, bool ser_start)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct ps_obj *ps = phl_info->ps_obj;

	ps->ser_ongoing = ser_start;
	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_, "[PS] ser_start = %d.\n", ser_start);
}

#ifdef RTW_WKARD_LPS_P2P_ROLE_TYPE
/* For EXTERNAL application to notify role change (expose)
 * @phl: refer to phl_info_t
 * @wrole: wifi role
 * @rtype: role real type
 */
enum rtw_phl_status
rtw_phl_ps_role_type_notify(void *phl, struct rtw_wifi_role_t *wrole,
	enum role_type rtype)
{
#ifdef PHL_INCLUDE_FSM
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *d = phl_to_drvpriv(phl_info);
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct ps_ntfy *ntfy;
	struct ps_role_info_param *ps_role;
	_os_event done;

	ntfy = _phl_ps_new_ntfy(phl_info);
	if (ntfy == NULL)
		return RTW_PHL_STATUS_FAILURE;

	_os_event_init(d, &done);

	ps_role = &ntfy->u.ps_role;
	ps_role->role_id = wrole->id;
	ps_role->rtype = rtype;
	ntfy->free_ntfy = true;
	ntfy->wait_done = true;
	ntfy->done = &done;

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[PS] Role type notify, role_id = %d, rtype = %d.\n",
		ps_role->role_id, ps_role->rtype);

	status = phl_ps_send_msg(phl, ntfy, sizeof(*ntfy),
			PS_EV_ROLE_TYPE_NOTIFY);
	if(status != RTW_PHL_STATUS_SUCCESS) {
		_phl_ps_free_ntfy(phl_info, ntfy);
		return status;
	}

	return _phl_ps_completion(phl_info, ntfy, true, &done);
#else
	return RTW_PHL_STATUS_SUCCESS;
#endif /* PHL_INCLUDE_FSM */
}
#endif

/* For EXTERNAL application to check the offset is valid or not (expose)
 * @phl: refer to phl_info_t
 * @offset: I/O offset
 */
enum rtw_phl_status
rtw_phl_ps_is_valid_access(void *phl, u32 offset)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct ps_obj *ps = phl_info->ps_obj;

	/* Allow all I/O when ser ongoing */
	if (ps->ser_ongoing)
		return RTW_PHL_STATUS_SUCCESS;

	if (phl_pm_is_low_power(ps->pm_obj) &&
		(rtw_hal_ps_lps_chk_access(phl_info->hal, offset)
		!= RTW_HAL_STATUS_SUCCESS))
		return RTW_PHL_STATUS_FAILURE;
	else
		return RTW_PHL_STATUS_SUCCESS;
}

/* For EXTERNAL application to issue ps command (expose)
 * @phl: refer to phl_info_t
 * @hdl: The handler represents this ps command
 * @cb: The callback function will be called after completed the command.
 * @ctx: The context will be input to callback function
 * @wait: Indicating if the caller wants to wait until command is completed.
 */
enum rtw_phl_status
phl_ps_issue_cmd(void *phl, u32 *token,
	void (*cb)(void *phl, void *hdl, void *ctx, enum rtw_phl_status stat),
	void *ctx, u8 wait, enum PHL_PS_NOTIFY_TYPE type)
{
#ifdef PHL_INCLUDE_FSM
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	void *d = phl_to_drvpriv(phl_info);
	struct ps_ntfy *ntfy;
	_os_event done;

	ntfy = _phl_ps_new_ntfy(phl_info);
	if (ntfy == NULL)
		return RTW_PHL_STATUS_FAILURE;

	_os_event_init(d, &done);

	ntfy->cb = cb;
	ntfy->ctx = ctx;
	ntfy->free_ntfy = true;
	ntfy->token = phl_ps_get_token(phl_info->ps_obj);
	ntfy->wait_done = wait;
	ntfy->done = &done;
	*token = ntfy->token;

	rtw_phl_ps_type_convert_to_string(type, ntfy->type_s);

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[PS] New ps cmd, token = %d.\n", ntfy->token);

	status = phl_ps_send_msg(phl, ntfy, sizeof(*ntfy), PS_EV_ISSUE_CMD);
	if(status != RTW_PHL_STATUS_SUCCESS) {
		_phl_ps_free_ntfy(phl_info, ntfy);
		*token = 0;
		return status;
	}

	return _phl_ps_completion(phl_info, ntfy, wait, &done);
#else
	return RTW_PHL_STATUS_SUCCESS;
#endif /* PHL_INCLUDE_FSM */
}

/* For EXTERNAL application to cancel ps command (expose)
 * @phl: refer to phl_info_t
 * @hdl: The handler represents this ps command to be canceled
 */
enum rtw_phl_status
phl_ps_cancel_cmd(void *phl, u32 token)
{
#ifdef PHL_INCLUDE_FSM
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *d = phl_to_drvpriv(phl_info);
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct ps_ntfy *ntfy;

	ntfy = _phl_ps_new_ntfy(phl_info);
	if (ntfy == NULL)
		return RTW_PHL_STATUS_FAILURE;

	ntfy->free_ntfy = true;
	ntfy->token = token;
	ntfy->wait_done = false;

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[PS] Cancel ps cmd, token = %d\n", token);

	status = phl_ps_send_msg(phl, ntfy, sizeof(*ntfy), PS_EV_CANCEL_CMD);
	if(status != RTW_PHL_STATUS_SUCCESS) {
		_phl_ps_free_ntfy(phl_info, ntfy);
		return status;
	}

	return RTW_PHL_STATUS_SUCCESS;
#else
	return RTW_PHL_STATUS_SUCCESS;
#endif /* PHL_INCLUDE_FSM */
}

u8 rtw_phl_ps_reject_roaming(struct phl_info_t *phl_info)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct rtw_ps_cap_t *ps_cap = &phl_com->dev_sw_cap.ps_cap;
	struct ps_obj *ps = phl_info->ps_obj;

	if ((ps_cap->ps_option & RTW_LPS_REJECT_ROAM) &&
		phl_lps_is_ongoing(ps->lps_obj))
		return true;
	else
		return false;
}

enum rtw_phl_status
phl_ps_wow_link_ptcl_cfg(struct phl_info_t *phl_info, bool en, u16 macid)
{
	struct ps_obj *ps = phl_info->ps_obj;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct rtw_ps_cap_t *ps_cap = &phl_com->dev_sw_cap.ps_cap;

	if (!ps_cap->lps_wow_en) {
		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
			"[LPS] WoWLAN LPS is disabled.\n");
		return RTW_PHL_STATUS_SUCCESS;
	}

	return phl_lps_wow_cfg(ps->lps_obj, en, macid);
}

enum rtw_phl_status
phl_ps_wow_link_pwr_cfg(struct phl_info_t *phl_info)
{
	struct ps_obj *ps = phl_info->ps_obj;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct rtw_ps_cap_t *ps_cap = &phl_com->dev_sw_cap.ps_cap;

	if (!ps_cap->lps_wow_en) {
		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
			"[LPS] WoWLAN LPS is disabled.\n");
		return RTW_PHL_STATUS_SUCCESS;
	}

	return phl_pm_wow_cfg(ps->pm_obj, ps_cap->lps_wow_cap);
}

enum rtw_phl_status
phl_ps_force_hw_leave_low_pwr(struct phl_info_t *phl_info)
{
	struct ps_obj *ps = phl_info->ps_obj;

	return phl_pm_wow_cfg(ps->pm_obj, HWPS_CAP_PWRON);
}

void phl_ps_watchdog(struct phl_info_t *phl_info)
{
	struct ps_obj *ps = phl_info->ps_obj;

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
			"[PS] %s ==> \n", __func__);

	phl_pm_watchdog(ps->pm_obj);
	phl_lps_watchdog(ps->lps_obj);

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
			"[PS] %s <== \n", __func__);

}
#endif /* CONFIG_PS */

#ifdef CONFIG_PS_PM

/* For EXTERNAL application to issue power command (expose)
 * @phl: refer to phl_info_t
 * @hdl: The handler represents this power command
 * @comp: The HW block this power command required
 * @cb: The callback function will be called after power state is met.
 * @ctx: The context will be input to callback function
 * @test: Test the current power state and return RTW_PHL_STATUS_RESOURCE if power state cannot meet request
 * @wait: Indicating if the caller wants to wait until power level is reached.
 */
enum rtw_phl_status
phl_ps_issue_pwr_cmd(void *phl, u32 *token, u32 comp,
	void (*cb)(void *phl, void *token, void *ctx, enum rtw_phl_status stat),
	void *ctx, u8 test, u8 wait, enum PHL_PS_NOTIFY_TYPE type)
{
#ifdef PHL_INCLUDE_FSM
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *d = phl_to_drvpriv(phl_info);
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct ps_ntfy *ntfy;
	_os_event done;

	if(test && false == phl_ps_test_pwr_level(phl_info, comp)) {
		return RTW_PHL_STATUS_RESOURCE;
	}

	ntfy = _phl_ps_new_ntfy(phl_info);
	if (ntfy == NULL)
		return RTW_PHL_STATUS_FAILURE;

	_os_event_init(d, &done);

	ntfy->cb = cb;
	ntfy->ctx = ctx;
	ntfy->free_ntfy = true;
	ntfy->token = phl_ps_get_token(phl_info->ps_obj);
	ntfy->u.pm_comp = comp;
	ntfy->wait_done = wait;
	ntfy->done = &done;

	rtw_phl_ps_type_convert_to_string(type, ntfy->type_s);

	*token = ntfy->token;

	PHL_TRACE(COMP_PHL_PS, _PHL_DEBUG_,
		"[PM] New power cmd: token = %d, type = %s, comp = %x\n",
		ntfy->token, ntfy->type_s, comp);

	status = phl_ps_send_msg(phl, ntfy, sizeof(*ntfy), PM_EV_ISSUE_CMD);
	if(status != RTW_PHL_STATUS_SUCCESS) {
		_phl_ps_free_ntfy(phl_info, ntfy);
		*token = 0;
		return status;
	}

	return _phl_ps_completion(phl_info, ntfy, wait, &done);
#else
	return RTW_PHL_STATUS_SUCCESS;
#endif /* PHL_INCLUDE_FSM */
}

/* For EXTERNAL application to cancel power command (expose)
 * @phl: refer to phl_info_t
 * @hdl: The power command to be canceled
 */
enum rtw_phl_status
phl_ps_cancel_pwr_cmd(void *phl, u32 token)
{
#ifdef PHL_INCLUDE_FSM
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *d = phl_to_drvpriv(phl_info);
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct ps_ntfy *ntfy;

	ntfy = _phl_ps_new_ntfy(phl_info);
	if (ntfy == NULL)
		return RTW_PHL_STATUS_FAILURE;

	ntfy->free_ntfy = true;
	ntfy->token = token;
	ntfy->wait_done = false;

	PHL_TRACE(COMP_PHL_PS, _PHL_DEBUG_,
		"[PM] Cancel power cmd: token = %d\n", token);

	status = phl_ps_send_msg(phl, ntfy, sizeof(*ntfy), PM_EV_CANCEL_CMD);
	if(status != RTW_PHL_STATUS_SUCCESS) {
		_phl_ps_free_ntfy(phl_info, ntfy);
	}

	return _phl_ps_completion(phl_info, ntfy, false, NULL);
#else
	return RTW_PHL_STATUS_SUCCESS;
#endif /* PHL_INCLUDE_FSM */
}

/* For EXTERNAL application to set radio state (expose)
 * @phl: refer to phl_info_t
 * @rf_state: refer to rtw_rf_state
 */
enum rtw_phl_status
phl_ps_set_radio_state(void *phl, enum rtw_rf_state rf_state)
{
#ifdef PHL_INCLUDE_FSM
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *d = phl_to_drvpriv(phl_info);
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct ps_ntfy *ntfy;
	_os_event done;

	ntfy = _phl_ps_new_ntfy(phl_info);
	if (ntfy == NULL)
		return RTW_PHL_STATUS_FAILURE;

	_os_event_init(d, &done);

	ntfy->free_ntfy = true;
	ntfy->u.rf_state = rf_state;
	ntfy->wait_done = true;
	ntfy->done = &done;

	status = phl_ps_send_msg(phl, ntfy, sizeof(*ntfy), PM_EV_SET_RADIO);
	if(status != RTW_PHL_STATUS_SUCCESS) {
		_phl_ps_free_ntfy(phl_info, ntfy);
		return status;
	}

	return _phl_ps_completion(phl_info, ntfy, true, &done);
#else
	return RTW_PHL_STATUS_SUCCESS;
#endif /* PHL_INCLUDE_FSM */
}

#if PS_TEST
/* For EXTERNAL application to issue power saving capability (expose)
 * @phl: refer to phl_info_t
 * @hdl: The handler represents this power saving capbility
 * @cap: The HW state this power saving capbility claimed
 */
enum rtw_phl_status
phl_ps_issue_pwr_cap(void *phl, u32 *token, u32 cap)
{
#ifdef PHL_INCLUDE_FSM
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *d = phl_to_drvpriv(phl_info);
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct ps_ntfy *ntfy;

	ntfy = _phl_ps_new_ntfy(phl_info);
	if (ntfy == NULL)
		return RTW_PHL_STATUS_FAILURE;

	ntfy->free_ntfy = true;
	ntfy->token = phl_ps_get_token(phl_info->ps_obj);
	ntfy->wait_done = false;
	ntfy->u.pm_cap = cap;
	*token = ntfy->token;

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[PM] New power cap: token = %d, cap = %x\n", ntfy->token, cap);

	status = phl_ps_send_msg(phl, ntfy, sizeof(*ntfy), PM_EV_ISSUE_CAP);
	if(status != RTW_PHL_STATUS_SUCCESS) {
		_phl_ps_free_ntfy(phl_info, ntfy);
		*token = 0;
	}

	return _phl_ps_completion(phl_info, ntfy, false, NULL);
#else
	return RTW_PHL_STATUS_SUCCESS;
#endif /* PHL_INCLUDE_FSM */
}

/* For EXTERNAL application to cancel power saving capability (expose)
 * @phl: refer to phl_info_t
 * @hdl: The handler represents this power saving capbility
 */
enum rtw_phl_status
phl_ps_cancel_pwr_cap(void *phl, u32 token)
{
#ifdef PHL_INCLUDE_FSM
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *d = phl_to_drvpriv(phl_info);
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct ps_ntfy *ntfy;

	ntfy = _phl_ps_new_ntfy(phl_info);
	if (ntfy == NULL)
		return RTW_PHL_STATUS_FAILURE;

	ntfy->free_ntfy = true;
	ntfy->token = token;
	ntfy->wait_done = false;

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[PM] Cancel power cap: token = %d\n", token);

	status = phl_ps_send_msg(phl, ntfy, sizeof(*ntfy), PM_EV_CANCEL_CAP);
	if(status != RTW_PHL_STATUS_SUCCESS) {
		_phl_ps_free_ntfy(phl_info, ntfy);
	}

	return _phl_ps_completion(phl_info, ntfy, false, NULL);
#else
	return RTW_PHL_STATUS_SUCCESS;
#endif /* PHL_INCLUDE_FSM */
}

#endif /*    PS_TEST */

#endif /* CONFIG_PS_PM */

#ifdef CONFIG_PS_IPS

/* For EXTERNAL application to issue ips command (expose)
 * @phl: refer to phl_info_t
 * @token: The token represents this ips command
 * @cb: The callback function will be called after completed the command.
 * @ctx: The context will be input to callback function
 * @wait: Indicating if the caller wants to wait until command is completed.
 */
enum rtw_phl_status
rtw_phl_ps_issue_ips_cmd(void *phl, u32 *token,
	void (*cb)(void *phl, void *hdl, void *ctx, enum rtw_phl_status stat),
	void *ctx, u8 wait, enum PHL_PS_NOTIFY_TYPE type)
{
#ifdef PHL_INCLUDE_FSM
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *d = phl_to_drvpriv(phl_info);
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct ps_ntfy *ntfy;
	_os_event done;

	ntfy = _phl_ps_new_ntfy(phl_info);
	if (ntfy == NULL)
		return RTW_PHL_STATUS_FAILURE;

	_os_event_init(d, &done);

	ntfy->cb = cb;
	ntfy->ctx = ctx;
	ntfy->free_ntfy = true;
	ntfy->token = phl_ps_get_token(phl_info->ps_obj);
	ntfy->wait_done = wait;
	ntfy->done = &done;
	*token = ntfy->token;

	rtw_phl_ps_type_convert_to_string(type, ntfy->type_s);

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[IPS] New ips cmd, token = %d.\n", ntfy->token);

	status = phl_ps_send_msg(phl, ntfy, sizeof(*ntfy), IPS_EV_ISSUE_CMD);
	if(status != RTW_PHL_STATUS_SUCCESS) {
		_phl_ps_free_ntfy(phl_info, ntfy);
		*token = 0;
		return status;
	}

	return _phl_ps_completion(phl_info, ntfy, wait, &done);
#else
	return RTW_PHL_STATUS_SUCCESS;
#endif /* PHL_INCLUDE_FSM */
}

/* For EXTERNAL application to cancel ips command (expose)
 * @phl: refer to phl_info_t
 * @token: The token represents this ips command to be canceled
 */
enum rtw_phl_status
rtw_phl_ps_cancel_ips_cmd(void *phl, u32 token)
{
#ifdef PHL_INCLUDE_FSM
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *d = phl_to_drvpriv(phl_info);
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct ps_ntfy *ntfy;

	ntfy = _phl_ps_new_ntfy(phl_info);
	if (ntfy == NULL)
		return RTW_PHL_STATUS_FAILURE;

	ntfy->free_ntfy = true;
	ntfy->token = token;
	ntfy->wait_done = false;

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[IPS] Cancel ips cmd, token = %d\n", token);

	status = phl_ps_send_msg(phl, ntfy, sizeof(*ntfy), IPS_EV_CANCEL_CMD);
	if(status != RTW_PHL_STATUS_SUCCESS) {
		_phl_ps_free_ntfy(phl_info, ntfy);
	}

	return _phl_ps_completion(phl_info, ntfy, false, NULL);
#else
	return RTW_PHL_STATUS_SUCCESS;
#endif /* PHL_INCLUDE_FSM */
}

#endif /* CONFIG_PS_IPS */

#ifdef CONFIG_PS_LPS

/* For EXTERNAL application to issue lps command (expose)
 * @phl: refer to phl_info_t
 * @hdl: The handler represents this lps command
 * @cb: The callback function will be called after completed the command.
 * @ctx: The context will be input to callback function
 * @wait: Indicating if the caller wants to wait until command is completed.
 */
enum rtw_phl_status
rtw_phl_ps_issue_lps_cmd(void *phl, u32 *token,
	void (*cb)(void *phl, void *hdl, void *ctx, enum rtw_phl_status stat),
	void *ctx, u8 wait, enum PHL_PS_NOTIFY_TYPE type)
{
#ifdef PHL_INCLUDE_FSM
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *d = phl_to_drvpriv(phl_info);
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct ps_ntfy *ntfy;
	_os_event done;

	ntfy = _phl_ps_new_ntfy(phl_info);
	if (ntfy == NULL)
		return RTW_PHL_STATUS_FAILURE;

	_os_event_init(d, &done);

	ntfy->cb = cb;
	ntfy->ctx = ctx;
	ntfy->free_ntfy = true;
	ntfy->token = phl_ps_get_token(phl_info->ps_obj);
	ntfy->wait_done = wait;
	ntfy->done = &done;
	*token = ntfy->token;

	rtw_phl_ps_type_convert_to_string(type, ntfy->type_s);

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[LPS] New lps cmd, token = %d.\n", ntfy->token);

	status = phl_ps_send_msg(phl, ntfy, sizeof(*ntfy), LPS_EV_ISSUE_CMD);
	if(status != RTW_PHL_STATUS_SUCCESS) {
		_phl_ps_free_ntfy(phl_info, ntfy);
		*token = 0;
		return status;
	}

	return _phl_ps_completion(phl_info, ntfy, wait, &done);
#else
	return RTW_PHL_STATUS_SUCCESS;
#endif /* PHL_INCLUDE_FSM */
}

/* For EXTERNAL application to cancel lps command (expose)
 * @phl: refer to phl_info_t
 * @hdl: The handler represents this lps command to be canceled
 */
enum rtw_phl_status
rtw_phl_ps_cancel_lps_cmd(void *phl, u32 token)
{
#ifdef PHL_INCLUDE_FSM
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *d = phl_to_drvpriv(phl_info);
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct ps_ntfy *ntfy;

	ntfy = _phl_ps_new_ntfy(phl_info);
	if (ntfy == NULL)
		return RTW_PHL_STATUS_FAILURE;

	ntfy->free_ntfy = true;
	ntfy->token = token;
	ntfy->wait_done = false;

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[LPS] Cancel lps cmd, token = %d\n", token);

	status = phl_ps_send_msg(phl, ntfy, sizeof(*ntfy), LPS_EV_CANCEL_CMD);
	if(status != RTW_PHL_STATUS_SUCCESS) {
		_phl_ps_free_ntfy(phl_info, ntfy);
	}

	return _phl_ps_completion(phl_info, ntfy, false, NULL);
#else
	return RTW_PHL_STATUS_SUCCESS;
#endif /* PHL_INCLUDE_FSM */
}

#endif /* CONFIG_PS_LPS */

