/******************************************************************************
 *
 * Copyright(c) 2019 - 2021 Realtek Corporation.
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
#include <drv_types.h>

#if defined(CONFIG_CMD_GENERAL) || defined(CONFIG_STA_CMD_DISPR) || defined(CONFIG_CMD_TSF_SYNC)
#ifdef CONFIG_CMD_GENERAL/*for warkaround*/
static void
phl_run_core_cmd(void *drv_priv, u8 *cmd, u32 cmd_len, enum rtw_phl_status status)
{
	struct dvobj_priv *dvobj = (struct dvobj_priv *)drv_priv;
	struct cmd_obj *pcmd = (struct cmd_obj *)cmd;
	_adapter *padapter = pcmd->padapter;
	struct cmd_priv *pcmdpriv = &dvobj->cmdpriv;

	if (status == RTW_PHL_STATUS_CANNOT_IO ||
		status == RTW_PHL_STATUS_CMD_ERROR ||
		RTW_CANNOT_RUN(dvobj)) {
		RTW_INFO(FUNC_ADPT_FMT "%s FALSE -bDriverStopped(%s) bSurpriseRemoved(%s)\n"
			, FUNC_ADPT_ARG(padapter)
			, rtw_cmd_name(pcmd)
			, dev_is_drv_stopped(dvobj) ? "True" : "False"
			, dev_is_surprise_removed(dvobj) ? "True" : "False");

		if (pcmd->cmdcode == CMD_SET_DRV_EXTRA) {
			struct drvextra_cmd_parm *extra_parm =
				(struct drvextra_cmd_parm *)pcmd->parmbuf;

			if (extra_parm->pbuf && (extra_parm->size > 0))
				rtw_mfree(extra_parm->pbuf, extra_parm->size);
		}

		_rtw_mutex_lock(&pcmdpriv->sctx_mutex);
		if (pcmd->sctx) {
			if (0)
				RTW_PRINT(FUNC_ADPT_FMT" pcmd->sctx\n", FUNC_ADPT_ARG(pcmd->padapter));
			rtw_sctx_done_err(&pcmd->sctx, RTW_SCTX_DONE_CMD_DROP);
		}
		_rtw_mutex_unlock(&pcmdpriv->sctx_mutex);
		rtw_free_cmd_obj(pcmd);
		return;
	}

	if (status == RTW_PHL_STATUS_CMD_TIMEOUT)
		RTW_ERR("%s %s cmd timeout\n", __func__, rtw_cmd_name(pcmd));
	else if (status == RTW_PHL_STATUS_FAILURE) /*PHL fail due to 1. CMD_DROP:cmd abort or cancel 2.CMD_FAIL*/
		RTW_ERR("%s %s cmd failure\n", __func__, rtw_cmd_name(pcmd));

	rtw_run_cmd(padapter, pcmd, false);
}
#endif /* CONFIG_CMD_GENERAL */

#ifdef CONFIG_STA_CMD_DISPR
static u32 _evt_joinbss_hdl(struct _ADAPTER *a, struct wlan_network *network)
{
	struct dvobj_priv *d;
	u8 *mac;
	struct sta_info *sta;
	enum rtw_phl_status status;
	u32 res = _FAIL;


	d = adapter_to_dvobj(a);
	mac = network->network.MacAddress;
	sta = rtw_get_stainfo(&a->stapriv, mac);
	if (!sta) {
		RTW_ERR(FUNC_ADPT_FMT ": sta(" MAC_FMT ") not found!\n",
			FUNC_ADPT_ARG(a), MAC_ARG(mac));
		goto disconnect;
	}

	if (network->join_res < 0)
		goto disconnect;

	// NEO
	RTW_ERR("%s NEO TODO rtw_phl_connect_linked\n", __func__);
#if 0 // NEO
	status = rtw_phl_connect_linked(d->phl, a->phl_role, sta->phl_sta, mac);
	if (status == RTW_PHL_STATUS_SUCCESS)
		return _SUCCESS;
#endif // if 0 NEO

	RTW_ERR(FUNC_ADPT_FMT ": rtw_phl_connect_linked FAIL(%u)!\n",
		FUNC_ADPT_ARG(a), status);

disconnect:
	RTW_INFO(FUNC_ADPT_FMT ": something may go wrong, run disconnect! "
		 "join_res=%d for " MAC_FMT "\n",
		 FUNC_ADPT_ARG(a), network->join_res, MAC_ARG(mac));

	status = rtw_connect_disconnect_prepare(a);
	if (status != RTW_PHL_STATUS_SUCCESS) {
		RTW_ERR(FUNC_ADPT_FMT ": rtw_connect_disconnect_prepare FAIL(%u)!\n",
			FUNC_ADPT_ARG(a), status);
		goto exit;
	}
	res = _SUCCESS;

exit:
	return res;
}
#endif /* CONFIG_STA_CMD_DISPR */

#ifdef CONFIG_CMD_TSF_SYNC
static enum rtw_phl_status _send_tsf_sync_done_msg(struct _ADAPTER *a)
{
	struct dvobj_priv *d = adapter_to_dvobj(a);
	struct phl_msg msg = {0};
	struct phl_msg_attribute attr = {0};
	struct rtw_wifi_role_t *role = a->phl_role;
	enum rtw_phl_status status;


	RTW_DBG(FUNC_ADPT_FMT ": +\n", FUNC_ADPT_ARG(a));

	SET_MSG_MDL_ID_FIELD(msg.msg_id, PHL_MDL_PHY_MGNT/*PHL_MDL_GENERAL*/);
	SET_MSG_EVT_ID_FIELD(msg.msg_id, MSG_EVT_TSF_SYNC_DONE);
	msg.band_idx = role->hw_band;
	msg.inbuf = (u8 *)role;

	status = rtw_phl_send_msg_to_dispr(GET_PHL_INFO(d),
					   &msg, &attr, NULL);
	if (status != RTW_PHL_STATUS_SUCCESS) {
		RTW_ERR(FUNC_ADPT_FMT ": Send tsf sync done event fail(0x%02x)!\n",
			FUNC_ADPT_ARG(a), status);
	}

	return status;
}
#endif /* CONFIG_CMD_TSF_SYNC */

u32 rtw_enqueue_phl_cmd(struct cmd_obj *pcmd)
{
	u32 res = _FAIL;
	_adapter *padapter = pcmd->padapter;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;

	if (rtw_is_adapter_up(padapter) == _FALSE)
		goto free_cmd;

#if defined(CONFIG_STA_CMD_DISPR) || defined(CONFIG_CMD_TSF_SYNC) || defined(CONFIG_CMD_AP_DISPR)
	switch (pcmd->cmdcode) {
#ifdef CONFIG_STA_CMD_DISPR
	case CMD_JOINBSS:
		{
		struct _WLAN_BSSID_EX *network;

		network = (struct _WLAN_BSSID_EX*)pcmd->parmbuf;
		psts = rtw_connect_cmd(padapter, network);
		if (psts == RTW_PHL_STATUS_SUCCESS)
			res = _SUCCESS;
		/* pcmd & pcmd->parmbuf should be freed here */
		goto free_cmd;
		}

	case CMD_DISCONNECT:
		psts = rtw_disconnect_cmd(padapter, pcmd);
		if (psts != RTW_PHL_STATUS_SUCCESS)
			goto free_cmd;
		/* pcmd & pcmd->parmbuf would be freed in framework */
		res = _SUCCESS;
		goto exit;

	case CMD_SET_MLME_EVT:
		{
		struct rtw_evt_header *hdr;
		struct wlan_network *network;

		hdr = (struct rtw_evt_header*)pcmd->parmbuf;
		switch (hdr->id) {
		case EVT_JOINBSS:
			network = (struct wlan_network *)(pcmd->parmbuf + sizeof(*hdr));
			res = _evt_joinbss_hdl(padapter, network);
			goto free_cmd;

		case EVT_DEL_STA:
			if (MLME_IS_STA(padapter)) {
				psts = rtw_disconnect_cmd(padapter, pcmd);
				if (psts != RTW_PHL_STATUS_SUCCESS)
					goto free_cmd;
				/* pcmd & pcmd->parmbuf would be freed in framework */
				res = _SUCCESS;
				goto exit;
			}
			break;

		default:
			break;
		}
		/* not handled event would be processed later */
		break;
		}
#endif /* CONFIG_STA_CMD_DISPR */
#ifdef CONFIG_AP_CMD_DISPR
	case CMD_CREATE_BSS:
		{
		RTW_ERR("%s NEO TODO - rtw_ap_start_cmd\n", __func__);
		#if 0 // NEO
		psts = rtw_ap_start_cmd(pcmd);
		if (psts != RTW_PHL_STATUS_SUCCESS)
			goto free_cmd;
		/* pcmd & pcmd->parmbuf should be freed here */
		#endif // NEO
		res = _SUCCESS;
		goto exit;
		}
#endif /* CONFIG_AP_CMD_DISPR */


#if defined(CONFIG_CMD_TSF_SYNC) || defined (CONFIG_AP_CMD_DISPR)
	case CMD_SET_DRV_EXTRA:
		{
		struct drvextra_cmd_parm *parm;

		parm = (struct drvextra_cmd_parm *)pcmd->parmbuf;
		if (parm->ec_id == TSF_SYNC_DONE_WK_CID) {
			psts = _send_tsf_sync_done_msg(padapter);
			if (psts == RTW_PHL_STATUS_SUCCESS)
				res = _SUCCESS;
			goto free_cmd;
		}
#ifdef CONFIG_AP_CMD_DISPR
		else if (parm->ec_id == STOP_AP_WK_CID) {
			psts = rtw_ap_stop_cmd(pcmd);
			if (psts != RTW_PHL_STATUS_SUCCESS)
				goto free_cmd;
			res = _SUCCESS;
			goto exit;
		}
#endif
		break;
		}
#endif /* CONFIG_CMD_TSF_SYNC || CONFIG_AP_CMD_DISPR */

	default:
		break;
	}
#endif /* CONFIG_STA_CMD_DISPR || CONFIG_CMD_TSF_SYNC || CONFIG_CMD_AP_DISPR */

#ifdef CONFIG_CMD_GENERAL
	psts = rtw_phl_cmd_enqueue(dvobj->phl,
			padapter->phl_role->hw_band,
			MSG_EVT_LINUX_CMD_WRK,
			(u8 *)pcmd, sizeof(struct cmd_obj),
			phl_run_core_cmd,
			PHL_CMD_NO_WAIT, 0);
	if ((false == is_cmd_enqueue(psts)) && (RTW_PHL_STATUS_SUCCESS != psts))
		goto free_cmd; /* keep res == _FAIL */
#endif /* CONFIG_CMD_GENERAL */

	res = _SUCCESS;
	goto exit;

free_cmd:
	if (pcmd->cmdcode == CMD_SET_DRV_EXTRA) {
		struct drvextra_cmd_parm *extra_parm =
			(struct drvextra_cmd_parm *)pcmd->parmbuf;

		if (extra_parm->pbuf && (extra_parm->size > 0))
			rtw_mfree(extra_parm->pbuf, extra_parm->size);
	}
	rtw_free_cmd_obj(pcmd);

exit:
	return res;
}

#else /*CONFIG_FSM*/
static void phl_run_core_cmd(void *priv, void *parm, bool discard)
{
	_adapter *padapter = (_adapter *)priv;
	struct cmd_obj *pcmd = (struct cmd_obj *)parm;

	rtw_run_cmd(padapter, pcmd, discard);
}

#define PHL_RES2RES(a) (a == RTW_PHL_STATUS_SUCCESS) ? _SUCCESS : _FAIL
u32 rtw_enqueue_phl_cmd(struct cmd_obj *pcmd)
{
	u32 res = RTW_PHL_STATUS_FAILURE;
	_adapter *padapter = pcmd->padapter;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct phl_cmd_job job;
	void *msg;

	_rtw_memset(&job, 0, sizeof(job));

	switch (pcmd->cmdcode) {
	#ifndef CONFIG_CMD_SCAN
	case CMD_SITE_SURVEY:
	{
		res = rtw_site_survey_fsm(padapter, pcmd);
	}
	break;
	#endif /*CONFIG_CMD_SCAN*/
	case CMD_SET_DRV_EXTRA:
	{
		struct drvextra_cmd_parm *parm;

		parm = (struct drvextra_cmd_parm *)pcmd->parmbuf;

		if ((parm->ec_id ==  MGNT_TX_WK_CID) &&
			(rtw_cfg80211_get_is_roch(padapter))) {

			rtw_phl_job_fill_fptr(dvobj->phl, &job,
				phl_run_core_cmd, padapter,
				pcmd, rtw_cmd_name(pcmd),
				(pcmd->no_io) ? PWR_NO_IO : PWR_BASIC_IO);

			res = rtw_phl_scan_off_ch_tx(
				dvobj->phl, &job, sizeof(job));

			if (res != RTW_PHL_STATUS_SUCCESS)
				goto free_cmd;

			return PHL_RES2RES(res);
		}
	}

	/* fall through */
	default:
		rtw_phl_job_fill_fptr(dvobj->phl, &job,
			phl_run_core_cmd, padapter,
			pcmd, rtw_cmd_name(pcmd),
			(pcmd->no_io) ? PWR_NO_IO : PWR_BASIC_IO);

		res = phl_cmd_complete_job(dvobj->phl, &job);
		if (res != RTW_PHL_STATUS_SUCCESS)
			goto free_cmd;

		return PHL_RES2RES(res);
	}

free_cmd:

	if (pcmd->cmdcode == CMD_SET_DRV_EXTRA) {
		struct drvextra_cmd_parm *extra_parm =
			(struct drvextra_cmd_parm *)pcmd->parmbuf;

		if (extra_parm->pbuf && extra_parm->size > 0)
			rtw_mfree(extra_parm->pbuf, extra_parm->size);
	}
	rtw_free_cmd_obj(pcmd);

	return PHL_RES2RES(res);
}
#endif /*CONFIG_FSM*/

