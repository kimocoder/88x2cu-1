/******************************************************************************
 *
 * Copyright(c) 2021 Realtek Corporation.
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
#define _PHL_CMD_PS_C_
#include "phl_headers.h"

struct cmd_ps {
	struct phl_info_t *phl_info;
	void *dispr;
	/* ... */
};

static enum phl_mdl_ret_code _ps_mdl_init(void *phl, void *dispr, void **priv)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct cmd_ps *ps = NULL;

	PHL_INFO("[PS_CMD], %s(): \n", __func__);

	if (priv == NULL)
		return MDL_RET_FAIL;

	(*priv) = NULL;
	ps = (struct cmd_ps *)_os_mem_alloc(phl_to_drvpriv(phl_info),
					       sizeof(struct cmd_ps));
	if (ps == NULL) {
		PHL_ERR("[PS_CMD], %s(): allocate cmd ps fail.\n", __func__);
		return MDL_RET_FAIL;
	}

	_os_mem_set(phl_to_drvpriv(phl_info), ps, 0, sizeof(struct cmd_ps));

	ps->phl_info = phl_info;
	ps->dispr = dispr;
	(*priv) = (void*)ps;

	return MDL_RET_SUCCESS;
}

static void _ps_mdl_deinit(void *dispr, void *priv)
{
	struct cmd_ps *ps = (struct cmd_ps *)priv;

	PHL_INFO("[PS_CMD], %s(): \n", __func__);

	_os_mem_free(phl_to_drvpriv(ps->phl_info), ps, sizeof(struct cmd_ps));
}

static enum phl_mdl_ret_code _ps_mdl_start(void *dispr, void *priv)
{
	enum phl_mdl_ret_code ret = MDL_RET_SUCCESS;

	PHL_INFO("[PS_CMD], %s(): \n", __func__);

	return ret;
}

static enum phl_mdl_ret_code _ps_mdl_stop(void *dispr, void *priv)
{
	enum phl_mdl_ret_code ret = MDL_RET_SUCCESS;

	PHL_INFO("[PS_CMD], %s(): \n", __func__);

	return ret;
}

static bool _is_ignored_msg(u32 msg_id)
{
	/* bypass specific msg, by required */
	return false;
}

static enum phl_mdl_ret_code
_ext_msg_pre_hdlr(struct phl_info_t *phl_info)
{
	/* power request */
	return MDL_RET_SUCCESS;
}

static enum phl_mdl_ret_code
_ext_msg_post_hdlr(struct phl_info_t *phl_info)
{
	/* cancel power request */
	return MDL_RET_SUCCESS;
}

enum phl_mdl_ret_code _ps_mdl_hdl_external_evt(
	void *dispr, struct cmd_ps *ps, struct phl_msg *msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_CANNOT_IO;
	struct phl_info_t *phl_info = ps->phl_info;

	if (_is_ignored_msg(msg->msg_id)) {
		PHL_INFO("[PS_CMD], %s(): ignore MDL_ID(%d)-EVT_ID(%d).\n", __func__,
		         MSG_MDL_ID_FIELD(msg->msg_id), MSG_EVT_ID_FIELD(msg->msg_id));
		return MDL_RET_SUCCESS;
	}

	switch (MSG_EVT_ID_FIELD(msg->msg_id)) {
	case MSG_EVT_SWCH_START:
	case MSG_EVT_PCIE_TRX_MIT:
	case MSG_EVT_FORCE_USB_SW:
	case MSG_EVT_GET_USB_SPEED:
	case MSG_EVT_GET_USB_SW_ABILITY:
	case MSG_EVT_CFG_AMPDU:
	case MSG_EVT_DFS_PAUSE_TX:
	case MSG_EVT_ROLE_RECOVER:
	case MSG_EVT_ROLE_SUSPEND:
	case MSG_EVT_HAL_SET_L2_LEAVE:
	case MSG_EVT_NOTIFY_HAL:
	case MSG_EVT_ISSUE_BCN:
	case MSG_EVT_STOP_BCN:
	case MSG_EVT_SEC_KEY:
	case MSG_EVT_ROLE_START:
	case MSG_EVT_ROLE_CHANGE:
	case MSG_EVT_ROLE_STOP:
	case MSG_EVT_STA_INFO_CTRL:
	case MSG_EVT_STA_MEDIA_STATUS_UPT:
	case MSG_EVT_CFG_CHINFO:
	case MSG_EVT_STA_CHG_STAINFO:
		PHL_INFO("[PS_CMD], %s(): MDL_ID(%d)-EVT_ID(%d) in %s phase.\n", __func__,
			MSG_MDL_ID_FIELD(msg->msg_id), MSG_EVT_ID_FIELD(msg->msg_id),
			(IS_MSG_IN_PRE_PHASE(msg->msg_id) ? "pre-protocol" : "post-protocol"));
		if (IS_MSG_IN_PRE_PHASE(msg->msg_id))
			ret = _ext_msg_pre_hdlr(phl_info);
		else
			ret = _ext_msg_post_hdlr(phl_info);
		break;
	case MSG_EVT_WATCHDOG:
		PHL_INFO("[PS_CMD], MSG_EVT_WATCHDOG\n");
		/* handle periodic check */
		ret = MDL_RET_SUCCESS; /* ret = MDL_RET_CANNOT_IO */
		break;
	default:
		ret = MDL_RET_SUCCESS; /* ret = MDL_RET_CANNOT_IO */
		break;
	}

	return ret;
}


enum phl_mdl_ret_code _ps_mdl_hdl_internal_evt(
	void *dispr, struct cmd_ps *ps, struct phl_msg *msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_CANNOT_IO;

	switch (MSG_EVT_ID_FIELD(msg->msg_id)) {
	case MSG_EVT_PHY_ON:
		if (IS_MSG_IN_PRE_PHASE(msg->msg_id)) {
			PHL_INFO("[PS_CMD], MSG_EVT_PHY_ON\n");
			/* handle power request, return MDL_RET_CANNOT_IO if power state is not meet */
			ret = MDL_RET_SUCCESS;
		} else {
			ret = MDL_RET_SUCCESS;
		}
		break;
	case MSG_EVT_PHY_IDLE:
		if (!IS_MSG_IN_PRE_PHASE(msg->msg_id)) {
			PHL_INFO("[PS_CMD], MSG_EVT_PHY_IDLE\n");
			ret = MDL_RET_SUCCESS;
		} else {
			ret = MDL_RET_SUCCESS;
		}
		break;
	case MSG_EVT_BATTERY_CHG:
		PHL_INFO("[PS_CMD], MSG_EVT_BATTERY_CHG\n");
		ret = MDL_RET_SUCCESS;
		break;
	default:
		ret = MDL_RET_SUCCESS; /* ret = MDL_RET_CANNOT_IO */
		break;
	}

	return ret;
}

static enum phl_mdl_ret_code _ps_mdl_msg_hdlr(void *dispr, void *priv,
						      struct phl_msg *msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_IGNORE;
	struct cmd_ps *ps = (struct cmd_ps *)priv;

	if (IS_MSG_FAIL(msg->msg_id)) {
		PHL_INFO("[PS_CMD], %s(): MDL_ID(%d)-EVT_ID(%d) fail.\n", __func__,
		         MSG_MDL_ID_FIELD(msg->msg_id), MSG_EVT_ID_FIELD(msg->msg_id));

	}

	switch (MSG_MDL_ID_FIELD(msg->msg_id)) {
		case PHL_MDL_POWER_MGNT:
			ret = _ps_mdl_hdl_internal_evt(dispr, ps, msg);
			break;
		default:
			ret = _ps_mdl_hdl_external_evt(dispr, ps, msg);
			break;
	}

	return ret;
}

static enum phl_mdl_ret_code
_ps_mdl_set_info(void *dispr, void *priv, struct phl_module_op_info *info)
{
	PHL_INFO("[PS_CMD], %s(): \n", __func__);

	return MDL_RET_SUCCESS;
}

static enum phl_mdl_ret_code
_ps_mdl_query_info(void *dispr, void *priv, struct phl_module_op_info *info)
{
	PHL_INFO("[PS_CMD], %s(): \n", __func__);

	return MDL_RET_SUCCESS;
}

enum rtw_phl_status phl_register_ps_module(struct phl_info_t *phl_info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);
	struct phl_bk_module_ops bk_ops = {0};
	u8 i = 0;

	PHL_INFO("[PS_CMD], %s(): \n", __func__);

	bk_ops.init = _ps_mdl_init;
	bk_ops.deinit = _ps_mdl_deinit;
	bk_ops.start = _ps_mdl_start;
	bk_ops.stop = _ps_mdl_stop;
	bk_ops.msg_hdlr = _ps_mdl_msg_hdlr;
	bk_ops.set_info = _ps_mdl_set_info;
	bk_ops.query_info = _ps_mdl_query_info;

	for (i = 0; i < disp_eng->phy_num; i++) {
		phl_status = phl_disp_eng_register_module(phl_info, i,
						 PHL_MDL_POWER_MGNT, &bk_ops);
		if (phl_status != RTW_PHL_STATUS_SUCCESS) {
			PHL_ERR("register cmd PS module of phy%d failed.\n", i + 1);
			break;
		}
	}

	return phl_status;
}
