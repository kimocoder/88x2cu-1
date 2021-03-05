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
#include "phl_btc_fsm.h"

struct btc_obj {
	struct fsm_main *fsm;
	struct fsm_obj *fsm_obj;
	struct phl_info_t *phl_info;

	struct rtw_phl_btc_ntfy *ntfy;
};

enum BTC_STATE_ST {
	BTC_ST_IDLE,
	BTC_ST_SERVICE
};

enum BTC_EV_ID {
	BTC_EV_START,
	BTC_EV_QUERY_COEX_INFO,
	BTC_EV_UPDATE_ROLE_INFO,
	BTC_EV_CTRL,
	BTC_EV_FWINFO,
	BTC_EV_RADIO_ST,
	BTC_EV_SCAN_START,
	BTC_EV_SCAN_START_OK,
	BTC_EV_SET_ANT_PATH,
	BTC_EV_BT_IQK_DONE,
	BTC_EV_WLSTA,
	BTC_EV_TIMER,
	BTC_EV_PACKET,
	BTC_EV_PERIOTIC,
	BTC_EV_MAX
};

static int btc_idle_st_hdl(void *obj, u16 event, void *param);
static int btc_service_st_hdl(void *obj, u16 event, void *param);

/* STATE table */
static struct fsm_state_ent btc_state_tbl[] = {
	ST_ENT(BTC_ST_IDLE, btc_idle_st_hdl),
	ST_ENT(BTC_ST_SERVICE, btc_service_st_hdl)
	//{BTC_ST_MAX, "BTC_ST_MAX", NULL} /* ST_MAX for fsm safety checking */
};

/* EVENT table */
static struct fsm_event_ent btc_event_tbl[] = {
	EV_ENT(BTC_EV_START),
	EV_ENT(BTC_EV_QUERY_COEX_INFO),
	EV_ENT(BTC_EV_UPDATE_ROLE_INFO),
	EV_ENT(BTC_EV_CTRL),
	EV_ENT(BTC_EV_FWINFO),
	EV_ENT(BTC_EV_RADIO_ST),
	EV_ENT(BTC_EV_SCAN_START),
	EV_ENT(BTC_EV_SCAN_START_OK),
	EV_ENT(BTC_EV_SET_ANT_PATH),
	EV_ENT(BTC_EV_BT_IQK_DONE),
	EV_ENT(BTC_EV_WLSTA),
	EV_DBG(BTC_EV_TIMER),
	EV_ENT(BTC_EV_PACKET),
	EV_DBG(BTC_EV_PERIOTIC),
	EV_ENT(BTC_EV_MAX) /* EV_MAX for fsm safety checking */
};


/*
 * btc idle handler
 */
static int btc_idle_st_hdl(void *obj, u16 event, void *param)
{
	struct btc_obj *pbtc = (struct btc_obj *)obj;
	struct phl_info_t *phl_info = pbtc->phl_info;
	void *d = phl_to_drvpriv(pbtc->phl_info);

	FSM_DBG(pbtc->fsm, "[BTC], %s, event : 0x%x\n", __func__, event);
	switch (event) {
	case FSM_EV_STATE_IN:
		break;

	case FSM_EV_TIMER_EXPIRE:
		break;

	case BTC_EV_START:
		phl_fsm_state_goto(pbtc->fsm_obj, BTC_ST_SERVICE);
		break;

	case FSM_EV_STATE_OUT:
		break;

	default:
		break;
	}
	return 0;
}

static int btc_service_st_hdl(void *obj, u16 event, void *param)
{
	struct btc_obj *pbtc = (struct btc_obj *)obj;
	struct phl_info_t *phl_info = pbtc->phl_info;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	void *d = phl_to_drvpriv(pbtc->phl_info);
	int rtn = FSM_FREE_PARAM;

	FSM_DBG(pbtc->fsm, "[BTC], %s, event : 0x%x\n", __func__, event);

	RTW_INFO("%s NEO TODO\n", __func__);

#if 0 // NEO
	switch (event) {
	case FSM_EV_STATE_IN:
		break;

	case BTC_EV_UPDATE_ROLE_INFO:
		if (param) {
			struct rtw_phl_btc_role_info_param *prinfo = NULL;
			struct rtw_wifi_role_t *wrole = NULL;
			struct rtw_phl_stainfo_t *sta = NULL;

			pbtc->ntfy = param;
			prinfo = &pbtc->ntfy->u.rinfo;

			if (prinfo->role_id < MAX_WIFI_ROLE_NUMBER) {
				wrole = &phl_com->wifi_roles[prinfo->role_id];
				sta = rtw_phl_get_stainfo_self(phl_info, wrole);
			}
#ifdef CONFIG_BTCOEX
			rtw_hal_btc_update_role_info_ntfy(phl_info->hal,
					prinfo->role_id, wrole, sta, prinfo->rstate);
#endif
			if (pbtc->ntfy->ntfy_cb)
				pbtc->ntfy->ntfy_cb(pbtc->ntfy->priv,
						pbtc->ntfy->notify,
						pbtc->ntfy);

			rtn = FSM_FREE_PARAM;
		}
		break;

	case BTC_EV_FWINFO:
#ifdef CONFIG_BTCOEX
		rtw_hal_btc_fwinfo_ntfy(phl_info->hal);
#endif
		break;

	case BTC_EV_PERIOTIC:
	{
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
					/* PHL_INFO("[BTC], sta[%d] = %p \n", ntfy_num, wrole_sta[ntfy_num]); */
					ntfy_num++;
				}
			}
		}

#ifdef CONFIG_BTCOEX
		rtw_hal_btc_wl_status_ntfy(phl_info->hal, phl_com, ntfy_num,
				wrole_sta, PHL_BTC_NTFY_RSN_PERIOTIC);
		rtw_hal_btc_timer(phl_info->hal);
#endif
	}
		break;

	case BTC_EV_PACKET:
		if (param) {
			struct rtw_phl_btc_pkt_param *pkt = NULL;
			struct rtw_wifi_role_t *wrole = NULL;
			pbtc->ntfy = param;
			pkt = &pbtc->ntfy->u.pkt;

			if (pkt->role_id < MAX_WIFI_ROLE_NUMBER) {
				wrole = &phl_com->wifi_roles[pkt->role_id];
			}
#ifdef CONFIG_BTCOEX
			rtw_hal_btc_specific_packet_ntfy(phl_info->hal,
							pkt->pkt_type);
#endif
			if (pbtc->ntfy->ntfy_cb)
				pbtc->ntfy->ntfy_cb(pbtc->ntfy->priv,
						pbtc->ntfy->notify,
						pbtc->ntfy);

			rtn = FSM_FREE_PARAM;
		}
		break;

	case FSM_EV_TIMER_EXPIRE:
		break;

	case FSM_EV_CANCEL:
		phl_fsm_state_goto(pbtc->fsm_obj, BTC_ST_IDLE);
		break;

	case FSM_EV_STATE_OUT:
		phl_fsm_cancel_alarm(pbtc->fsm_obj);
		break;

	default:
		break;
	}
#endif // if 0 NEO
	return rtn;
}

static void btc_dump_obj(void *obj, char *s, int *sz)
{
	/* nothing to do for now */
}

static void btc_dump_fsm(void *fsm, char *s, int *sz)
{
	/* nothing to do for now */
}

/* For EXTERNAL application to create a btc FSM */
/* @root: FSM root structure
 * @phl_info: private data structure to invoke hal/phl function
 *
 * return
 * fsm_main: FSM main structure (Do NOT expose)
 */
struct fsm_main *phl_btc_new_fsm(struct fsm_root *root,
				struct phl_info_t *phl_info)
{
	void *d = phl_to_drvpriv(phl_info);
	struct fsm_main *fsm = NULL;
	struct rtw_phl_fsm_tb tb;

	_os_mem_set(d, &tb, 0, sizeof(tb));
	tb.max_state = sizeof(btc_state_tbl)/sizeof(btc_state_tbl[0]);
	tb.max_event = sizeof(btc_event_tbl)/sizeof(btc_event_tbl[0]);
	tb.state_tbl = btc_state_tbl;
	tb.evt_tbl = btc_event_tbl;
	tb.dump_obj = btc_dump_obj;
	tb.dump_obj = btc_dump_fsm;
	tb.dbg_level = FSM_DBG_INFO;
	tb.evt_level = FSM_DBG_INFO;

	fsm = phl_fsm_init_fsm(root, "btc", phl_info, &tb);

	return fsm;
}

/* For EXTERNAL application to destory btc fsm */
/* @fsm: see fsm_main
 */
void phl_btc_destory_fsm(struct fsm_main *fsm)
{
	if (fsm == NULL)
		return;

	/* deinit fsm local variable if has */

	/* call FSM Framewro to deinit fsm */
	phl_fsm_deinit_fsm(fsm);
}

/* For EXTERNAL application to create btcoex object */
/* @fsm: FSM main structure which created by phl_btc_new_fsm()
 * @phl_info: private data structure to invoke hal/phl function
 *
 * return
 * btc_obj: structure of command object (Do NOT expose)
 */
struct btc_obj *phl_btc_new_obj(struct fsm_main *fsm,
				struct phl_info_t *phl_info)
{
	struct fsm_obj *obj;
	struct btc_obj *pbtc;

	pbtc = phl_fsm_new_obj(fsm, (void **)&obj, sizeof(*pbtc));

	if (pbtc == NULL || obj == NULL) {
		/* TODO free fsm; currently will be freed in deinit process */
		FSM_ERR(fsm, "btc: malloc obj fail\n");
		return NULL;
	}
	pbtc->fsm = fsm;
	pbtc->fsm_obj = obj;
	pbtc->phl_info = phl_info;

	return pbtc;
}

/* For EXTERNAL application to destory btc object */
/* @pbtc: local created command object
 *
 */
void phl_btc_destory_obj(struct btc_obj *pbtc)
{
	if (pbtc == NULL)
		return;

	/* inform FSM framewory to recycle fsm_obj */
	phl_fsm_destory_obj(pbtc->fsm_obj);
}

/* For EXTERNAL application to start btcoex service (expose) */
/* @pbtc: btc object
 */
enum rtw_phl_status phl_btc_start(struct btc_obj *pbtc)
{
	void *d = phl_to_drvpriv(pbtc->phl_info);
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_SUCCESS;
	struct fsm_msg *msg;

	/* Start FSM */
	phl_status = phl_fsm_start_fsm(pbtc->fsm);
	if (phl_status != RTW_PHL_STATUS_SUCCESS)
		return phl_status;

	if (pbtc->fsm_obj == NULL)
		return RTW_PHL_STATUS_FAILURE;

	/* NEW message to start btc object */
	msg = phl_fsm_new_msg(pbtc->fsm_obj, BTC_EV_START);
	if (msg == NULL) {
		FSM_ERR(pbtc->fsm, "btc: alloc msg fail\n");
		return RTW_PHL_STATUS_RESOURCE;
	}
	if (phl_fsm_sent_msg(pbtc->fsm_obj, msg) != RTW_PHL_STATUS_SUCCESS) {
		_os_kmem_free(d, msg, sizeof(*msg));
		return RTW_PHL_STATUS_FAILURE;
	}

	return phl_status;
}

/* For EXTERNAL application to stop btc service (expose) */
/* @pbtc: btc to be cancelled
 */
enum rtw_phl_status phl_btc_cancel(struct btc_obj *pbtc)
{
#ifdef PHL_INCLUDE_FSM
	return phl_fsm_cancel_obj(pbtc->fsm_obj);
#else
	return RTW_PHL_STATUS_SUCCESS;
#endif /* PHL_INCLUDE_FSM */
}

#ifdef CONFIG_FSM

/* For EXTERNAL application to notify btc (expose) */
/* @phl: refer to phl_infi_t
 * @notify: notification event
 * @ntfy: notify information (optional)
 * return value:
 *	0: no wait
 *	1: have to wait call back info->ntfy_cb()
 *	-1: Failure
 */
int rtw_phl_btc_notify(void *phl, enum RTW_PHL_BTC_NOTIFY notify,
				struct rtw_phl_btc_ntfy *ntfy)
{
#ifdef PHL_INCLUDE_FSM
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct btc_obj *pbtc = phl_info->btc_obj;
	int wait = 0, sz = 0;
	u16 event;

	if (ntfy)
		sz = sizeof(*ntfy);

	switch (notify) {
	case PHL_BTC_NTFY_ROLE_INFO:
		event = BTC_EV_UPDATE_ROLE_INFO;
		break;
	case PHL_BTC_NTFY_FWINFO:
		event = BTC_EV_FWINFO;
		break;
	case PHL_BTC_NTFY_SPECIFIC_PKT:
		event = BTC_EV_PACKET;
		break;
	case PHL_BTC_NTFY_MAX:
		PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "rtw_phl_btc_notify(): Unsupported case:%d, please check it\n",
				notify);
		return wait;
	case PHL_BTC_NTFY_PERIOTIC:
		event = BTC_EV_PERIOTIC;
		break;
	default:
		PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "rtw_phl_btc_notify(): Unrecognize case:%d, please check it\n",
				notify);
		return wait;
	}
	phl_fsm_gen_msg(phl, pbtc->fsm_obj, ntfy, sz, event);
	return wait;
#else
	return 0;
#endif /* PHL_INCLUDE_FSM */
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

#endif // CONFIG_FSM

void rtw_phl_btc_specific_packet_notify(void *phl, u8 role_id, u8 packet_type)
{
	struct rtw_phl_btc_ntfy ntfy = {0};
	struct rtw_phl_btc_pkt_param *pkt = &ntfy.u.pkt;

	pkt->role_id = role_id;
	pkt->pkt_type = packet_type;

	ntfy.notify = PHL_BTC_NTFY_SPECIFIC_PKT;
	ntfy.ops = NULL;
	ntfy.priv = NULL;
	ntfy.ntfy_cb = NULL;

	rtw_phl_btc_notify(phl, ntfy.notify, &ntfy);
}

#ifdef CONFIG_FSM

void rtw_phl_btc_hub_msg_hdl(void *phl, struct phl_msg *msg)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct btc_obj *pbtc = phl_info->btc_obj;
	u8 mdl_id = MSG_MDL_ID_FIELD(msg->msg_id);
	u16 evt_id = MSG_EVT_ID_FIELD(msg->msg_id);

	switch (mdl_id) {
	case PHL_MDL_BTC:
		if (evt_id == BTC_HMSG_TMR_EN) {
			rtw_phl_btc_notify(phl, PHL_BTC_NTFY_PERIOTIC, NULL);
		} else if (evt_id == BTC_HMSG_FW_EV) {
			rtw_phl_btc_notify(phl, PHL_BTC_NTFY_FWINFO, NULL);
		}
		break;
	default:
		break;
	}
}

#endif // CONFIG_FSM
