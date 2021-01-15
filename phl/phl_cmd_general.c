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
#define _PHL_CMD_GENERAL_C_
#include "phl_headers.h"

static enum rtw_phl_status
_phl_cmd_general_pre_phase_msg_hdlr(struct phl_info_t *phl_info, void *dispr,
				    struct phl_msg *msg)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	enum phl_msg_evt_id evt_id = MSG_EVT_ID_FIELD(msg->msg_id);

	switch (evt_id) {

#if 0 //NEO
	case MSG_EVT_PCIE_TRX_MIT:
		phl_evt_pcie_trx_mit_hdlr(phl_info, msg);
		break;
#endif

	default:
		break;
	}

	if (RTW_PHL_STATUS_SUCCESS != status)
		return RTW_PHL_STATUS_FAILURE;

	return RTW_PHL_STATUS_SUCCESS;
}

static enum rtw_phl_status
_phl_cmd_general_post_phase_msg_hdlr(struct phl_info_t *phl_info, void *dispr,
				     struct phl_msg *msg)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	enum phl_msg_evt_id evt_id = MSG_EVT_ID_FIELD(msg->msg_id);

	switch (evt_id) {

	case MSG_EVT_NONE:
		break;

	default:
		break;
	}

	if (RTW_PHL_STATUS_SUCCESS != status)
		return RTW_PHL_STATUS_FAILURE;

	return RTW_PHL_STATUS_SUCCESS;
}

static enum phl_mdl_ret_code _phl_cmd_general_init(void *phl, void *dispr,
						   void **priv)
{
	*priv = phl;
	return MDL_RET_SUCCESS;
}

static void _phl_cmd_general_deinit(void *dispr, void *priv) {}

static enum phl_mdl_ret_code _phl_cmd_general_start(void *dispr, void *priv)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)priv;
	u8 dispr_idx = 0;

	if (RTW_PHL_STATUS_SUCCESS != phl_dispr_get_idx(dispr, &dispr_idx))
		return MDL_RET_FAIL;

	//NEO
	#if 0
	if (RTW_PHL_STATUS_SUCCESS !=
	    phl_pcie_trx_mit_start(phl_info, dispr_idx))
		return MDL_RET_FAIL;
	#endif

	return MDL_RET_SUCCESS;
}

static enum phl_mdl_ret_code _phl_cmd_general_stop(void *dispr, void *priv)
{
	return MDL_RET_SUCCESS;
}

static enum phl_mdl_ret_code _phl_cmd_general_msg_hdlr(void *dispr, void *priv,
						       struct phl_msg *msg)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)priv;
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;

	if (IS_MSG_FAIL(msg->msg_id)) {

		PHL_INFO("%s :: MSG(%d)_FAIL - EVT_ID=%d \n", __func__,
			 MSG_MDL_ID_FIELD(msg->msg_id),
			 MSG_EVT_ID_FIELD(msg->msg_id));

		return MDL_RET_FAIL;
	}

	if (MSG_MDL_ID_FIELD(msg->msg_id) != PHL_MDL_GENERAL)
		return MDL_RET_IGNORE;

	if (IS_MSG_IN_PRE_PHASE(msg->msg_id))
		status =
		    _phl_cmd_general_pre_phase_msg_hdlr(phl_info, dispr, msg);
	else
		status =
		    _phl_cmd_general_post_phase_msg_hdlr(phl_info, dispr, msg);

	if (status != RTW_PHL_STATUS_SUCCESS)
		return MDL_RET_FAIL;

	return MDL_RET_SUCCESS;
}

static enum phl_mdl_ret_code
_phl_cmd_general_set_info(void *dispr, void *priv,
			  struct phl_module_op_info *info)
{
	return MDL_RET_SUCCESS;
}

static enum phl_mdl_ret_code
_phl_cmd_general_query_info(void *dispr, void *priv,
			    struct phl_module_op_info *info)
{
	return MDL_RET_SUCCESS;
}

enum rtw_phl_status phl_register_cmd_general(struct phl_info_t *phl_info)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);
	u8 band_idx = 0;

	struct phl_bk_module_ops bk_ops;
	bk_ops.init = _phl_cmd_general_init;
	bk_ops.deinit = _phl_cmd_general_deinit;
	bk_ops.start = _phl_cmd_general_start;
	bk_ops.stop = _phl_cmd_general_stop;
	bk_ops.msg_hdlr = _phl_cmd_general_msg_hdlr;
	bk_ops.set_info = _phl_cmd_general_set_info;
	bk_ops.query_info = _phl_cmd_general_query_info;

	status = RTW_PHL_STATUS_SUCCESS;
	for (band_idx = 0; band_idx < disp_eng->phy_num; band_idx++) {

		if (RTW_PHL_STATUS_SUCCESS !=
		    phl_disp_eng_register_module(phl_info, band_idx,
						 PHL_MDL_GENERAL, &bk_ops)) {

			PHL_ERR(
			    "%s register MDL_GENRAL in cmd disp failed :%d\n",
			    __func__, band_idx + 1);

			status = RTW_PHL_STATUS_FAILURE;
		}
	}

	return status;
}
