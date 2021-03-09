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
#define _PHL_NOTIFY_C_
#include "phl_headers.h"

#ifdef CONFIG_CMD_DISP
struct cmd_notify_param {
	u8 hw_idx;
	enum phl_msg_evt_id event;
};

static void _phl_notify_done(void *drv_priv, u8 *cmd, u32 cmd_len, enum rtw_phl_status status)
{
	if (cmd) {
		_os_kmem_free(drv_priv, cmd, cmd_len);
		cmd = NULL;
		PHL_INFO("%s.....\n", __func__);
	}
}

enum rtw_phl_status
phl_notify_cmd_hdl(struct phl_info_t *phl_info, u8 *param)
{
	struct cmd_notify_param *cmd_notify = (struct cmd_notify_param *)param;

	rtw_hal_notification(phl_info->hal, cmd_notify->event, cmd_notify->hw_idx);

	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status
rtw_phl_cmd_notify(struct rtw_phl_com_t *phl_com,
                   enum phl_msg_evt_id event,
                   u8 hw_idx)
{
	void *drv = phlcom_to_drvpriv(phl_com);
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;

	struct cmd_notify_param *param = NULL;
	u32 param_len = 0;

	param_len = sizeof(struct cmd_notify_param);
	param = _os_kmem_alloc(drv, param_len);
	if (param == NULL) {
		PHL_ERR("%s: alloc param failed!\n", __func__);
		psts = RTW_PHL_STATUS_RESOURCE;
		goto error_param;
	}
	_os_mem_set(drv, param, 0, param_len);

	param->event = event;
	param->hw_idx = hw_idx;

	psts = phl_cmd_enqueue(phl_com->phl_priv,
	                       hw_idx,
	                       MSG_EVT_NOTIFY_HAL,
	                       (u8 *)param,
	                       param_len,
	                       _phl_notify_done,
	                       PHL_CMD_WAIT,
	                       0);
	if ((false == is_cmd_enqueue(psts)) && (RTW_PHL_STATUS_SUCCESS != psts))
		goto error_cmd;

	return psts;

error_cmd:
	_os_kmem_free(drv, param, param_len);

error_param:
	return psts;
}
#endif /* CONFIG_CMD_DISP */

void rtw_phl_notification(void *phl,
                          enum phl_msg_evt_id event,
                          struct rtw_wifi_role_t *wrole)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

#ifdef CONFIG_CMD_DISP
	rtw_phl_cmd_notify(phl_info->phl_com, event, wrole->hw_band);
#else
	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "%s: not support cmd notify\n",
	          __func__);

	rtw_hal_notification(phl_info->hal, event, wrole->hw_band);
#endif /* CONFIG_CMD_DISP */
}