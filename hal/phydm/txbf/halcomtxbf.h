/******************************************************************************
 *
 * Copyright(c) 2007 - 2017  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/
#ifndef __HAL_COM_TXBF_H__
#define __HAL_COM_TXBF_H__


enum txbf_set_type {
	TXBF_SET_SOUNDING_ENTER,
	TXBF_SET_SOUNDING_LEAVE,
	TXBF_SET_SOUNDING_RATE,
	TXBF_SET_SOUNDING_STATUS,
	TXBF_SET_SOUNDING_FW_NDPA,
	TXBF_SET_SOUNDING_CLK,
	TXBF_SET_TX_PATH_RESET,
	TXBF_SET_GET_TX_RATE
};

enum txbf_get_type {
	TXBF_GET_EXPLICIT_BEAMFORMEE,
	TXBF_GET_EXPLICIT_BEAMFORMER,
	TXBF_GET_MU_MIMO_STA,
	TXBF_GET_MU_MIMO_AP
};

/* @2 HAL TXBF related */
struct _HAL_TXBF_INFO {
	u8 txbf_idx;
	u8 ndpa_idx;
	u8 BW;
	u8 rate;

	struct phydm_timer_list txbf_fw_ndpa_timer;
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	RT_WORK_ITEM txbf_enter_work_item;
	RT_WORK_ITEM txbf_leave_work_item;
	RT_WORK_ITEM txbf_fw_ndpa_work_item;
	RT_WORK_ITEM txbf_clk_work_item;
	RT_WORK_ITEM txbf_status_work_item;
	RT_WORK_ITEM txbf_rate_work_item;
	RT_WORK_ITEM txbf_reset_tx_path_work_item;
	RT_WORK_ITEM txbf_get_tx_rate_work_item;
#endif
};

#define hal_com_txbf_beamform_init(dm_void) NULL
#define hal_com_txbf_config_gtab(dm_void) NULL
#define hal_com_txbf_enter_work_item_callback(_adapter) NULL
#define hal_com_txbf_leave_work_item_callback(_adapter) NULL
#define hal_com_txbf_fw_ndpa_work_item_callback(_adapter) NULL
#define hal_com_txbf_clk_work_item_callback(_adapter) NULL
#define hal_com_txbf_rate_work_item_callback(_adapter) NULL
#define hal_com_txbf_fw_ndpa_timer_callback(_adapter) NULL
#define hal_com_txbf_status_work_item_callback(_adapter) NULL
#define hal_com_txbf_get(_adapter, _get_type, _pout_buf)

#endif /*  @#ifndef __HAL_COM_TXBF_H__ */
