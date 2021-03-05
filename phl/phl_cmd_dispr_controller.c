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
#define _PHL_CMD_DISPR_CONTROLLER_C_
#include "phl_headers.h"

#ifdef CONFIG_CMD_DISP
/* Each dispr has a controller.
 * A dispr controller is designed for phl instance to interact with dispr modules that are belonged to a specific hw band,
 * phl instance can perform follwing actions via dedicated controller:
 * 1. allow (phl status/non-dispr phl modules) to monitor & drop msg
 * 2. allow dispr modules, that are belonged to same dispr, to sequentially communicate with phl instance & call phl api,
 *    and also allow (phl status/non-dispr phl modules) to notify dispr by hw band.
 * *Note*
 * 1. when cmd dispatch engine is in solo thread mode (each dispr has its own dedicated thread).
 *    phl instance might receive msg from different dispr simutaneously and
 *    currently using semaphore (dispr_ctrl_sema) to prevent multi-thread condition.
 * 2. when cmd dispatch engine is in share thread mode, msg from different dispr would pass to controller sequentially.

 * PS:
 * phl instance: means phl_info_t, which include phl mgnt status & non-dispr phl modules
 * dispr modules: all existing background & foreground modules.
 * non-dispr phl module : Data path (TX/Rx), etc
 * phl mgnt status : stop/surprise remove/cannot io
*/
enum phl_mdl_ret_code _dispr_ctrl_init(void *phl, void *dispr,
						  void **priv)
{
	PHL_INFO("%s(): \n", __func__);

	*priv = phl;
	return MDL_RET_SUCCESS;
}

void _dispr_ctrl_deinit(void *dispr, void *priv)
{
	PHL_INFO("%s(): \n", __func__);
}

enum phl_mdl_ret_code _dispr_ctrl_start(void *dispr, void *priv)
{
	enum phl_mdl_ret_code ret = MDL_RET_SUCCESS;

	PHL_INFO("%s(): \n", __func__);

	return ret;
}

enum phl_mdl_ret_code _dispr_ctrl_stop(void *dispr, void *priv)
{
	enum phl_mdl_ret_code ret = MDL_RET_SUCCESS;

	PHL_INFO("%s(): \n", __func__);

	return ret;
}

static void _fail_hdlr(void *dispr, struct phl_info_t *phl_info, struct phl_msg *msg)
{

}

static enum phl_mdl_ret_code _internal_msg_hdlr(void *dispr, struct phl_info_t *phl_info,
						      struct phl_msg *msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_IGNORE;

	/* sender is controller itself */
	switch (MSG_EVT_ID_FIELD(msg->msg_id)) {
	case MSG_EVT_DEV_RESUME_IO:
		ret = MDL_RET_SUCCESS;
	break;

	case MSG_EVT_DEV_CANNOT_IO:
		ret = MDL_RET_SUCCESS;
	break;

	case MSG_EVT_TX_RESUME:
		ret = MDL_RET_SUCCESS;
	break;

	default:
	break;
	}

	return ret;
}

static enum phl_mdl_ret_code
_external_msg_hdlr(void *dispr,
                   struct phl_info_t *phl_info,
                   struct phl_msg *msg)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	enum phl_mdl_ret_code ret = MDL_RET_IGNORE;

	if (phl_com->dev_state & ~(RTW_DEV_WORKING)) {
		PHL_TRACE(COMP_PHL_CMDDISP, _PHL_INFO_, "Controller: msg fail due to invalid device state\n");

		ret = MDL_RET_FAIL;
	} else {
		ret = MDL_RET_SUCCESS;
	}

	return ret;
}
enum phl_mdl_ret_code _dispr_ctrl_msg_hdlr(void *dispr, void *priv,
						      struct phl_msg *msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_IGNORE;
	struct phl_info_t *phl_info = (struct phl_info_t *)priv;

	FUNCIN();
	if (IS_MSG_FAIL(msg->msg_id)) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_,
		          "%s: cmd dispatcher notify cmd failure: 0x%x.\n",
		          __FUNCTION__, msg->msg_id);
		_fail_hdlr(dispr, phl_info, msg);
		FUNCOUT();
		return MDL_RET_FAIL;
	}

	if (IS_DISPR_CTRL(MSG_MDL_ID_FIELD(msg->msg_id)))
		ret = _internal_msg_hdlr(dispr, phl_info, msg);
	else
		ret = _external_msg_hdlr(dispr, phl_info, msg);
	FUNCOUT();
	return ret;
}

enum phl_mdl_ret_code
_dispr_ctrl_set_info(void *dispr, void *priv,
			 struct phl_module_op_info *info)
{
	PHL_INFO("%s(): \n", __func__);

	return MDL_RET_IGNORE;
}

enum phl_mdl_ret_code
_dispr_ctrl_query_info(void *dispr, void *priv,
			   struct phl_module_op_info *info)
{
	PHL_INFO("%s(): \n", __func__);

	return MDL_RET_IGNORE;
}

void dispr_ctrl_hook_ops(void *dispr, struct phl_bk_module_ops *ops)
{
	if (ops == NULL)
		return;
	ops->init 	= _dispr_ctrl_init;
	ops->deinit 	= _dispr_ctrl_deinit;
	ops->start 	= _dispr_ctrl_start;
	ops->stop 	= _dispr_ctrl_stop;
	ops->msg_hdlr 	= _dispr_ctrl_msg_hdlr;
	ops->set_info 	= _dispr_ctrl_set_info;
	ops->query_info = _dispr_ctrl_query_info;
}
#endif

