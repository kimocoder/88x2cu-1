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
#define _RTL8822C_HALINIT_C_
#include "../hal_headers.h"
#include "../hal_api.h"


void init_hal_spec_8822c(struct rtw_phl_com_t *phl_com,
					struct hal_info_t *hal)
{
	struct hal_spec_t *hal_spec = phl_get_ic_spec(phl_com);
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	struct protocol_cap_t *hw_proto_cap = hal_com->proto_hw_cap;

	hal_spec->ic_name = "rtl8852a";
	//hal_spec->macid_num = hal_mac_get_macid_num(hal);
	hal_spec->macid_num = 128; // NEO TODO need to check macid numm of 8822cu
#if 0 //NEO
	/* hal_spec->sec_cam_ent_num follow halmac setting */
	hal_spec->sec_cap = SEC_CAP_CHK_BMC;

	hal_spec->rfpath_num_2g = 2;
	hal_spec->rfpath_num_5g = 2;
	hal_spec->rf_reg_path_num = 2;
	hal_com->rfpath_rx_num = 2;
	hal_com->rfpath_tx_num = 2;
	hal_com->phy_hw_cap[0].rx_num = 2;
	hal_com->phy_hw_cap[0].tx_num = 2;
	hal_com->phy_hw_cap[1].rx_num = 2;
	hal_com->phy_hw_cap[1].tx_num = 2;
	hal_spec->max_tx_cnt = 2;
#endif // if 0 NEO
	hal_spec->tx_nss_num = 2;
	hal_spec->rx_nss_num = 2;
	hal_spec->band_cap = BAND_CAP_2G | BAND_CAP_5G | BAND_CAP_6G;
	hal_spec->bw_cap = BW_CAP_20M | BW_CAP_40M | BW_CAP_80M;
	hal_spec->port_num = 5;

#if 0 // NEO
	hal_spec->proto_cap = PROTO_CAP_11B | PROTO_CAP_11G | PROTO_CAP_11N |
				PROTO_CAP_11AC | PROTO_CAP_11AX;

	hal_spec->wl_func = 0
				| WL_FUNC_P2P
				| WL_FUNC_MIRACAST
				| WL_FUNC_TDLS
				;

#endif // if 0
	hal_spec->max_csi_buf_su_nr = 4;
	hal_spec->max_csi_buf_mu_nr = 6;

	hal_spec->max_bf_ent_nr = 16;
	hal_spec->max_su_sta_nr = 16;
	hal_spec->max_mu_sta_nr = 6;
#if 0 //NEO
#ifdef RTW_WKARD_PHY_CAP
	/* HE */
	hw_proto_cap[0].he_su_bfme = 1;
	hw_proto_cap[0].he_su_bfmr = 1;
	hw_proto_cap[0].he_mu_bfme = 1;
	hw_proto_cap[0].he_mu_bfmr = 1;

	hw_proto_cap[1].he_su_bfme = 1;
	hw_proto_cap[1].he_su_bfmr = 1;
	hw_proto_cap[1].he_mu_bfme = 1;
	hw_proto_cap[1].he_mu_bfmr = 0;

	hw_proto_cap[0].trig_cqi_fb = 1;
	hw_proto_cap[0].non_trig_cqi_fb = 1;
	hw_proto_cap[1].trig_cqi_fb = 1;
	hw_proto_cap[1].non_trig_cqi_fb = 1;

	/* VHT */
	hw_proto_cap[0].vht_su_bfmr = 1;
	hw_proto_cap[0].vht_su_bfme = 1;
	hw_proto_cap[0].vht_mu_bfmr = 1;
	hw_proto_cap[0].vht_mu_bfme = 1;

	hw_proto_cap[1].vht_su_bfmr = 1;
	hw_proto_cap[1].vht_su_bfme = 1;
	hw_proto_cap[1].vht_mu_bfmr = 0;
	hw_proto_cap[1].vht_mu_bfme = 1;

	/* HT */
	hw_proto_cap[0].ht_su_bfmr = 1;
	hw_proto_cap[0].ht_su_bfme = 1;

	hw_proto_cap[1].ht_su_bfmr = 1;
	hw_proto_cap[1].ht_su_bfme = 1;
#endif

	/*get mac capability*/
	phl_com->dev_cap.hw_sup_flags = HW_SUP_DBCC |
			HW_SUP_AMSDU |
			HW_SUP_TCP_TX_CHKSUM |
			HW_SUP_TXPKT_CONVR;
	if (hal_com->cut_version >= CHIP_CUT_B)
		phl_com->dev_cap.hw_sup_flags |= HW_SUP_TCP_RX_CHKSUM;

	phl_com->dev_cap.hw_sup_flags |= HW_SUP_OFDMA | HW_SUP_CHAN_INFO;
	phl_com->dev_cap.hw_sup_flags |= HW_SUP_TSSI | HW_SUP_TANK_K;


#ifdef RTW_WKARD_LAMODE
	hal_com->dev_hw_cap.la_mode = true;/*TODO : get info from halbb*/
#endif

#ifdef CONFIG_DBCC_SUPPORT
	if (phl_com->dev_cap.hw_sup_flags & HW_SUP_DBCC)
		hal_com->dev_hw_cap.dbcc_sup = true;/*get info from efuse*/
#endif
	hal_com->dev_hw_cap.hw_hdr_conv = true;

#ifdef CONFIG_MCC_SUPPORT
	hal_com->dev_hw_cap.mcc_sup = true;
#endif /* CONFIG_MCC_SUPPORT */

#endif // if 0

}


void init_default_value_8822c(struct hal_info_t *hal)
{

	struct rtw_hal_com_t *hal_com = hal->hal_com;
	struct rtw_chan_def *chandef = NULL;
	u8 bid = 0;

	for (bid = 0; bid < HW_BAND_1; bid++) {

		chandef = &(hal_com->band[bid].cur_chandef);
		chandef->bw = CHANNEL_WIDTH_80;
		chandef->chan = 0;
		chandef->offset = CHAN_OFFSET_NO_EXT;
	}
}

#if 0 // NEO

u32 _hal_cfg_rom_fw_8852a(enum rtw_fw_type fw_type, struct rtw_fw_info_t *fw_info,
			  char *ic_name)
{
	char *hal_phy_folder = FW_FILE_CONFIG_PATH;
	char *filename_postfix = "";

	switch (fw_type) {
	case RTW_FW_NIC:
		filename_postfix = FW_FILE_NIC_POSTFIX;
		break;
	case RTW_FW_WOWLAN:
		filename_postfix = FW_FILE_WOWLAN_POSTFIX;
		break;
	case RTW_FW_SPIC:
		filename_postfix = FW_FILE_SPIC_POSTFIX;
		break;
	case RTW_FW_AP:
		filename_postfix = FW_FILE_AP_POSTFIX;
		break;
	default:
		break;
	}

	_os_snprintf(fw_info->rom_path, MAX_PATH_LEN, "%s%s%s%s", hal_phy_folder,
		     ic_name, _os_path_sep, "rtl8852afw_rom.bin");

	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "%s : %s\n", __func__, fw_info->rom_path);

	fw_info->rom_size = _os_read_file(fw_info->rom_path,
					  fw_info->rom_buff,
					  RTW_MAX_FW_SIZE);
	if (!fw_info->rom_size)
		return RTW_HAL_STATUS_FAILURE;

	return RTW_HAL_STATUS_SUCCESS;
}

u32 _hal_cfg_intnal_fw_8852a(struct rtw_phl_com_t *phl_com, enum rtw_fw_type fw_type, struct rtw_fw_info_t *fw_info)
{
	/* any related to fw from header can be defined here */
	return RTW_HAL_STATUS_SUCCESS;
}

u32 _hal_cfg_extnal_fw_8852a(enum rtw_fw_type fw_type, struct rtw_fw_info_t *fw_info,
			     char *ic_name)
{
	char *hal_phy_folder = FW_FILE_CONFIG_PATH;
	char *filename_postfix = "";

	switch (fw_type) {
	case RTW_FW_NIC:
		filename_postfix = FW_FILE_NIC_POSTFIX;
		break;
	case RTW_FW_WOWLAN:
		filename_postfix = FW_FILE_WOWLAN_POSTFIX;
		break;
	case RTW_FW_SPIC:
		filename_postfix = FW_FILE_SPIC_POSTFIX;
		break;
	case RTW_FW_AP:
		filename_postfix = FW_FILE_AP_POSTFIX;
		break;
	default:
		break;
	}

	_os_snprintf(fw_info->ram_path, MAX_PATH_LEN, "%s%s%s%s%s%s", hal_phy_folder,
		     ic_name, _os_path_sep, "rtl8852afw", filename_postfix, ".bin");

	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "%s : %s\n", __func__, fw_info->ram_path);

	fw_info->ram_size = _os_read_file(fw_info->ram_path,
					  fw_info->ram_buff,
					  RTW_MAX_FW_SIZE);
	if (!fw_info->ram_size)
		return RTW_HAL_STATUS_FAILURE;

	return RTW_HAL_STATUS_SUCCESS;
}

enum rtw_hal_status hal_cfg_fw_8852a(struct rtw_phl_com_t *phl_com,
				     struct hal_info_t *hal,
				     char *ic_name,
				     enum rtw_fw_type fw_type)
{
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	struct rtw_fw_info_t *fw_info = &phl_com->fw_info;
	struct rtw_fw_cap_t *fw_cap = &phl_com->dev_cap.fw_cap;

	FUNCIN();

	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "%s : fw_src %d.\n", __func__, fw_cap->fw_src);
	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "%s : dlram_en %d.\n", __func__, fw_cap->dlram_en);
	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "%s : dlrom_en %d.\n", __func__, fw_cap->dlrom_en);
	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "%s : fw_type %d.\n", __func__, fw_type);

	fw_info->fw_en = false;

	if (!fw_cap->dlram_en)
		return hstatus;

	/* Should handle fw src from header */
	if (fw_cap->dlrom_en) {
		if (RTW_HAL_STATUS_SUCCESS !=_hal_cfg_rom_fw_8852a(fw_type,fw_info,ic_name))
			goto init_fw_fail;
	}

	/* RAM */
	if (fw_cap->fw_src == RTW_FW_SRC_EXTNAL) {
		fw_info->fw_src = RTW_FW_SRC_EXTNAL;
		if (RTW_HAL_STATUS_SUCCESS != _hal_cfg_extnal_fw_8852a(fw_type,fw_info,ic_name))
			goto init_fw_fail;
	} else if (fw_cap->fw_src == RTW_FW_SRC_INTNAL) {
		fw_info->fw_src = RTW_FW_SRC_INTNAL;
		if (RTW_HAL_STATUS_SUCCESS != _hal_cfg_intnal_fw_8852a(phl_com,fw_type,fw_info))
			goto init_fw_fail;
	} else {
		goto init_fw_fail;
	}

	fw_info->fw_type = fw_type;
	fw_info->fw_en = true;
	fw_info->dlram_en = fw_cap->dlram_en;
	fw_info->dlrom_en = fw_cap->dlrom_en;

	/* fw_en, dlram_en, dlrom_en, ram_buff, ram_size, rom_buff, rom_size are ready here. */

	hstatus = RTW_HAL_STATUS_SUCCESS;

init_fw_fail:
	PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_, "%s : fw_en %d.\n", __func__, fw_info->fw_en);
	return hstatus;
}

enum rtw_hal_status hal_get_efuse_8852a(struct rtw_phl_com_t *phl_com,
					struct hal_info_t *hal,
					struct hal_init_info_t *init_info)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

	FUNCIN();

	hal_status = rtw_hal_mac_hal_fast_init(phl_com, hal, init_info);
	if (hal_status != RTW_HAL_STATUS_SUCCESS)
		goto hal_fast_init_fail;

	rtw_hal_efuse_process(hal, init_info->ic_name);

	hal_status = rtw_hal_mac_power_switch(phl_com, hal, 0);
	if (hal_status != RTW_HAL_STATUS_SUCCESS)
		goto hal_power_off_fail;

	FUNCOUT();

	return RTW_HAL_STATUS_SUCCESS;

hal_power_off_fail:
hal_fast_init_fail:
	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "==> %s : hal get efuse fail\n", __func__);
	return hal_status;
}

enum rtw_hal_status hal_start_8852a(struct rtw_phl_com_t *phl_com,
				   struct hal_info_t *hal,
				   struct hal_init_info_t *init_info)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

	/* Read phy parameter files */
	rtw_hal_dl_all_para_file(phl_com, init_info->ic_name, hal);

	hal_status = rtw_hal_mac_hal_init(phl_com, hal, init_info);
	if (hal_status != RTW_HAL_STATUS_SUCCESS)
		goto hal_init_fail;

	rtw_hal_set_rxfltr_by_mode(hal, HW_BAND_0, RX_FLTR_MODE_STA_NORMAL);
	/* MAC Suggested : 11264 Byte */
	rtw_hal_mac_set_rxfltr_mpdu_size(hal->hal_com, HW_BAND_0, 0x2c00);
	if (hal->hal_com->dbcc_en == true) {
		rtw_hal_set_rxfltr_by_mode(hal, HW_BAND_1, RX_FLTR_MODE_STA_NORMAL);
		rtw_hal_mac_set_rxfltr_mpdu_size(hal->hal_com, HW_BAND_1, 0x2c00);
	}
#ifdef CONFIG_BTCOEX
	/* power on config for btc */
	rtw_hal_btc_power_on_ntfy(hal);
#endif

	/* EFUSE config */
	rtw_hal_efuse_process(hal, init_info->ic_name);
	rtw_hal_final_cap_decision(phl_com, hal);

	/*[Pre-config BB/RF] BBRST / RFC reset */
	rtw_hal_mac_enable_bb_rf(hal, 0);
	rtw_hal_mac_enable_bb_rf(hal, 1);

	/* load parameters or config mac, phy, btc, ... */
#ifdef USE_TRUE_PHY
	rtw_hal_init_bb_reg(phl_com, hal);
	rtw_hal_init_rf_reg(phl_com, hal);
#endif

#ifdef CONFIG_BTCOEX
	/* After mac/bb/rf initialized, set btc config */
	rtw_hal_btc_init_coex_cfg_ntfy(hal);
#endif
	/* start watchdog/dm */
	rtw_hal_bb_dm_init(hal);
	rtw_hal_rf_dm_init(hal);

	/* Temporarily read hardware setting for RX
	 * ToDo: driver control MAC setting.
	 */
	{
		u32 reg32 = hal_read32(hal->hal_com, R_AX_MPDU_PROC);
		if (reg32 & B_AX_APPEND_FCS)
			phl_com->append_fcs = true;
		if (reg32 & B_AX_A_ICV_ERR)
			phl_com->accept_icv_err = true;
	}

#ifdef RTW_WKARD_HW_MGNT_GCMP_256_DISABLE
	rtw_hal_mac_config_hw_mgnt_sec(hal, false);
#endif

	PHL_INFO("==> Default ENABLE RX_PPDU_STS for Band0\n");
	/* Enable PPDU STS in default for BAND-0 for phy status */
	hal->hal_com->band[HW_BAND_0].ppdu_sts_appen_info = HAL_PPDU_MAC_INFO | HAL_PPDU_PLCP | HAL_PPDU_RX_CNT;
	hal->hal_com->band[HW_BAND_0].ppdu_sts_filter = HAL_PPDU_HAS_CRC_OK;
	rtw_hal_mac_ppdu_stat_cfg(
			hal, HW_BAND_0, true,
			hal->hal_com->band[HW_BAND_0].ppdu_sts_appen_info,
			hal->hal_com->band[HW_BAND_0].ppdu_sts_filter);

	phl_com->ppdu_sts_info.en_ppdu_sts[HW_BAND_0] = true;
	/*TODO Enable PPDU STS in default for BAND-1 for phy status */

	hal_status = rtw_hal_hdr_conv_cfg(hal, phl_com->dev_cap.hw_hdr_conv);
	if (hal_status != RTW_HAL_STATUS_SUCCESS)
		goto hal_init_fail;

	/* Enable FW basic logs */
	hal_fw_en_basic_log(hal->hal_com);

	return RTW_HAL_STATUS_SUCCESS;

hal_init_fail:
	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "==> %s : hal init fail\n", __func__);
	return hal_status;
}

enum rtw_hal_status hal_stop_8852a(struct rtw_phl_com_t *phl_com, struct hal_info_t *hal)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

#ifdef CONFIG_BTCOEX
	/* power off config for btc */
	rtw_hal_btc_power_off_ntfy(hal);
#endif
	hal_status = rtw_hal_mac_hal_deinit(phl_com, hal);
	return hal_status;
}

#ifdef CONFIG_WOWLAN
enum rtw_hal_status
hal_wow_init_8852a(struct rtw_phl_com_t *phl_com,
				struct hal_info_t *hal_info, struct rtw_phl_stainfo_t *sta,
					struct hal_init_info_t *init_info)
{
	struct hal_ops_t *hal_ops = hal_get_ops(hal_info);
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_SUCCESS;

	hal_status = hal_ops->hal_cfg_fw(phl_com, hal_info, init_info->ic_name, RTW_FW_WOWLAN);
	if (hal_status != RTW_HAL_STATUS_SUCCESS) {
		PHL_ERR("%s: cfg fw fail(%d)!!\n", __func__, hal_status);
		goto exit;
	}

	hal_status = rtw_hal_redownload_fw(phl_com, hal_info);
	if (hal_status != RTW_HAL_STATUS_SUCCESS) {
		PHL_ERR("%s: redownload fw fail(%d)!!\n", __func__, hal_status);
		goto exit;
	}

	hal_status = rtw_hal_update_sta_entry(hal_info, sta, true);
	if (hal_status != RTW_HAL_STATUS_SUCCESS) {
		PHL_ERR("%s: update sta entry fail(%d)!!\n", __func__, hal_status);
		goto exit;
	}

#ifdef RTW_WKARD_HW_MGNT_GCMP_256_DISABLE
	rtw_hal_mac_config_hw_mgnt_sec(hal_info, true);
#endif

exit:
	return hal_status;
}

enum rtw_hal_status
hal_wow_deinit_8852a(struct rtw_phl_com_t *phl_com,
				struct hal_info_t *hal_info, struct rtw_phl_stainfo_t *sta,
					struct hal_init_info_t *init_info)
{
	struct hal_ops_t *hal_ops = hal_get_ops(hal_info);
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_SUCCESS;

	/* AOAC Report */

	hal_status = hal_ops->hal_cfg_fw(phl_com, hal_info, init_info->ic_name, RTW_FW_NIC);
	if (hal_status != RTW_HAL_STATUS_SUCCESS) {
		PHL_ERR("%s: cfg fw fail(%d)!!\n", __func__, hal_status);
		goto exit;
	}

	hal_status = rtw_hal_redownload_fw(phl_com, hal_info);
	if (hal_status != RTW_HAL_STATUS_SUCCESS) {
		PHL_ERR("%s: redownload fw fail(%d)!!\n", __func__, hal_status);
		goto exit;
	}

	hal_status = rtw_hal_update_sta_entry(hal_info, sta, true);
	if (hal_status != RTW_HAL_STATUS_SUCCESS) {
		PHL_ERR("%s: update sta entry fail(%d)!!\n", __func__, hal_status);
		goto exit;
	}

	/* To Do : Recover RA ? */

#ifdef RTW_WKARD_HW_MGNT_GCMP_256_DISABLE
	rtw_hal_mac_config_hw_mgnt_sec(hal_info, false);
#endif

exit:
	return hal_status;
}
#endif /* CONFIG_WOWLAN */

#ifdef RTW_PHL_BCN //fill 8852a bcn ops

enum rtw_hal_status hal_config_beacon_8852a(struct rtw_phl_com_t *phl_com, struct hal_info_t *hal, struct rtw_bcn_entry *bcn_entry)
{
	if(hal_mac_ax_config_beacon(hal, bcn_entry) == RTW_HAL_STATUS_FAILURE)
		return RTW_HAL_STATUS_FAILURE;

	return RTW_HAL_STATUS_SUCCESS;
}


enum rtw_hal_status hal_update_beacon_8852a(struct rtw_phl_com_t *phl_com, struct hal_info_t *hal, struct rtw_bcn_entry *bcn_entry)
{
	if(hal_mac_ax_send_beacon(hal, bcn_entry) == RTW_HAL_STATUS_FAILURE)
		return RTW_HAL_STATUS_FAILURE;

	return RTW_HAL_STATUS_SUCCESS;
}

#endif //RTW_PHL_BCN

#endif // if 0 NEO
