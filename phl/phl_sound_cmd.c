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

#ifdef CONFIG_CMD_DISP

/* START of sounding / beamform cmd_disp module */
void
_phl_snd_cmd_set_eng_busy(struct phl_info_t *phl_info, enum snd_cmd_disp_ctrl ctrl)
{
	struct phl_sound_obj *snd = (struct phl_sound_obj *)phl_info->snd_obj;

	if (snd != NULL) {
		snd->msg_busy |= BIT(ctrl);
	}
}

void
_phl_snd_cmd_set_eng_idle(struct phl_info_t *phl_info, enum snd_cmd_disp_ctrl ctrl)
{
	struct phl_sound_obj *snd = (struct phl_sound_obj *)phl_info->snd_obj;

	if (snd != NULL) {
		snd->msg_busy &= ~(BIT(ctrl));
	}
}

enum rtw_phl_status
_phl_snd_cmd_get_eng_status(struct phl_info_t *phl_info, enum snd_cmd_disp_ctrl ctrl)
{
	struct phl_sound_obj *snd = (struct phl_sound_obj *)phl_info->snd_obj;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	if (snd != NULL) {
		if (0 != (snd->msg_busy & BIT(ctrl))) {
			pstatus = RTW_PHL_STATUS_PENDING;
		} else {
			pstatus = RTW_PHL_STATUS_SUCCESS;
		}
	} else {
		pstatus = RTW_PHL_STATUS_RESOURCE;
	}
	return pstatus;
}

static enum phl_mdl_ret_code
_phl_snd_cmd_module_init(void *phl, void *dispr, void **priv)
{
	*priv = phl;
	return MDL_RET_SUCCESS;
}

static void _phl_snd_cmd_module_deinit(void *dispr, void *priv)
{
	return;
}

static enum phl_mdl_ret_code _phl_snd_cmd_module_start(void *dispr, void *priv)
{
	return MDL_RET_SUCCESS;
}

static enum phl_mdl_ret_code _phl_snd_cmd_module_stop(void *dispr, void *priv)
{
	return MDL_RET_SUCCESS;
}

static enum phl_mdl_ret_code
_phl_snd_cmd_module_set_info(void *dispr, void *priv,
			     struct phl_module_op_info *info)
{
	return MDL_RET_IGNORE;
}

static enum phl_mdl_ret_code
_phl_snd_cmd_module_query_info(void *dispr, void *priv,
			       struct phl_module_op_info *info)
{
	return MDL_RET_IGNORE;
}

static enum phl_mdl_ret_code
_phl_snd_cmd_module_msg_msg_hdlr_pre(struct phl_info_t *phl,
				     struct phl_msg *msg)
{
	enum phl_mdl_ret_code mstatus = MDL_RET_IGNORE;
	switch (MSG_EVT_ID_FIELD(msg->msg_id)) {
	case MSG_EVT_SET_VHT_GID:
		/* do nothing in pre phase */
	break;
	default:
		break;
	}

	return mstatus;
}

static enum phl_mdl_ret_code
_phl_snd_cmd_module_msg_msg_hdlr_post(struct phl_info_t *phl,
				      struct phl_msg *msg)
{
	enum phl_mdl_ret_code mstatus = MDL_RET_IGNORE;
	switch (MSG_EVT_ID_FIELD(msg->msg_id)) {
	case MSG_EVT_SET_VHT_GID:
	{
		struct rtw_phl_gid_pos_tbl *gid_tbl = (struct rtw_phl_gid_pos_tbl *)msg->inbuf;
		if (msg->inlen != sizeof(struct rtw_phl_gid_pos_tbl)) {
			PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_,
				  "%s() VHT-BFEE : Error, size mis-match \n", __func__);
			mstatus = MDL_RET_FAIL;
			break;
		}

		//NEO
		RTW_INFO("%s TODO rtw_hal_beamform_set_vht_gid\n", __func__);
		//rtw_hal_beamform_set_vht_gid(phl->hal, msg->band_idx, gid_tbl);
		mstatus = MDL_RET_SUCCESS;
	}
	break;
	default:
		break;
	}

	return mstatus;
}

static enum phl_mdl_ret_code
_phl_snd_cmd_module_msg_hdlr(void *dispr, void *priv,
			     struct phl_msg *msg)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)priv;
	enum phl_mdl_ret_code mstatus = MDL_RET_IGNORE;

	PHL_TRACE(COMP_PHL_SOUND, _PHL_INFO_,
		  "===> %s() event id : 0x%x \n", __func__, MSG_EVT_ID_FIELD(msg->msg_id));

	if (IS_MSG_FAIL(msg->msg_id)) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_,
			  "%s: cmd dispatcher notify cmd failure: 0x%x.\n",
			  __FUNCTION__, msg->msg_id);
		mstatus = MDL_RET_FAIL;
		return mstatus;
	}

	if (IS_MSG_IN_PRE_PHASE(msg->msg_id)) {
		mstatus = _phl_snd_cmd_module_msg_msg_hdlr_pre(phl_info, msg);
	} else {
		mstatus = _phl_snd_cmd_module_msg_msg_hdlr_post(phl_info, msg);
	}

	return mstatus;
}

enum rtw_phl_status phl_snd_cmd_register_module(struct phl_info_t *phl_info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	struct phl_bk_module_ops bk_ops;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);
	u8 i = 0;

	bk_ops.init = _phl_snd_cmd_module_init;
	bk_ops.deinit = _phl_snd_cmd_module_deinit;
	bk_ops.start = _phl_snd_cmd_module_start;
	bk_ops.stop = _phl_snd_cmd_module_stop;
	bk_ops.msg_hdlr = _phl_snd_cmd_module_msg_hdlr;
	bk_ops.set_info = _phl_snd_cmd_module_set_info;
	bk_ops.query_info = _phl_snd_cmd_module_query_info;


	for (i = 0; i < disp_eng->phy_num; i++) {
		phl_status = phl_disp_eng_register_module(phl_info, i,
						PHL_MDL_SOUND, &bk_ops);
		if (phl_status != RTW_PHL_STATUS_SUCCESS) {
			PHL_ERR("%s register SOUND module in cmd disp band[%d] failed\n",
				__func__, i);
			phl_status = RTW_PHL_STATUS_FAILURE;
			break;
		}
	}

	return phl_status;
}


/* Start of APIs for Core/OtherModule */

/* sub-functions */
/**
 * _phl_snd_cmd_post_set_vht_gid(...)
 * 	used by rtw_phl_snd_cmd_set_vht_gid(..)
 **/
void _phl_snd_cmd_post_set_vht_gid(void* priv, struct phl_msg* msg)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)priv;
	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "--> %s : release memeory \n", __func__);
	if (msg->inbuf && msg->inlen){
		_os_kmem_free(phl_to_drvpriv(phl_info), msg->inbuf, msg->inlen);
	}
	/* release cmd_disp_eng control */
	_phl_snd_cmd_set_eng_idle(phl_info, SND_CMD_DISP_CTRL_BFEE);
}
/* main-functions */
/**
 * rtw_phl_snd_cmd_set_vht_gid (...)
 * input : struct rtw_phl_gid_pos_tbl *tbl
 * 		the received VHT GID management frame's GID / Position informaion.
 **/
enum rtw_phl_status
rtw_phl_snd_cmd_set_vht_gid(void *phl,
			    struct rtw_wifi_role_t *wrole,
			    struct rtw_phl_gid_pos_tbl *tbl)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);
	struct phl_msg msg = {0};
	struct phl_msg_attribute attr = {0};
	void *d = phl_to_drvpriv(phl_info);
	struct rtw_phl_gid_pos_tbl *gid_tbl;

	/* acuired cmd_disp_eng control */
	phl_status = _phl_snd_cmd_get_eng_status(phl_info, SND_CMD_DISP_CTRL_BFEE);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		return phl_status;
	}
	_phl_snd_cmd_set_eng_busy(phl_info, SND_CMD_DISP_CTRL_BFEE);

	SET_MSG_MDL_ID_FIELD(msg.msg_id, PHL_MDL_SOUND);
	SET_MSG_EVT_ID_FIELD(msg.msg_id, MSG_EVT_SET_VHT_GID);

	msg.band_idx = wrole->hw_band;

	attr.completion.completion = _phl_snd_cmd_post_set_vht_gid;
	attr.completion.priv = phl_info;
	gid_tbl = (struct rtw_phl_gid_pos_tbl *)_os_kmem_alloc(d, sizeof(*gid_tbl));
	_os_mem_cpy(d, gid_tbl, tbl, sizeof(struct rtw_phl_gid_pos_tbl));

	msg.inbuf = (u8 *)gid_tbl;
	msg.inlen = sizeof(struct rtw_phl_gid_pos_tbl);

	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_, "%s: Get dispr fail!\n",
			  __func__);
		goto exit;
	}

	phl_status = phl_disp_eng_send_msg(phl_info, &msg, &attr, NULL);

	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_, "%s: Dispr send msg fail!\n",
			  __func__);
		goto exit;
	}

	return phl_status;

exit:
	_os_kmem_free(d, gid_tbl,
		      sizeof(struct rtw_phl_gid_pos_tbl));
	/* release cmd_disp_eng control */
	_phl_snd_cmd_set_eng_idle(phl_info, SND_CMD_DISP_CTRL_BFEE);
	return phl_status;

}

#endif
