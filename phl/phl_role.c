/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
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
#define _PHL_ROLE_C_
#include "phl_headers.h"


static enum rtw_phl_status
_phl_alloc_hw_port(struct phl_info_t *phl,
					struct rtw_wifi_role_t *wifi_role)
{
	struct rtw_phl_com_t *phl_com = phl->phl_com;
	struct hal_spec_t *hal_spec = phl_get_ic_spec(phl_com);
	struct mr_ctl_t *mr_ctl = phlcom_to_mr_ctrl(phl_com);
	struct hw_band_ctl_t *band_ctrl;
	u8 max_port_num = hal_spec->port_num;
	u8 i = max_port_num;

	band_ctrl = &mr_ctl->band_ctrl[wifi_role->hw_band];

	_os_spinlock(phl_to_drvpriv(phl), &band_ctrl->lock, _ps, NULL);
	for (i = 0; i < max_port_num; i++) {
		if (!(band_ctrl->port_map & BIT(i))) {
			band_ctrl->port_map |= BIT(i);
			break;
		}
	}
	_os_spinunlock(phl_to_drvpriv(phl), &band_ctrl->lock, _ps, NULL);

	if (i == max_port_num) {
		PHL_ERR("%s Can't get port idx\n", __func__);
		return RTW_PHL_STATUS_FAILURE;
	}
	wifi_role->hw_port = i;
	return RTW_PHL_STATUS_SUCCESS;
}

static enum rtw_phl_status
_phl_release_hw_port(struct phl_info_t *phl,
				struct rtw_wifi_role_t *wifi_role)
{
	struct rtw_phl_com_t *phl_com = phl->phl_com;
	struct mr_ctl_t *mr_ctl = phlcom_to_mr_ctrl(phl_com);
	struct hw_band_ctl_t *band_ctrl;

	band_ctrl = &mr_ctl->band_ctrl[wifi_role->hw_band];

	_os_spinlock(phl_to_drvpriv(phl), &band_ctrl->lock, _ps, NULL);
	band_ctrl->port_map &= ~BIT(wifi_role->hw_port);
	_os_spinunlock(phl_to_drvpriv(phl), &band_ctrl->lock, _ps, NULL);
	return RTW_PHL_STATUS_SUCCESS;
}

static enum rtw_phl_status
_phl_update_hw_resource(struct phl_info_t *phl,
					struct rtw_wifi_role_t *wifi_role)
{
	struct rtw_phl_com_t *phl_com = phl->phl_com;

	return RTW_PHL_STATUS_SUCCESS;
}

static enum rtw_phl_status
_phl_alloc_hw_resource(struct phl_info_t *phl,
					struct rtw_wifi_role_t *wifi_role)
{
	struct rtw_phl_com_t *phl_com = phl->phl_com;

	wifi_role->hw_band = 0;
	phl_mr_role_map_set(phl, wifi_role);
#ifdef CONFIG_DBCC_SUPPORT
	if ((phl_com->dev_cap.hw_sup_flags & HW_SUP_DBCC) && (phl_com->dev_cap.dbcc_sup)) {
		wifi_role->hw_band = 0;
		//TODO
	}
#endif
	/*alloc  hw_port*/
	if (_phl_alloc_hw_port(phl, wifi_role) != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s alloc_hw_port failed\n", __func__);
		return INVALID_WIFI_ROLE_IDX;
	}

	return RTW_PHL_STATUS_SUCCESS;
}

static enum rtw_phl_status
_phl_free_hw_resource(struct phl_info_t *phl,
					struct rtw_wifi_role_t *wifi_role)
{
	struct rtw_phl_com_t *phl_com = phl->phl_com;

	/*release hw_port*/
	if (_phl_release_hw_port(phl, wifi_role) != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s _phl_release_hw_port failed\n", __func__);
		return INVALID_WIFI_ROLE_IDX;
	}
	phl_mr_role_map_clr(phl, wifi_role);

	return RTW_PHL_STATUS_SUCCESS;
}

static u8 _phl_search_roleidx_by_addr(
			struct phl_info_t *phl_info, u8 *mac_addr)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	u8 ridx = MAX_WIFI_ROLE_NUMBER;
	struct mr_ctl_t *mr_ctl = phlcom_to_mr_ctrl(phl_com);

	for (ridx = 0; ridx < MAX_WIFI_ROLE_NUMBER; ridx++) {
		if (mr_ctl->role_map & BIT(ridx)) {
			if (_os_mem_cmp(phl_to_drvpriv(phl_info),
				phl_com->wifi_roles[ridx].mac_addr,
				mac_addr, MAC_ALEN) == 0)
				break;
		}
	}

	#if 0
	if (ridx == MAX_WIFI_ROLE_NUMBER)
		PHL_INFO("%s cannot get rid\n", __func__);
	#endif
	return ridx;
}


struct rtw_wifi_role_t *
phl_get_wrole_by_addr(struct phl_info_t *phl_info, u8 *mac_addr)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	int ridx = MAX_WIFI_ROLE_NUMBER;
	struct mr_ctl_t *mr_ctl = phlcom_to_mr_ctrl(phl_com);
	struct rtw_wifi_role_t *wrole = NULL;
	bool found = false;

	for (ridx = 0; ridx < MAX_WIFI_ROLE_NUMBER; ridx++) {
		if (mr_ctl->role_map & BIT(ridx)) {
			wrole = &(phl_com->wifi_roles[ridx]);
			if (_os_mem_cmp(phl_to_drvpriv(phl_info),
				wrole->mac_addr, mac_addr, MAC_ALEN) == 0) {
				found = true;
				break;
			}
		}
	}

	if (found == false)
		wrole = NULL;

	return wrole;
}

struct rtw_wifi_role_t *
phl_get_wrole_by_ridx(struct phl_info_t *phl_info, u8 rold_idx)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;

	if (rold_idx < MAX_WIFI_ROLE_NUMBER)
		return &(phl_com->wifi_roles[rold_idx]);

	return NULL;
}


static void
_phl_role_notify_buf_done(void* priv, struct phl_msg* msg)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)priv;

	if(msg->inbuf && msg->inlen){
		_os_mem_free(phl_to_drvpriv(phl_info), msg->inbuf, msg->inlen);
	}
}

static void
_phl_send_role_notify_hub_msg(struct phl_info_t *phl_info,
			struct rtw_wifi_role_t *wrole, enum role_state rstate)
{
	struct phl_msg msg = {0};
	struct phl_msg_attribute attr = {0};
	struct role_ntfy_info *rinfo;
	struct rtw_phl_stainfo_t *stainfo = NULL;
	void *d = phl_to_drvpriv(phl_info);

	stainfo = rtw_phl_get_stainfo_self(phl_info, wrole);
	if(stainfo == NULL) {
		PHL_ERR("%s stainfo is NULL\n", __func__);
		return;
	}

	rinfo = (struct role_ntfy_info *)_os_mem_alloc(d, sizeof(*rinfo));
	if (rinfo == NULL) {
		PHL_ERR("%s: alloc ntfy fail.\n", __func__);
		return;
	}
	_os_mem_set(d, rinfo, 0, sizeof(*rinfo));

	rinfo->role_id = wrole->id;
	rinfo->macid = stainfo->macid;
	rinfo->rstate = rstate;

	msg.inbuf = (u8 *)rinfo;
	msg.inlen = sizeof(*rinfo);
	SET_MSG_MDL_ID_FIELD(msg.msg_id, PHL_MDL_MRC);
	SET_MSG_EVT_ID_FIELD(msg.msg_id, MSG_EVT_ROLE_NTFY);
	msg.band_idx = HW_BAND_0;
	attr.completion.completion = _phl_role_notify_buf_done;
	attr.completion.priv = phl_info;
	if (phl_msg_hub_send(phl_info, &attr, &msg) != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s: send msg_hub failed\n", __func__);
		_os_mem_free(d, rinfo, sizeof(struct role_ntfy_info));
	}
}

static enum rtw_phl_status
_phl_role_notify(struct phl_info_t *phl_info,
		struct rtw_wifi_role_t *wrole,
		enum role_state rstate)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;

	if (rstate >= PHL_ROLE_STATE_UNKNOWN) {
		PHL_ERR("%s unknow rstate:%d\n", __func__, rstate);
		return RTW_PHL_STATUS_FAILURE;
	}

	#ifdef CONFIG_BTCOEX
	rtw_phl_btc_role_notify(phl_info, wrole->id, rstate);
	#endif
	_phl_send_role_notify_hub_msg(phl_info, wrole, rstate);

	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status
_phl_wifi_role_start(struct phl_info_t *phl_info, struct rtw_wifi_role_t *wrole)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct rtw_phl_stainfo_t *phl_sta = NULL;

	RTW_INFO("%s NEO TODO rtw_hal_role_cfg\n", __func__);

#if 0 // NEO
	if (rtw_hal_role_cfg(phl_info->hal, wrole) != RTW_HAL_STATUS_SUCCESS) {
		PHL_ERR("%s hal role cfg failed\n", __func__);
		return RTW_PHL_STATUS_FAILURE;
	}
#endif // if 0 NEO

	/*alloc sta_info for slef*/
	phl_sta = rtw_phl_alloc_stainfo((void *)phl_info, wrole->mac_addr,
						wrole);

	if (phl_sta == NULL) {
		PHL_ERR("%s alloc sta failed\n", __func__);
		return RTW_PHL_STATUS_FAILURE;
	}

	return RTW_PHL_STATUS_SUCCESS;
}

u8 rtw_phl_wifi_role_alloc(void *phl, u8 *mac_addr, enum role_type type,
				u8 ridx, struct rtw_wifi_role_t **wifi_role)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct rtw_wifi_role_t *role = NULL;
	struct mr_ctl_t *mr_ctl = phlcom_to_mr_ctrl(phl_com);
	u8 role_idx = MAX_WIFI_ROLE_NUMBER;


	if (mac_addr == NULL) {
		PHL_ERR("%s mac_addr == NULL\n", __func__);
		return role_idx;
	}

	RTW_INFO("%s NEO" MAC_FMT "\n", __func__, MAC_ARG(mac_addr));
	/*search rid by mac_addr, if had assigned then return role index*/
	role_idx = _phl_search_roleidx_by_addr(phl_info, mac_addr);
	if (role_idx != INVALID_WIFI_ROLE_IDX) {
		PHL_INFO("%s wifi-role(%d) had allociated\n", __func__, role_idx);
		*wifi_role = &phl_com->wifi_roles[role_idx];
		return role_idx;
	}
	RTW_INFO("%s NEO role_idx=%d\n", __func__, role_idx);

	_os_spinlock(phl_to_drvpriv(phl_info), &mr_ctl->lock, _ps, NULL);
	if (ridx == UNSPECIFIED_ROLE_ID) {
		/*search avaliable rid */
		for (role_idx = 0; role_idx < MAX_WIFI_ROLE_NUMBER; role_idx++) {
			if (!(mr_ctl->role_map & BIT(role_idx))) {
				mr_ctl->role_map |= BIT(role_idx);
				break;
			}
		}
	} else {
		if (mr_ctl->role_map & BIT(ridx)) {
			PHL_ERR("role_idx:%d has used\n", ridx);
			_os_warn_on(1);
		} else {
			mr_ctl->role_map |= BIT(ridx);
			role_idx = ridx;
		}
	}
	_os_spinunlock(phl_to_drvpriv(phl_info), &mr_ctl->lock, _ps, NULL);

	if (role_idx == MAX_WIFI_ROLE_NUMBER) {
		PHL_ERR("%s Can't get role idx\n", __func__);
		return INVALID_WIFI_ROLE_IDX;
	}

	role = &phl_com->wifi_roles[role_idx];

	/*alloc hw_band and hw_port*/
	if (_phl_alloc_hw_resource(phl_info, role) != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s alloc_hw_band failed\n", __func__);
		return INVALID_WIFI_ROLE_IDX;
	}

	role->type = type;
	role->mstate = MLME_NO_LINK;
	role->chanctx = NULL;

	#ifdef RTW_WKARD_STA_BCN_INTERVAL
	#ifdef RTW_PHL_BCN
	if (role->type == PHL_RTYPE_AP) {
		role->bcn_cmn.bcn_interval = 100;
		role->mstate = MLME_LINKED;
	}
	#endif
	#endif

	_os_mem_cpy(phl_to_drvpriv(phl_info),
			role->mac_addr, mac_addr, MAC_ALEN);

	phl_init_role_cap(phl_info, role);
	phl_init_protocol_cap(phl_info, role);

	if (_phl_wifi_role_start(phl_info, role) != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s wifi_role_start failed\n", __func__);
		return INVALID_WIFI_ROLE_IDX;
	}
	role->active = true;
	_phl_role_notify(phl_info, role, PHL_ROLE_START);

	*wifi_role = role;
	PHL_DUMP_MR_EX(phl_info);
	return role_idx;
}

enum rtw_phl_status
phl_role_notify(struct phl_info_t *phl_info, struct rtw_wifi_role_t *wrole)
{
	RTW_ERR("%s NEO TODO\n", __func__);
	return PHL_ROLE_STATE_UNKNOWN;
#if 0
	enum role_state rstate = PHL_ROLE_STATE_UNKNOWN;

	switch (wrole->type) {
	case PHL_RTYPE_STATION:
	case PHL_RTYPE_P2P_GC:
	{
		if (wrole->mstate == MLME_LINKING)
			rstate = PHL_ROLE_MSTS_STA_CONN_START;
		else if (wrole->mstate == MLME_LINKED)
			rstate = PHL_ROLE_MSTS_STA_CONN_END;
		else
			rstate = PHL_ROLE_MSTS_STA_DIS_CONN;
	}
	break;

	case PHL_RTYPE_AP:
	case PHL_RTYPE_MESH:
	case PHL_RTYPE_P2P_GO:
	{
		rstate = (wrole->mstate == MLME_NO_LINK)
			? PHL_ROLE_MSTS_AP_STOP
			: PHL_ROLE_MSTS_AP_START;
	}
	break;

	case PHL_RTYPE_NONE:
	case PHL_RTYPE_VAP:
	case PHL_RTYPE_ADHOC:
	case PHL_RTYPE_ADHOC_MASTER:
	case PHL_RTYPE_MONITOR:
	case PHL_RTYPE_P2P_DEVICE:
	case PHL_RTYPE_NAN:
	case PHL_MLME_MAX:
		PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_,
			"%s: Unsupported case:%d in WR_CHG_MSTATE, please check it\n",
			__func__, wrole->type);
		break;
	default:
		PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_,
			"%s role-type(%d) not support WR_CHG_MSTATE notify\n",
			__func__, wrole->type);
		break;
	}

	_phl_role_notify(phl_info, wrole, rstate);
	return RTW_PHL_STATUS_SUCCESS;
#endif // if 0
}

#if 0 // NEO TODO

/**
 * This function is called once wifi info changed
 * (see enum wr_chg_id)
 * @phl: see phl_info_t
 * @wrole: information is updated for this wifi role
 * @chg_id: see enum wr_chg_id
 * @chg_info: the change info to be update
 */
enum rtw_phl_status
rtw_phl_wifi_role_change(void *phl, struct rtw_wifi_role_t *wrole,
				enum wr_chg_id chg_id, void *chg_info)
{
	enum rtw_phl_status pstate = RTW_PHL_STATUS_FAILURE;
	enum rtw_hal_status hstate;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_stainfo_t *sta = NULL;
	enum phl_upd_mode mode = PHL_UPD_ROLE_MAX;
	void *drv = phl_to_drvpriv(phl_info);

	switch (chg_id) {
	case WR_CHG_TYPE:
	{
		enum role_type type = *(enum role_type *)chg_info;

		if (wrole->type == type) {
			PHL_WARN("wrole type(%d) not change\n", wrole->type);
			pstate = RTW_PHL_STATUS_SUCCESS;
			return pstate;
		}
		PHL_INFO("wrole type(%d) change to type(%d)\n",
							wrole->type, type);

		sta = rtw_phl_get_stainfo_self(phl_info, wrole);
		if (sta) {
			wrole->type = type;
			wrole->mstate = MLME_NO_LINK;

			#ifdef RTW_WKARD_STA_BCN_INTERVAL
			#ifdef RTW_PHL_BCN
			if (wrole->type == PHL_RTYPE_AP) {
				wrole->bcn_cmn.bcn_interval = 100;
				wrole->mstate = MLME_LINKED;
				wrole->hiq_win = 16; /* unit: ms */
			}
			#endif
			#endif
			phl_init_role_cap(phl_info, wrole);
			phl_init_protocol_cap(phl_info, wrole);

			rtw_hal_role_cfg(phl_info->hal, wrole);

			mode = PHL_UPD_ROLE_TYPE_CHANGE;
			_os_mem_cpy(drv, sta->mac_addr, wrole->mac_addr, MAC_ALEN);
			pstate = phl_change_stainfo(phl_info, sta, mode);
			/*rtw_hal_change_sta_entry(phl_info->hal, sta);*/
			_phl_role_notify(phl_info, wrole, PHL_ROLE_CHG_TYPE);
		}
		else {
			PHL_ERR("cannot get stainfo_self\n");
		}
		PHL_DUMP_MR_EX(phl_info);
	}
		break;

	case WR_CHG_MSTATE:
	{
		#if 0
		enum role_state rstate = PHL_ROLE_STATE_UNKNOWN;
		enum mlme_state msts = *(enum mlme_state *)chg_info;

		PHL_INFO("wrole mstate(%d) WR_CHG_MSTATE mstate(%d)\n",
				wrole->mstate, msts);

		switch (wrole->type) {
		case PHL_RTYPE_STATION:
		case PHL_RTYPE_P2P_GC:
		{
			if (msts == MLME_LINKING)
				rstate = PHL_ROLE_MSTS_STA_CONN_START;
			else if (msts == MLME_LINKED)
				rstate = PHL_ROLE_MSTS_STA_CONN_END;
			else
				rstate = PHL_ROLE_MSTS_STA_DIS_CONN;
		}
		break;

		case PHL_RTYPE_AP:
		case PHL_RTYPE_MESH:
		case PHL_RTYPE_P2P_GO:
		{
			rstate = (msts == MLME_NO_LINK)
				? PHL_ROLE_MSTS_AP_STOP
				: PHL_ROLE_MSTS_AP_START;
		}
		break;

		case PHL_RTYPE_NONE:
		case PHL_RTYPE_VAP:
		case PHL_RTYPE_ADHOC:
		case PHL_RTYPE_ADHOC_MASTER:
		case PHL_RTYPE_MONITOR:
		case PHL_RTYPE_P2P_DEVICE:
		case PHL_RTYPE_NAN:
		case PHL_MLME_MAX:
			PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_, "rtw_phl_wifi_role_change(): Unsupported case:%d in WR_CHG_MSTATE, please check it\n",
					wrole->type);
			break;
		default:
			PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_, "%s role-type(%d) not support WR_CHG_MSTATE notify\n",
					__func__, wrole->type);
			break;
		}

		_phl_role_notify(phl_info, wrole, rstate);
		phl_mr_info_upt(phl_info, wrole);
		#endif

		pstate = RTW_PHL_STATUS_SUCCESS;
	}
		break;
	case WR_CHG_MADDR:
	{
		u8 *maddr = (u8 *)chg_info;

		if(_os_mem_cmp(drv, wrole->mac_addr, maddr,  MAC_ALEN) == 0) {
			PHL_WARN("wrole maddr %02x:%02x:%02x:%02x:%02x:%02x not change\n",
			         maddr[0], maddr[1], maddr[2],
			         maddr[3], maddr[4], maddr[5]);
			return RTW_PHL_STATUS_SUCCESS;
		}
		PHL_INFO("wrole maddr %02x:%02x:%02x:%02x:%02x:%02x change to %02x:%02x:%02x:%02x:%02x:%02x\n",
		         wrole->mac_addr[0], wrole->mac_addr[1],
		         wrole->mac_addr[2], wrole->mac_addr[3],
		         wrole->mac_addr[4], wrole->mac_addr[5],
		         maddr[0], maddr[1], maddr[2],
		         maddr[3], maddr[4], maddr[5]);

		_os_mem_cpy(drv, wrole->mac_addr, maddr, MAC_ALEN);
		sta = rtw_phl_get_stainfo_self(phl_info, wrole);
		if (sta) {
			mode = PHL_UPD_ROLE_INFO_CHANGE;
			_os_mem_cpy(drv, sta->mac_addr, wrole->mac_addr, MAC_ALEN);
			pstate = phl_change_stainfo(phl_info, sta, mode);
			/*rtw_hal_change_sta_entry(phl_info->hal, sta);*/
		}
	}
		break;
	case WR_CHG_AP_PARAM:
	{
		struct rtw_ap_param *param = (struct rtw_ap_param *)chg_info;

		if (wrole->type != PHL_RTYPE_AP)
			break;

		if (param->cfg_id == CFG_BSS_CLR) {
			u32 bsscolor = param->value;

			wrole->proto_role_cap.bsscolor = (u8)bsscolor;
			rtw_hal_role_cfg_ex(phl_info->hal, wrole, PCFG_BSS_CLR, &bsscolor);
		}
		/* else if (param->cfg_id == CFG_HIQ_DTIM) */
		else {
			PHL_INFO("wrole->type(%d) WR_CHG_AP_PARAM todo.....\n", wrole->type);
		}

		pstate = RTW_PHL_STATUS_SUCCESS;
	}
		break;
	case WR_CHG_EDCA_PARAM:
	{
		struct rtw_edca_param *param = (struct rtw_edca_param*)chg_info;

		hstate = rtw_hal_set_edca(phl_info->hal, wrole, param->ac,
						param->param);
		if (hstate == RTW_HAL_STATUS_SUCCESS)
			pstate = RTW_PHL_STATUS_SUCCESS;
	}
		break;
	case WR_CHG_MU_EDCA_PARAM:
	{
		struct rtw_mu_edca_param *param = (struct rtw_mu_edca_param*)chg_info;
		hstate = rtw_hal_set_mu_edca(phl_info->hal, wrole->hw_band, param->ac,
					param->timer, (param->cw & 0x0f), (param->cw>>4),
					param->aifsn);
		if (hstate == RTW_HAL_STATUS_SUCCESS)
			pstate = RTW_PHL_STATUS_SUCCESS;
	}
		break;
	case WR_CHG_MU_EDCA_CFG:
	{
		u8 val = *(u8 *)chg_info;
		hstate = rtw_hal_set_mu_edca_ctrl(phl_info->hal,
					wrole->hw_band, wrole->hw_wmm, val);
		if (hstate == RTW_HAL_STATUS_SUCCESS)
			pstate = RTW_PHL_STATUS_SUCCESS;
	}
		break;
	case WR_CHG_BSS_COLOR:
	{
		u32 val = *(u8 *)chg_info;
		hstate = rtw_hal_role_cfg_ex(phl_info->hal, wrole, PCFG_BSS_CLR, &val);
		if (hstate == RTW_HAL_STATUS_SUCCESS)
			pstate = RTW_PHL_STATUS_SUCCESS;
	}
		break;
	case WR_CHG_MAX:
		PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_,
				"%s: Unsupported case:%d, please check it\n",
				__func__, chg_id);
		break;
	default :
		PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_,
				"%s: unknown chg_id(%d), please check it\n",
				__func__, chg_id);
		break;
	};
	return pstate;
}

#endif // if 0 NEO

enum rtw_phl_status
_phl_wifi_role_stop(struct phl_info_t *phl_info, struct rtw_wifi_role_t *wrole)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	pstatus = phl_wifi_role_free_stainfo(phl_info, wrole);
	if (pstatus != RTW_PHL_STATUS_SUCCESS)
		PHL_ERR("phl_wifi_role_free_stainfo failed\n");

	return pstatus;
}


void rtw_phl_wifi_role_free(void *phl, u8 role_idx)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct rtw_wifi_role_t *wrole = NULL;
	struct mr_ctl_t *mr_ctl = phlcom_to_mr_ctrl(phl_com);

	if (role_idx >= MAX_WIFI_ROLE_NUMBER) {
		PHL_ERR("%s invalid role index :%d", __func__, role_idx);
		return;
	}

	wrole = &phl_com->wifi_roles[role_idx];
	_phl_role_notify(phl_info, wrole, PHL_ROLE_STOP);
	if (_phl_wifi_role_stop(phl_info, wrole) != RTW_PHL_STATUS_SUCCESS)
		PHL_ERR("%s _phl_wifi_role_stop failed :%d", __func__, role_idx);

	_phl_free_hw_resource(phl, wrole);
	wrole->active = false;
	_os_spinlock(phl_to_drvpriv(phl_info), &mr_ctl->lock, _ps, NULL);
	mr_ctl->role_map &= ~BIT(role_idx);
	_os_spinunlock(phl_to_drvpriv(phl_info), &mr_ctl->lock, _ps, NULL);
	PHL_DUMP_MR_EX(phl_info);
}

enum rtw_phl_status
phl_register_mrc_module(struct phl_info_t *phl_info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	struct mr_ctl_t *mr_ctl = phlcom_to_mr_ctrl(phl_info->phl_com);
	struct phl_bk_module_ops bk_ops = mr_ctl->bk_ops;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);
	u8 i = 0;

	for(i = 0; i < disp_eng->phy_num; i++)
	{
		phl_status = phl_disp_eng_register_module(phl_info, i,
						       PHL_MDL_MRC,
						       &bk_ops);
		if (RTW_PHL_STATUS_SUCCESS != phl_status) {
			PHL_ERR("%s register MRC module in cmd disp failed :%d\n", __func__, i+1);
			goto error_register_bk;
		}
	}

	return phl_status;

error_register_bk:
	while(i > 0){
		phl_status = phl_disp_eng_deregister_module(phl_info, --i,
							 PHL_MDL_MRC);
		if (RTW_PHL_STATUS_SUCCESS != phl_status) {
			PHL_ERR("%s deregister MRC module in cmd disp failed :%d\n", __func__, i+1);
		}
	}
	return RTW_PHL_STATUS_FAILURE;
}

#if 0 //NEO

#ifdef RTW_WKARD_RADIO_IPS_FLOW
enum rtw_phl_status
phl_role_recover(struct phl_info_t *phl_info)
{
	u8 role_idx;
	struct rtw_wifi_role_t *wrole;
	struct rtw_phl_stainfo_t *sta;
	enum rtw_phl_status pstatus;
	enum rtw_hal_status hstatus;

	for (role_idx = 0; role_idx < MAX_WIFI_ROLE_NUMBER; role_idx++) {
		wrole = phl_get_wrole_by_ridx(phl_info, role_idx);
		if(wrole == NULL)
			continue;

		PHL_INFO("%s with role_idx %d\n", __func__, role_idx);

		sta = rtw_phl_get_stainfo_self(phl_info, wrole);
		if(sta == NULL)
			continue;

		hstatus = rtw_hal_role_cfg(phl_info->hal, wrole);
		if (hstatus != RTW_HAL_STATUS_SUCCESS) {
			PHL_ERR("%s hal role cfg failed\n", __func__);
			return RTW_PHL_STATUS_FAILURE;
		}

		pstatus = phl_alloc_stainfo_hw(phl_info, sta);
		if (pstatus != RTW_PHL_STATUS_SUCCESS) {
			PHL_ERR("%s failed\n", __func__);
			return RTW_PHL_STATUS_FAILURE;
		}
	}

	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status
phl_role_suspend(struct phl_info_t *phl_info)
{
	u8 role_idx;
	struct rtw_wifi_role_t *wrole;
	struct rtw_phl_stainfo_t *sta;
	enum rtw_phl_status pstatus;

	for (role_idx = 0; role_idx < MAX_WIFI_ROLE_NUMBER; role_idx++) {
		wrole = phl_get_wrole_by_ridx(phl_info, role_idx);
		if(wrole == NULL)
			continue;

		PHL_INFO("%s with role_idx %d\n", __func__, role_idx);

		sta = rtw_phl_get_stainfo_self(phl_info, wrole);
		if(sta == NULL)
			continue;

		pstatus = phl_free_stainfo_hw(phl_info, sta);
		if (pstatus != RTW_PHL_STATUS_SUCCESS) {
			PHL_ERR("%s failed\n", __func__);
			return RTW_PHL_STATUS_FAILURE;
		}
	}

	return RTW_PHL_STATUS_SUCCESS;
}
#endif

#ifdef RTW_WKARD_LPS_ROLE_CONFIG
void phl_role_recover_unused_role(struct phl_info_t *phl_info,
	struct rtw_wifi_role_t *cur_wrole)
{
	u8 role_idx;
	struct rtw_wifi_role_t * wrole;
	enum rtw_hal_status hstatus;

	if (cur_wrole == NULL) {
		PHL_ERR("%s cur role is NULL\n", __func__);
		return;
	}

	for (role_idx = 0; role_idx < MAX_WIFI_ROLE_NUMBER; role_idx++) {
		wrole = phl_get_wrole_by_ridx(phl_info, role_idx);
		if(wrole == NULL || !wrole->active)
			continue;

		if (wrole == cur_wrole)
			continue;

		hstatus = rtw_hal_role_cfg(phl_info->hal, wrole);
		if (hstatus != RTW_HAL_STATUS_SUCCESS) {
			PHL_ERR("%s hal role cfg failed\n", __func__);
		} else {
			PHL_INFO("%s recover role: %d\n", __func__, role_idx);
		}
	}
}

void phl_role_suspend_unused_role(struct phl_info_t *phl_info,
	struct rtw_wifi_role_t *cur_wrole)
{
	u8 role_idx;
	struct rtw_wifi_role_t *wrole;
	u32 func_en = false;

	if (cur_wrole == NULL) {
		PHL_ERR("%s cur role is NULL\n", __func__);
		return;
	}

	for (role_idx = 0; role_idx < MAX_WIFI_ROLE_NUMBER; role_idx++) {
		wrole = phl_get_wrole_by_ridx(phl_info, role_idx);
		if(wrole == NULL || !wrole->active)
			continue;

		if (wrole == cur_wrole)
			continue;

		PHL_INFO("%s suspend role: %d\n", __func__, role_idx);
		rtw_hal_role_cfg_ex(phl_info->hal, wrole, PCFG_FUNC_SW, &func_en);
	}
}
#endif // if 0 NEO

#endif

