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
#define _PHL_TRX_MIT_C_
#include "phl_headers.h"

#if defined(CONFIG_PCI_HCI) && defined(PCIE_TRX_MIT_EN)
struct _phl_pcie_trx_mit_info_t {
	u32 tx_timer;
	u8 tx_counter;
	u32 rx_timer;
	u8 rx_counter;
};

enum rtw_phl_status phl_pcie_trx_mit_start(struct phl_info_t *phl_info,
					   u8 dispr_idx)
{
	struct _phl_pcie_trx_mit_info_t info = {0};

	if (dispr_idx != HW_BAND_0)
		return RTW_PHL_STATUS_SUCCESS;

	PHL_INFO("%s :: pcie trx interrupt mitigation off\n", __func__);

	if (RTW_HAL_STATUS_SUCCESS !=
	    rtw_hal_pcie_trx_mit(phl_info->hal, info.tx_timer, info.tx_counter,
				 info.rx_timer, info.rx_counter))
		return RTW_PHL_STATUS_FAILURE;

	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status
phl_evt_pcie_trx_mit_hdlr(struct phl_info_t *phl_info, u8 *mit_info)
{
	struct _phl_pcie_trx_mit_info_t *info = (struct _phl_pcie_trx_mit_info_t *)mit_info;

	PHL_INFO("%s :: tx_timer == %d us, tx_counter = %d, rx_timer == %d us, "
		 "rx_counter = %d\n",
		 __func__, info->tx_timer, info->tx_counter, info->rx_timer,
		 info->rx_counter);

	if (RTW_HAL_STATUS_SUCCESS !=
	    rtw_hal_pcie_trx_mit(phl_info->hal, info->tx_timer,
				 info->tx_counter, info->rx_timer,
				 info->rx_counter))
		return RTW_PHL_STATUS_FAILURE;

	return RTW_PHL_STATUS_SUCCESS;
}

static void _phl_pcie_trx_mit_done(void *drv_priv, u8 *cmd, u32 cmd_len, enum rtw_phl_status status)
{
	if (cmd) {
		_os_mem_free(drv_priv, cmd, cmd_len);
		cmd = NULL;
	}
}

static enum rtw_phl_status
phl_pcie_trx_mit(struct phl_info_t *phl_info,
		u32 tx_timer, u8 tx_counter, u32 rx_timer, u8 rx_counter)
{
#ifdef CONFIG_CMD_DISP
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct _phl_pcie_trx_mit_info_t *info = NULL;

	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;
	u32 info_len = sizeof(struct _phl_pcie_trx_mit_info_t);

	info = _os_mem_alloc(drv_priv, info_len);
	if (info == NULL) {
		PHL_ERR("%s: alloc mit_info failed!\n", __func__);
		goto _exit;
	}

	psts = phl_cmd_enqueue(phl_info,
	                       HW_BAND_0,
	                       MSG_EVT_PCIE_TRX_MIT,
	                       (u8 *)info,
	                       info_len,
	                       _phl_pcie_trx_mit_done,
	                       PHL_CMD_NO_WAIT,
	                       0);
	if ((false == is_cmd_enqueue(psts)) && (RTW_PHL_STATUS_SUCCESS != psts))
		_os_mem_free(drv_priv, info, info_len);
_exit:
	return psts;
#else
	PHL_ERR("phl_fsm not support %s\n", __func__);
	return RTW_PHL_STATUS_FAILURE;
#endif /*CONFIG_CMD_DISP*/
}

void phl_pcie_trx_mit_watchdog(struct phl_info_t *phl_info)
{
	static enum rtw_tfc_lvl rx_traffic_lvl = RTW_TFC_IDLE;

	struct rtw_stats *phl_stats = &phl_info->phl_com->phl_stats;

	if (rx_traffic_lvl == phl_stats->rx_traffic.lvl)
		return;

	rx_traffic_lvl = phl_stats->rx_traffic.lvl;

	if (rx_traffic_lvl == RTW_TFC_HIGH)
		phl_pcie_trx_mit(phl_info, 0, 0, 100000, 200);
	else
		phl_pcie_trx_mit(phl_info, 0, 0, 0, 0);
}

#endif /*defined(CONFIG_PCI_HCI) && defined(PCIE_TRX_MIT_EN)*/