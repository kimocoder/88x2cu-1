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
#define _PHL_CMD_BTC_C_
#include "phl_headers.h"

#ifdef CONFIG_BTCOEX
#ifdef CONFIG_PHL_CMD_BTC
static void _fail_hdlr(void *phl, struct phl_msg *msg)
{
}

static void _hdl_tmr(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_wifi_role_t *wrole = NULL;
	struct rtw_phl_stainfo_t *sta = NULL;
	struct rtw_phl_stainfo_t *wrole_sta[MAX_WIFI_ROLE_NUMBER] = {0};
	u8 ntfy_num = 0;
	u8 i;

	for (i = 0; i < MAX_WIFI_ROLE_NUMBER; i++) {

		wrole = phl_get_wrole_by_ridx(phl_info, i);
		if(wrole->mstate == MLME_LINKED) {
			sta = rtw_phl_get_stainfo_self(phl_info, wrole);
			if(sta != NULL) {
				wrole_sta[ntfy_num] = sta;
				ntfy_num++;
			}
		}
	}
	rtw_hal_btc_wl_status_ntfy(phl_info->hal,
		phl_info->phl_com, ntfy_num,
		wrole_sta, PHL_BTC_NTFY_RSN_PERIOTIC);
	rtw_hal_btc_timer(phl_info->hal);
}

static void _hdl_role_notify(void *phl, struct phl_msg *msg)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_role_cmd *rcmd = NULL;
	struct rtw_wifi_role_t *wrole = NULL;
	struct rtw_phl_stainfo_t *sta = NULL;

	if (msg->inbuf && (msg->inlen == sizeof(struct rtw_role_cmd))) {
		rcmd  = (struct rtw_role_cmd *)msg->inbuf;
		wrole = rcmd->wrole;
		sta = rtw_phl_get_stainfo_self(phl_info, wrole);

		rtw_hal_btc_update_role_info_ntfy(phl_info->hal,
			rcmd->wrole->id, wrole, sta, rcmd->rstate);
	} else {
		PHL_ERR("%s: invalid msg, buf = %p, len = %d\n",
			__func__, msg->inbuf, msg->inlen);
	}
}

static void _hdl_pkt_notify(void *phl, struct phl_msg *msg)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_pkt_ntfy *pkt = NULL;
	struct rtw_wifi_role_t *wrole = NULL;

	if (msg->inbuf && (msg->inlen == sizeof(struct rtw_pkt_ntfy))) {
		pkt = (struct rtw_pkt_ntfy *)msg->inbuf;
		wrole = pkt->wrole; /* not used currently */

		rtw_hal_btc_specific_packet_ntfy(phl_info->hal,
					(u8)pkt->type);
	} else {
		PHL_ERR("%s: invalid msg, buf = %p, len = %d\n",
			__func__, msg->inbuf, msg->inlen);
	}
}

static enum phl_mdl_ret_code _btc_cmd_init(void *phl, void *dispr,
						  void **priv)
{
	PHL_INFO("[BTCCMD], %s(): \n", __func__);

	*priv = phl;
	return MDL_RET_SUCCESS;
}

static void _btc_cmd_deinit(void *dispr, void *priv)
{
	PHL_INFO("[BTCCMD], %s(): \n", __func__);
}

static enum phl_mdl_ret_code _btc_cmd_start(void *dispr, void *priv)
{
	enum phl_mdl_ret_code ret = MDL_RET_SUCCESS;

	PHL_INFO("[BTCCMD], %s(): \n", __func__);

	return ret;
}

static enum phl_mdl_ret_code _btc_cmd_stop(void *dispr, void *priv)
{
	enum phl_mdl_ret_code ret = MDL_RET_SUCCESS;

	PHL_INFO("[BTCCMD], %s(): \n", __func__);

	return ret;
}

static enum phl_mdl_ret_code
_pre_msg_hdlr(struct phl_info_t *phl_info, void *dispr,
				    struct phl_msg *msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_IGNORE;
	enum phl_msg_evt_id evt_id = MSG_EVT_ID_FIELD(msg->msg_id);

	/*PHL_INFO("[BTCCMD], msg->band_idx = %d,  msg->msg_id = 0x%x\n",
		msg->band_idx, msg->msg_id);*/

	switch(evt_id) {
		case MSG_EVT_SCAN_START:
			PHL_INFO("[BTCCMD], MSG_EVT_SCAN_START \n");
			ret = MDL_RET_SUCCESS;
			break;

		case MSG_EVT_CONNECT_START:
			PHL_INFO("[BTCCMD], MSG_EVT_CONNECT_START \n");
			ret = MDL_RET_SUCCESS;
			break;

		case MSG_EVT_BTC_REQ_BT_SLOT:
			PHL_INFO("[BTCCMD], MSG_EVT_BTC_REQ_BT_SLOT \n");
			ret = MDL_RET_SUCCESS;
			break;

		case MSG_EVT_PACKET_NTFY:
			PHL_INFO("[BTCCMD], MSG_EVT_PACKET_NTFY \n");
			_hdl_pkt_notify(phl_info, msg);
			ret = MDL_RET_SUCCESS;
			break;

		default:
			break;
	}

	return ret;
}

static enum phl_mdl_ret_code
_post_msg_hdlr(struct phl_info_t *phl_info, void *dispr,
				     struct phl_msg *msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_IGNORE;
	enum phl_msg_evt_id evt_id = MSG_EVT_ID_FIELD(msg->msg_id);

	switch(evt_id) {
		case MSG_EVT_SCAN_END:
			PHL_DBG("[BTCCMD], MSG_EVT_SCAN_END \n");
			ret = MDL_RET_SUCCESS;
			break;

		case MSG_EVT_CONNECT_END:
			PHL_DBG("[BTCCMD], MSG_EVT_CONNECT_END \n");
			ret = MDL_RET_SUCCESS;
			break;

		case MSG_EVT_ROLE_NTFY:
			PHL_DBG("[BTCCMD], MSG_EVT_ROLE_NTFY \n");
			_hdl_role_notify(phl_info, msg);
			ret = MDL_RET_SUCCESS;
			break;

		case MSG_EVT_BTC_TMR:
			PHL_DBG("[BTCCMD], MSG_EVT_BTC_TMR \n");
			_hdl_tmr(phl_info);
			ret = MDL_RET_SUCCESS;
			break;

		case MSG_EVT_BTC_FWEVNT:
			PHL_DBG("[BTCCMD], MSG_EVT_BTC_FWEVNT \n");
			rtw_hal_btc_fwinfo_ntfy(phl_info->hal);
			ret = MDL_RET_SUCCESS;
			break;

		default:
			break;
	}

	return ret;
}

static enum phl_mdl_ret_code _btc_msg_hdlr(void *dispr, void *priv,
						      struct phl_msg *msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_IGNORE;
	struct phl_info_t *phl_info = (struct phl_info_t *)priv;

	FUNCIN();

	if (IS_MSG_FAIL(msg->msg_id)) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_,
			  "%s: cmd dispatcher notify cmd failure: 0x%x.\n",
			   __FUNCTION__, msg->msg_id);
		_fail_hdlr(phl_info, msg);
		FUNCOUT();
		return MDL_RET_FAIL;
	}

	if (IS_PRIVATE_MSG(msg->msg_id)) {
		FUNCOUT();
		return ret;
	}

	if (IS_MSG_IN_PRE_PHASE(msg->msg_id))
		ret = _pre_msg_hdlr(phl_info, dispr, msg);
	else
		ret = _post_msg_hdlr(phl_info, dispr, msg);

	FUNCOUT();
	return ret;
}

static enum phl_mdl_ret_code
_btc_set_info(void *dispr, void *priv,
			 struct phl_module_op_info *info)
{
	PHL_INFO("[BTCCMD], %s(): \n", __func__);

	return MDL_RET_SUCCESS;
}

static enum phl_mdl_ret_code
_btc_query_info(void *dispr, void *priv,
			   struct phl_module_op_info *info)
{
	PHL_INFO("[BTCCMD], %s(): \n", __func__);

	return MDL_RET_SUCCESS;
}

enum rtw_phl_status phl_register_btc_module(struct phl_info_t *phl_info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);
	struct phl_bk_module_ops bk_ops = {0};
	u8 i = 0;

	PHL_INFO("[BTCCMD], %s(): \n", __func__);

	bk_ops.init = _btc_cmd_init;
	bk_ops.deinit = _btc_cmd_deinit;
	bk_ops.start = _btc_cmd_start;
	bk_ops.stop = _btc_cmd_stop;
	bk_ops.msg_hdlr = _btc_msg_hdlr;
	bk_ops.set_info = _btc_set_info;
	bk_ops.query_info = _btc_query_info;

	phl_status = phl_disp_eng_register_module(phl_info, HW_BAND_0,
						PHL_MDL_BTC, &bk_ops);
	if (RTW_PHL_STATUS_SUCCESS != phl_status) {
		PHL_ERR(" Failed to register BTC module in cmd disp (%d) \n", i+1);
	}

	return phl_status;
}

void rtw_phl_btc_send_cmd(struct rtw_phl_com_t *phl_com,
				u8 *buf, u32 len, u16 ev_id)
{
	struct phl_info_t *phl_info = phl_com->phl_priv;
	u8 band_idx = HW_BAND_0;
	struct phl_msg msg = {0};
	struct phl_msg_attribute attr = {0};

	SET_MSG_MDL_ID_FIELD(msg.msg_id, PHL_MDL_BTC);
	msg.band_idx = band_idx;
	switch (ev_id) {
	case BTC_HMSG_TMR_EN:
		SET_MSG_EVT_ID_FIELD(msg.msg_id,
			MSG_EVT_BTC_TMR);
		break;
	case BTC_HMSG_SET_BT_REQ_SLOT:
		SET_MSG_EVT_ID_FIELD(msg.msg_id,
			MSG_EVT_BTC_REQ_BT_SLOT);
		break;
	case BTC_HMSG_FW_EV:
		SET_MSG_EVT_ID_FIELD(msg.msg_id,
			MSG_EVT_BTC_FWEVNT);
		break;
	default:
		PHL_ERR("%s: Unknown msg !\n", __func__);
		return;
	}

	if (phl_disp_eng_send_msg(phl_info, &msg, &attr, NULL) !=
				RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s: [BTC] dispr_send_msg failed !\n", __func__);
	}
}

static void
_phl_packet_ntfy_done(void* priv, struct phl_msg* msg)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)priv;

	if(msg->inbuf && msg->inlen){
		_os_mem_free(phl_to_drvpriv(phl_info),
			msg->inbuf, msg->inlen);
	}
}

void rtw_phl_btc_specific_packet_notify(void *phl, u8 role_id, u8 packet_type)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_msg msg = {0};
	struct phl_msg_attribute attr = {0};
	struct rtw_pkt_ntfy *pkt = NULL;

	pkt = (struct rtw_pkt_ntfy *)_os_mem_alloc(
		phl_to_drvpriv(phl_info), sizeof(struct rtw_pkt_ntfy));
	if (pkt == NULL) {
		PHL_ERR("%s: alloc packet cmd fail.\n", __func__);
		return;
	}

	pkt->type = packet_type;

	msg.inbuf = (u8 *)pkt;
	msg.inlen = sizeof(*pkt);

	SET_MSG_MDL_ID_FIELD(msg.msg_id, PHL_MDL_BTC);
	SET_MSG_EVT_ID_FIELD(msg.msg_id, MSG_EVT_PACKET_NTFY);
	msg.band_idx = HW_BAND_0;
	attr.completion.completion = _phl_packet_ntfy_done;
	attr.completion.priv = phl_info;

	if (phl_disp_eng_send_msg(phl_info, &msg, &attr, NULL) !=
				RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s: dispr_send_msg failed !\n", __func__);
		goto cmd_fail;
	}

	return;

cmd_fail:
	_os_mem_free(phl_to_drvpriv(phl_info), pkt,
			sizeof(struct rtw_pkt_ntfy));
}
#endif /*CONFIG_PHL_CMD_BTC*/

#ifndef CONFIG_FSM
int rtw_phl_btc_notify(void *phl, enum RTW_PHL_BTC_NOTIFY notify,
				struct rtw_phl_btc_ntfy *ntfy)
{
	PHL_ERR("CMD_BTC not support :%s\n", __func__);
	return 0;
}
void rtw_phl_btc_role_notify(void *phl, u8 role_id, enum role_state rstate)
{
	struct rtw_phl_btc_ntfy ntfy = {0};
	struct rtw_phl_btc_role_info_param *prinfo = &ntfy.u.rinfo;

	prinfo->role_id = role_id;
	prinfo->rstate = rstate;

	ntfy.notify = PHL_BTC_NTFY_ROLE_INFO;
	ntfy.ops = NULL;
	ntfy.priv = NULL;
	ntfy.ntfy_cb = NULL;

	rtw_phl_btc_notify(phl, ntfy.notify, &ntfy);
}

void rtw_phl_btc_hub_msg_hdl(void *phl, struct phl_msg *msg)
{
}
#endif

#endif /*CONFIG_BTCOEX*/

