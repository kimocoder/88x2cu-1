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
#ifndef _HAL_COM_I_C_
#define _HAL_COM_I_C_
#include "hal_headers.h"

enum rtw_hal_status rtw_hal_notify_scan_start(void *hinfo, enum phl_phy_idx phy_idx)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_SUCCESS;
#ifdef CONFIG_BTCOEX
	enum band_type band;
	struct hal_info_t *hal_info = (struct hal_info_t *)hinfo;

	band = hal_info->hal_com->band[phy_idx].cur_chandef.band;
	rtw_hal_btc_scan_start_ntfy(hal_info, phy_idx, band);
#endif
	return hal_status;
}

enum rtw_hal_status rtw_hal_notify_scan_complete(void *hinfo, enum phl_phy_idx phy_idx)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_SUCCESS;
#ifdef CONFIG_BTCOEX
	struct hal_info_t *hal_info = (struct hal_info_t *)hinfo;

	rtw_hal_btc_scan_finish_ntfy(hal_info, phy_idx);
#endif
	return hal_status;
}

enum rtw_hal_status rtw_hal_scan_pause_tx_fifo(void *hinfo,
	u8 band_idx, bool off_ch)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_SUCCESS;
	struct hal_info_t *hal_info = (struct hal_info_t *)hinfo;


	hal_status = rtw_hal_tx_pause(hal_info->hal_com, band_idx,
		off_ch, PAUSE_RSON_NOR_SCAN);

	return hal_status;
}

enum rtw_hal_status rtw_hal_dfs_pause_tx(void *hinfo,
					 u8 band_idx, bool off_ch)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_SUCCESS;
	struct hal_info_t *hal_info = (struct hal_info_t *)hinfo;

	hal_status = rtw_hal_tx_pause(hal_info->hal_com, band_idx,
				      off_ch, PAUSE_RSON_DFS);

	return hal_status;
}

enum rtw_hal_status rtw_hal_scan_flush_queue(void *hinfo,
	struct rtw_wifi_role_t *wrole)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hinfo;
	int wait = 5; /* 10ms */
	u8 val8;

	RTW_ERR("%s NEO TODO\n", __func__);

#if 0 // NEO

#if 0
	unsigned long sp_flags;
	struct phl_queue *sta_queue;
	struct rtw_phl_stainfo_t *sta;
	void *drv = hal_to_drvpriv(hal_info);
	_os_list *sta_list;
	u8 rts_txcnt_lmt_sel = 1;
	u8 rts_txcnt_lmt = 1;
	u8 data_txcnt_lmt_sel = 1;
	u8 data_tx_cnt_lmt = 1;
#endif
	rtw_hal_mac_is_tx_mgnt_empty(hal_info, wrole->hw_band, &val8);

	if (val8)
		return RTW_HAL_STATUS_SUCCESS;

#if 0
	sta_queue = &wrole->assoc_sta_queue;
	sta_list = &sta_queue->queue;

	/* TODO wait halmac api */
	/* shorten retry limit */
	_os_spinlock(drv, &sta_queue->lock, _irq, &sp_flags);
	phl_list_for_loop(sta, struct rtw_phl_stainfo_t, sta_list, list) {
		rtw_hal_mac_set_rty_lmt(hal_info->hal_com, sta->macid,
			rts_txcnt_lmt_sel, rts_txcnt_lmt,
			data_txcnt_lmt_sel, data_tx_cnt_lmt);
	}
	_os_spinunlock(drv, &sta_queue->lock, _irq, &sp_flags);
#endif
	while (wait-- && (!val8)) {
		_os_delay_ms(hal_info->hal_com->drv_priv, 2);
		rtw_hal_mac_is_tx_mgnt_empty(hal_info, wrole->hw_band, &val8);
	}
#if 0
	/* restore retry limit */
	if (wrole->cap.rty_lmt_rts == 0xFF)
		rts_txcnt_lmt_sel = 0;
	else
		rts_txcnt_lmt = wrole->cap.rty_lmt_rts & 0xF;

	if (wrole->cap.rty_lmt == 0xFF)
		data_txcnt_lmt_sel = 0;
	else
		data_tx_cnt_lmt = wrole->cap.rty_lmt & 0x3F;

	rtw_hal_mac_set_rty_lmt(hal_info->hal_com, sta->macid,
		rts_txcnt_lmt_sel, rts_txcnt_lmt,
		data_txcnt_lmt_sel, data_tx_cnt_lmt);
#endif
	if (val8 == DLE_QUEUE_EMPTY)
		return RTW_HAL_STATUS_SUCCESS;

#endif // if 0 NEO
	return RTW_HAL_STATUS_FAILURE;
}

enum rtw_hal_status rtw_hal_notify_switch_band(void *hinfo,
			enum band_type band, enum phl_phy_idx phy_idx)
{
	RTW_ERR("%s NEO TODO\n", __func__);
	return RTW_HAL_STATUS_FAILURE;

#if 0 // NEO
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_SUCCESS;
	struct hal_info_t *hal_info = (struct hal_info_t *)hinfo;

#ifdef CONFIG_BTCOEX
	rtw_hal_btc_switch_band_ntfy(hal_info, phy_idx, band);
#endif

	rtw_hal_bb_fw_edcca(hal_info);

	rtw_hal_rf_set_power(hal_info, phy_idx, PWR_BY_RATE);

	rtw_hal_rf_do_tssi_scan(hal_info, phy_idx);

	return hal_status;
#endif // if 0 NEO
}

enum rtw_hal_status
rtw_hal_proc_cmd(void *hal, char proc_cmd, struct rtw_proc_cmd *incmd,
							char *output, u32 out_len)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

	if(proc_cmd == RTW_PROC_CMD_BB){
		if(rtw_hal_bb_proc_cmd(hal_info, incmd, output, out_len))
			hal_status = RTW_HAL_STATUS_SUCCESS;
	}
	if(proc_cmd == RTW_PROC_CMD_RF){
		if(rtw_hal_rf_proc_cmd(hal_info, incmd, output, out_len))
			hal_status = RTW_HAL_STATUS_SUCCESS;
	}
	if(proc_cmd == RTW_PROC_CMD_MAC){
		RTW_INFO("%s NEO TODO RTW_PROC_CMD_MAC\n", __func__);
#if 0 // NEO
		if(rtw_hal_mac_proc_cmd(hal_info, incmd, output, out_len))
			hal_status = RTW_HAL_STATUS_SUCCESS;
#endif // if 0 NEO
	}
#ifdef CONFIG_BTCOEX
	if(proc_cmd == RTW_PROC_CMD_BTC){
		if(rtw_hal_btc_proc_cmd(hal_info, incmd, output, out_len))
			hal_status = RTW_HAL_STATUS_SUCCESS;
	}
#endif
	return hal_status;
}

void rtw_hal_get_fw_ver(void *hal, char *ver_str, u16 len)
{
	rtw_hal_mac_get_fw_ver((struct hal_info_t *)hal, ver_str, len);
}

enum rtw_hal_status
rtw_hal_set_mu_edca(void *hal, u8 band, u8 ac,
	u16 timer, u8 cw_min, u8 cw_max, u8 aifsn)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;
	u16 timer_32us;
	u8 aifs_us;
	u8 slot_time, sifs;

	RTW_INFO("%s NEO TODO\n", __func__);
#if 0
	/* TODO: Get aSlotTime and aSIFS according to current PHY */
	slot_time = 9;
	sifs = 16;
	timer_32us = (timer/4);
	aifs_us = (aifsn == 0)?0:(aifsn*slot_time + sifs);
	hal_status = rtw_hal_mac_set_mu_edca(hal_info->hal_com,
					band, ac, timer_32us, cw_min, cw_max, aifs_us);
#endif // if 0 NEO
	return hal_status;
}

enum rtw_hal_status
rtw_hal_set_mu_edca_ctrl(void *hal, u8 band, u8 wmm, u8 set)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

	RTW_INFO("%s TODO NEO\n", __func__);
#if 0 // NEO
	hal_status = rtw_hal_mac_set_mu_edca_ctrl(hal_info->hal_com,
					band, wmm, set);
#endif // if 0 NEO
	return hal_status;
}
enum rtw_hal_status
rtw_hal_reset(struct rtw_hal_com_t *hal_com, enum phl_phy_idx phy_idx, u8 band_idx, bool reset)
{
	RTW_ERR(" %s TODO NEO \n", __func__);
	return RTW_HAL_STATUS_FAILURE;
#if 0 // NEO TODO
    struct hal_info_t *hal_info = hal_com->hal_priv;
    enum rtw_hal_status status = RTW_HAL_STATUS_SUCCESS;

	if (reset){
		/* disable contention */
		status = rtw_hal_tx_pause(hal_com, band_idx, true, PAUSE_RSON_RESET);
		if(status != RTW_HAL_STATUS_SUCCESS){
			PHL_ERR("%s rtw_hal_tx_pause - failed\n", __func__);
			return status;
		}

		/* disable ppdu_sts */
		status = rtw_hal_mac_ppdu_stat_cfg(hal_info, band_idx, false, 0, 0);
		if(status != RTW_HAL_STATUS_SUCCESS){
			PHL_ERR("%s rtw_hal_mac_ppdu_stat_cfg - failed\n", __func__);
			return status;
		}

		/*disable DFS/TSSI*/
		rtw_hal_bb_dfs_en(hal_info, false);
		if (!hal_com->dbcc_en) {
			rtw_hal_bb_tssi_cont_en(hal_info, false, RF_PATH_A);
			rtw_hal_bb_tssi_cont_en(hal_info, false, RF_PATH_B);
		}else{
			if(phy_idx == HW_PHY_0)
				rtw_hal_bb_tssi_cont_en(hal_info, false, RF_PATH_A);
			else
				rtw_hal_bb_tssi_cont_en(hal_info, false, RF_PATH_B);
		}
		/*disable ADC*/
		rtw_hal_bb_adc_en(hal_info, false);
		/* wait 40us*/
		_os_delay_us(hal_com->drv_priv, 40);
		/* reset BB*/
		rtw_hal_bb_reset_en(hal_info, false, phy_idx);
	}else{
		/*enable ppdu_sts*/
		status = rtw_hal_mac_ppdu_stat_cfg(
			hal_info, band_idx, true,
			hal_com->band[band_idx].ppdu_sts_appen_info,
			hal_com->band[band_idx].ppdu_sts_filter);

		if(status != RTW_HAL_STATUS_SUCCESS){
			PHL_ERR("%s rtw_hal_mac_ppdu_stat_cfg - failed\n", __func__);
			return status;
		}
		/*enable ADC*/
		rtw_hal_bb_adc_en(hal_info, true);
		/*enable DFS/TSSI*/
		rtw_hal_bb_dfs_en(hal_info, true);
		if (!hal_com->dbcc_en) {
			rtw_hal_bb_tssi_cont_en(hal_info, true, RF_PATH_A);
			rtw_hal_bb_tssi_cont_en(hal_info, true, RF_PATH_B);
		}else{
			if(phy_idx == HW_PHY_0)
				rtw_hal_bb_tssi_cont_en(hal_info, true, RF_PATH_A);
			else
				rtw_hal_bb_tssi_cont_en(hal_info, true, RF_PATH_B);
		}
		/*BB reset set to 1*/
		rtw_hal_bb_reset_en(hal_info, true, phy_idx);

		status = rtw_hal_tx_pause(hal_com, band_idx, false, PAUSE_RSON_RESET);
		if(status != RTW_HAL_STATUS_SUCCESS){
			PHL_ERR("%s rtw_hal_tx_pause - failed\n", __func__);
			return status;
		}
	}
	return status;
#endif
}

#endif /* _HAL_COM_I_C_ */

