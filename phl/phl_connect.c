/******************************************************************************
 *
 * Copyright(c) 2020 Realtek Corporation.
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

#ifdef CONFIG_STA_CMD_DISPR
enum rtw_phl_status rtw_phl_connect_prepare(void *phl,
					    struct rtw_wifi_role_t *wrole,
					    u8 *addr)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);
	struct phl_msg msg = {0};
	struct phl_msg_attribute attr = {0};

	SET_MSG_MDL_ID_FIELD(msg.msg_id, PHL_FG_MDL_CONNECT);
	SET_MSG_EVT_ID_FIELD(msg.msg_id, MSG_EVT_CONNECT_START);
	msg.band_idx = wrole->hw_band;

	phl_status = phl_disp_eng_send_msg(phl_info, &msg, &attr, NULL);
	if(phl_status != RTW_PHL_STATUS_SUCCESS){
		PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_, "%s: Dispr send msg fail!\n",
			  __FUNCTION__);
		goto exit;
	}

exit:
	return phl_status;
}

enum rtw_phl_status
rtw_phl_connect_linked(void *phl,
                       struct rtw_wifi_role_t *wrole,
                       struct rtw_phl_stainfo_t *sta,
                       u8 *sta_addr)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_msg msg = {0};
	struct phl_msg_attribute attr = {0};

	SET_MSG_MDL_ID_FIELD(msg.msg_id, PHL_FG_MDL_CONNECT);
	SET_MSG_EVT_ID_FIELD(msg.msg_id, MSG_EVT_CONNECT_LINKED);
	msg.band_idx = wrole->hw_band;
	msg.rsvd[0] = sta;
	msg.rsvd[1] = sta_addr;

	phl_status = phl_disp_eng_send_msg(phl_info, &msg, &attr, NULL);
	if(phl_status != RTW_PHL_STATUS_SUCCESS){
		PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_, "%s: Dispr send msg fail!\n",
			  __FUNCTION__);
		goto exit;
	}

exit:
	return phl_status;
}

enum rtw_phl_status rtw_phl_connected(void *phl,
				      struct rtw_wifi_role_t *wrole,
				      struct rtw_phl_stainfo_t *sta)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_msg msg = {0};
	struct phl_msg_attribute attr = {0};

	SET_MSG_MDL_ID_FIELD(msg.msg_id, PHL_FG_MDL_CONNECT);
	SET_MSG_EVT_ID_FIELD(msg.msg_id, MSG_EVT_CONNECT_END);
	msg.band_idx = wrole->hw_band;

	phl_status = phl_disp_eng_send_msg(phl_info, &msg, &attr, NULL);
	if(phl_status != RTW_PHL_STATUS_SUCCESS){
		PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_, "%s: Dispr send msg fail!\n",
			  __FUNCTION__);
		goto exit;
	}

exit:
	return phl_status;
}

enum rtw_phl_status rtw_phl_disconnect(void *phl,
				       struct rtw_wifi_role_t *wrole,
				       bool is_disconnect)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);
	struct phl_msg msg = {0};
	struct phl_msg_attribute attr = {0};

	if(is_disconnect) {
		SET_MSG_MDL_ID_FIELD(msg.msg_id, PHL_FG_MDL_DISCONNECT);
		SET_MSG_EVT_ID_FIELD(msg.msg_id, MSG_EVT_DISCONNECT_PREPARE);
	} else {
		SET_MSG_MDL_ID_FIELD(msg.msg_id, PHL_FG_MDL_CONNECT);
		SET_MSG_EVT_ID_FIELD(msg.msg_id, MSG_EVT_DISCONNECT);
	}

	msg.band_idx = wrole->hw_band;
	msg.rsvd[0] = (u8*)wrole;

	phl_status = phl_disp_eng_send_msg(phl_info, &msg, &attr, NULL);
	if(phl_status != RTW_PHL_STATUS_SUCCESS){
		PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_, "%s: Dispr send msg fail!\n",
			  __FUNCTION__);
		goto exit;
	}

exit:
	return phl_status;
}
#else /* CONFIG_STA_CMD_DISPR */
enum rtw_phl_status rtw_phl_connect_prepare(void *phl,
					    struct rtw_wifi_role_t *wrole,
					    u8 *addr)
{
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	wrole->mstate = MLME_LINKING;
	psts = phl_role_notify(phl_info, wrole);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s role notify failed\n", __func__);
		goto _exit;
	}
	psts = phl_mr_info_upt(phl_info, wrole);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s mr info upt failed\n", __func__);
		goto _exit;
	}

	psts = rtw_phl_mr_rx_filter(phl, wrole);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s set mr_rx_filter failed\n", __func__);
		goto _exit;
	}

	PHL_DUMP_MR_EX(phl_info);
_exit:
	return psts;
}

enum rtw_phl_status rtw_phl_connected(void *phl,
				      struct rtw_wifi_role_t *wrole,
				      struct rtw_phl_stainfo_t *sta)
{
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	if (wrole->type == PHL_RTYPE_STATION || wrole->type == PHL_RTYPE_P2P_GC) {
		psts = phl_role_notify(phl_info, wrole);
		if (psts != RTW_PHL_STATUS_SUCCESS) {
			PHL_ERR("%s role notify failed\n", __func__);
			goto _exit;
		}
	}

	psts = phl_mr_info_upt(phl_info, wrole);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s mr info upt failed\n", __func__);
		goto _exit;
	}

	psts = rtw_phl_mr_rx_filter(phl, wrole);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s set mr_rx_filter failed\n", __func__);
		goto _exit;
	}

	psts = phl_mr_tsf_sync(phl, wrole, PHL_ROLE_MSTS_STA_CONN_END);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s set mr_tsf_sync failed\n", __func__);
		goto _exit;
	}
	#if 0
	psts = phl_mr_state_upt(phl_info, wrole);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s phl_mr_state_upt failed\n", __func__);
		goto _exit;
	}
	#endif

	PHL_DUMP_MR_EX(phl_info);
_exit:
	return psts;
}

enum rtw_phl_status rtw_phl_disconnect(void *phl,
				       struct rtw_wifi_role_t *wrole)
{
	enum rtw_phl_status psts = RTW_PHL_STATUS_SUCCESS;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	if (wrole->type == PHL_RTYPE_STATION || wrole->type == PHL_RTYPE_P2P_GC) {
		psts = phl_role_notify(phl_info, wrole);
		if (psts != RTW_PHL_STATUS_SUCCESS) {
			PHL_ERR("%s role notify failed\n", __func__);
			goto _exit;
		}
	}

	psts = phl_mr_info_upt(phl_info, wrole);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s mr info upt failed\n", __func__);
		goto _exit;
	}

	psts = rtw_phl_mr_rx_filter(phl, wrole);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s set mr_rx_filter failed\n", __func__);
		goto _exit;
	}

	psts = phl_mr_tsf_sync(phl, wrole, PHL_ROLE_MSTS_STA_DIS_CONN);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s set mr_tsf_sync failed\n", __func__);
		goto _exit;
	}
	psts = phl_mr_state_upt(phl_info, wrole);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s phl_mr_state_upt failed\n", __func__);
		goto _exit;
	}
	rtw_hal_disconnect_notify(phl_info->hal, &wrole->chandef);
	PHL_DUMP_MR_EX(phl_info);
_exit:
	return psts;
}
#endif

#ifdef CONFIG_AP_CMD_DISPR
enum rtw_phl_status rtw_phl_ap_started(void *phl, struct rtw_wifi_role_t *wrole)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);
	struct phl_msg msg = {0};
	struct phl_msg_attribute attr = {0};

	SET_MSG_MDL_ID_FIELD(msg.msg_id, PHL_FG_MDL_AP_START);
	SET_MSG_EVT_ID_FIELD(msg.msg_id, MSG_EVT_AP_START);
	msg.band_idx = wrole->hw_band;

	phl_status = phl_disp_eng_send_msg(phl_info, &msg, &attr, NULL);
	if(phl_status != RTW_PHL_STATUS_SUCCESS){
		PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_, "%s: Dispr send msg fail!\n",
			  __FUNCTION__);
		goto exit;
	}

exit:
	return phl_status;
}

enum rtw_phl_status rtw_phl_ap_stop(void *phl, struct rtw_wifi_role_t *wrole)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);
	struct phl_msg msg = {0};
	struct phl_msg_attribute attr = {0};

	SET_MSG_MDL_ID_FIELD(msg.msg_id, PHL_FG_MDL_AP_STOP);
	SET_MSG_EVT_ID_FIELD(msg.msg_id, MSG_EVT_AP_STOP);
	msg.band_idx = wrole->hw_band;
	msg.rsvd[0] = (u8*)wrole;

	phl_status = phl_disp_eng_send_msg(phl_info, &msg, &attr, NULL);
	if(phl_status != RTW_PHL_STATUS_SUCCESS){
		PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_, "%s: Dispr send msg fail!\n",
			  __FUNCTION__);
		goto exit;
	}

exit:
	return phl_status;
}
#else  /* CONFIG_AP_CMD_DISPR */
enum rtw_phl_status rtw_phl_ap_started(void *phl, struct rtw_wifi_role_t *wrole)
{
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	psts = phl_role_notify(phl_info, wrole);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s role notify failed\n", __func__);
		goto _exit;
	}
	psts = phl_mr_info_upt(phl_info, wrole);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s mr info upt failed\n", __func__);
		goto _exit;
	}
	psts = rtw_phl_mr_rx_filter(phl, wrole);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s set mr_rx_filter failed\n", __func__);
		goto _exit;
	}

	psts = phl_mr_tsf_sync(phl, wrole, PHL_ROLE_MSTS_AP_START);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s set mr_tsf_sync failed\n", __func__);
		goto _exit;
	}
	psts = phl_mr_state_upt(phl_info, wrole);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s phl_mr_state_upt failed\n", __func__);
		goto _exit;
	}

	PHL_DUMP_MR_EX(phl_info);

_exit:
	return psts;
}

enum rtw_phl_status rtw_phl_ap_stop(void *phl, struct rtw_wifi_role_t *wrole)
{
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	wrole->mstate = MLME_NO_LINK;
	psts = phl_role_notify(phl_info, wrole);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s role notify failed\n", __func__);
		goto _exit;
	}

	psts = phl_mr_info_upt(phl_info, wrole);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s mr info upt failed\n", __func__);
		goto _exit;
	}
	psts = rtw_phl_mr_rx_filter(phl, wrole);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s set mr_rx_filter failed\n", __func__);
		goto _exit;
	}

	psts = phl_mr_tsf_sync(phl, wrole, PHL_ROLE_MSTS_AP_STOP);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s set mr_tsf_sync failed\n", __func__);
		goto _exit;
	}
	psts = phl_mr_state_upt(phl_info, wrole);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s phl_mr_state_upt failed\n", __func__);
		goto _exit;
	}

#ifdef RTW_PHL_BCN
	psts = rtw_phl_free_bcn_entry(phl_info, wrole);
	if (psts != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s phl_free_bcn_entry failed\n", __func__);
		goto _exit;
	}
#endif
	rtw_hal_disconnect_notify(phl_info->hal, &wrole->chandef);
	PHL_DUMP_MR_EX(phl_info);
_exit:
	return psts;
}
#endif

#ifdef CONFIG_IBSS_CMD_DISPR
enum rtw_phl_status rtw_phl_ibss_started(void *phl, struct rtw_wifi_role_t *wrole)
{
	PHL_ERR("%s failed, cmd dispatcher not support\n", __func__);
	return RTW_PHL_STATUS_FAILURE;
}
#else  /* CONFIG_IBSS_CMD_DISPR */
enum rtw_phl_status rtw_phl_ibss_started(void *phl, struct rtw_wifi_role_t *wrole)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

#ifdef RTW_WKARD_IBSS_SNIFFER_MODE
	rtw_hal_set_rxfltr_by_mode(phl_info->hal, wrole->hw_band,
		RX_FLTR_MODE_SNIFFER);
#endif

	return RTW_PHL_STATUS_SUCCESS;
}

#endif

