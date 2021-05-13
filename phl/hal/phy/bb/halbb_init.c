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
#include "halbb_precomp.h"

#if 0 //NEO
bool halbb_chk_bb_rf_pkg_set_valid(struct bb_info *bb)
{
	struct rtw_hal_com_t	*hal_i = bb->hal_com;
	u8 bb_ver = 0; /*hal_i->bb_para_pkg_ver;*/ /*TBD*/
	u8 rf_ver = 0; /*hal_i->rf_para_pkg_ver;*/ /*TBD*/
	bool valid = true;

	switch (bb->ic_type) {
	#ifdef BB_8852A_CAV_SUPPORT
	case BB_RTL8852AA:
		valid = halbb_chk_pkg_valid_8852a(bb, bb_ver, rf_ver);
		break;
	#endif

	#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		valid = halbb_chk_pkg_valid_8852a_2(bb, bb_ver, rf_ver);
		break;
	#endif
	
	#ifdef BB_8852B_SUPPORT
	case BB_RTL8852B:
		valid = halbb_chk_pkg_valid_8852b(bb, bb_ver, rf_ver);
		break;
	#endif

	#ifdef BB_8834A_SUPPORT
	case BB_RTL8834A:
		valid = halbb_chk_pkg_valid_8834a(bb, bb_ver, rf_ver);
		break;
	#endif

	default:
		break;

	}

	return valid;
}

void halbb_ic_hw_setting_init(struct bb_info *bb)
{
	switch (bb->ic_type) {

	#ifdef BB_8852A_CAV_SUPPORT
	case BB_RTL8852AA:
		halbb_ic_hw_setting_init_8852a(bb);
		break;
	#endif

	#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		halbb_ic_hw_setting_init_8852a_2(bb);
		#ifdef HALBB_DYN_CSI_RSP_SUPPORT
		halbb_dcr_init(bb);
		#endif
		break;
	#endif

	#ifdef BB_8852B_SUPPORT
	case BB_RTL8852B:
		halbb_ic_hw_setting_init_8852b(bb);
		#ifdef HALBB_DYN_CSI_RSP_SUPPORT
		halbb_dcr_init(bb);
		#endif
		break;
	#endif

	default:
		break;
	}
}

void halbb_get_efuse_init(struct bb_info *bb)
{
	switch (bb->ic_type) {

	#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		halbb_get_normal_efuse_init_8852a_2(bb);
		halbb_get_hidden_efuse_init_8852a_2(bb);
		break;
	#endif
	#ifdef BB_8852B_SUPPORT
	case BB_RTL8852B:
		halbb_get_normal_efuse_init_8852b(bb);
		halbb_get_hide_efuse_init_8852b(bb);
		break;
	#endif

	#ifdef BB_8852C_SUPPORT
	case BB_RTL8852C:
		break;
	#endif

	default:
		break;
	}
}

void halbb_cmn_info_self_init(struct bb_info *bb)
{
	struct rtw_hal_com_t	*hal_i = bb->hal_com;

	/*[IC type]*/
	if (hal_i->chip_id == CHIP_WIFI6_8852A) {
		if (hal_i->cv == CAV)
			bb->ic_type = BB_RTL8852AA;
		else
			bb->ic_type = BB_RTL8852A;
	} else if (hal_i->chip_id == CHIP_WIFI6_8852B) {
		bb->ic_type = BB_RTL8852B;
	#ifdef CONFIG_RTL8852C
	} else if (hal_i->chip_id == CHIP_WIFI6_8852C) {
		bb->ic_type = BB_RTL8852C;
	#endif
	} else if (hal_i->chip_id == CHIP_WIFI6_8834A) {
		bb->ic_type = BB_RTL8834A;
	}

	/*[CR type]*/
	if (bb->ic_type == BB_RTL8852AA)
		bb->cr_type = BB_52AA;
	else if (bb->ic_type & BB_IC_AX_AP)
		bb->cr_type = BB_AP;
	else if (bb->ic_type & BB_IC_AX_CLIENT)
		bb->cr_type = BB_CLIENT;

	/*[RF path number]*/
	if (bb->ic_type & BB_IC_1SS)
		bb->num_rf_path = 1;
	else if (bb->ic_type & BB_IC_2SS)
		bb->num_rf_path = 2;
	else if (bb->ic_type & BB_IC_3SS)
		bb->num_rf_path = 3;
	else if (bb->ic_type & BB_IC_4SS)
		bb->num_rf_path = 4;
	else
		bb->num_rf_path = 1;

	BB_DBG(bb, DBG_INIT, "cr_type=%d, num_rf_path=%d\n",
	       bb->cr_type, bb->num_rf_path);

	BB_DBG(bb, DBG_INIT,
	       "num_rf_path=%d, rate_num{Legcy, HT, VHT, HE}={%d,%d,%d,%d}\n",
	       bb->num_rf_path,
	       LEGACY_RATE_NUM, HT_RATE_NUM, VHT_RATE_NUM, HE_RATE_NUM);

	bb->manual_support_ability = 0xffffffff;
	bb->bb_dm_init_ready = false;
	bb->bb_sys_up_time = 0;
	bb->bb_watchdog_en = true;
	bb->bb_ic_api_en = true;
	/*[Drv Dbg Info]*/
	bb->cmn_dbg_msg_period = 2;
	bb->cmn_dbg_msg_cnt = 0;
	/*[Dummy]*/
	bb->bool_dummy = false;
	bb->u8_dummy = 0xff;
	bb->u16_dummy = 0xffff;
	bb->u32_dummy = 0xffffffff;
	/*@=== [HALBB Structure] ============================================*/
	bb->bb_link_i.is_linked = false;
	bb->bb_link_i.is_linked_pre = false;
	bb->bb_path_i.rx_path_en = bb->num_rf_path;
	bb->bb_path_i.tx_path_en = bb->num_rf_path;
	bb->adv_bb_dm_en = true;

	bb->bb_cmn_info_init_ready = true;
	bb->bb_watchdog_period = 2; /*sec*/

	bb->bb_cmn_hooker->bb_dm_number = sizeof(halbb_func_i) / sizeof(struct halbb_func_info);
	halbb_cmn_info_self_reset(bb);
}

u64 halbb_supportability_default(struct bb_info *bb)
{
	u64 support_ability = 0;

	switch (bb->ic_type) {
#ifdef BB_8852A_CAV_SUPPORT
	case BB_RTL8852AA:
		support_ability |=
				/*BB_RA |*/
				BB_FA_CNT |
				/*BB_DFS |*/
				BB_EDCCA |
				/*BB_ENVMNTR |*/
				BB_CFO_TRK |
				/*BB_DIG |*/
				0;
		break;
#endif
#ifdef BB_8852A_2_SUPPORT
	case BB_RTL8852A:
		support_ability |=
				/*BB_RA |*/
				BB_FA_CNT |
				BB_DFS |
				BB_EDCCA |
				BB_ENVMNTR |
				BB_CFO_TRK |
				BB_DIG |
				/*BB_ANT_DIV |*/
				0;
		break;
#endif
#ifdef BB_8852B_SUPPORT
	case BB_RTL8852B:
		support_ability |=
				BB_RA |
				BB_FA_CNT |
				BB_DFS |
				BB_EDCCA |
				BB_CFO_TRK |
				BB_ENVMNTR |
				BB_DIG |
				0;

		break;
#endif
	default:
		BB_WARNING("[%s]\n", __func__);
		break;
	}
	return support_ability;
}

void halbb_supportability_init(struct bb_info *bb)
{
	u64 support_ability;

#if 0
	if (bb->phl_com->bb_ability_manual != 0xffffffff) {
		support_ability = bb->phl_com->bb_ability_manual;
	} else
#endif
	if (bb->phl_com->drv_mode != RTW_DRV_MODE_NORMAL) {
		support_ability = 0;
	} else {
		support_ability = halbb_supportability_default(bb);
	}

	bb->support_ability = support_ability;

	BB_DBG(bb, DBG_INIT, "IC=0x%x, mp=%d, Supportability=0x%llx\n",
		  bb->ic_type, bb->phl_com->drv_mode, bb->support_ability);
}

void halbb_hw_init(struct bb_info *bb)
{
	BB_DBG(bb, DBG_INIT, "[%s] phy_idx=%d\n", __func__, bb->bb_phy_idx);

	halbb_cmn_info_self_init(bb);

	if (!halbb_chk_bb_rf_pkg_set_valid(bb)) {
		BB_WARNING("[%s] Init fail\n", __func__);
		return;
	}
}

void halbb_dm_deinit(struct rtw_phl_com_t *phl_com, void *bb_phy_0)
{
	struct bb_info *bb = (struct bb_info *)bb_phy_0;

	if (!bb->bb_dm_init_ready)
		return;

	#ifdef HALBB_LA_MODE_SUPPORT
	halbb_la_deinit(bb);
	#endif
	#ifdef HALBB_PSD_SUPPORT
	halbb_psd_deinit(bb);
	#endif
	#ifdef HALBB_CH_INFO_SUPPORT
	halbb_ch_info_deinit(bb);
	#endif
	#ifdef HALBB_DIG_SUPPORT
	halbb_dig_deinit(bb);
	#endif
	#ifdef HALBB_ANT_DIV_SUPPORT
	halbb_antdiv_deinit(bb);
	#endif
	#ifdef HALBB_DYN_CSI_RSP_SUPPORT
	halbb_dcr_deinit(bb);
	#endif
	#ifdef BB_8852A_2_SUPPORT
	#ifdef BB_TDMA_PFD
	halbb_pkt_fmt_sel_deinit_8852a_2(bb);
	#endif
	#endif

	bb->bb_dm_init_ready = false;
}

enum rtw_hal_status halbb_dm_init(struct bb_info *bb, enum phl_phy_idx phy_idx)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_SUCCESS;

	if (!bb) {
		BB_WARNING("[%s] *bb = NULL", __func__);
		return RTW_HAL_STATUS_FAILURE;
	}

	if (!bb->bb_cmn_info_init_ready) {
		BB_WARNING("bb_cmn_info_init_ready = false");
		return RTW_HAL_STATUS_FAILURE;
	}

#ifdef HALBB_DBCC_SUPPORT
	#ifdef HALBB_DBCC_DVLP_FLAG
	if (phy_idx == HW_PHY_1)
		return hal_status;
	#endif
	bb = halbb_get_curr_bb_pointer(bb, phy_idx);
	BB_DBG(bb, DBG_INIT, "[%s] phy_idx=%d\n", __func__, bb->bb_phy_idx);
#endif
	halbb_ic_hw_setting_init(bb);
	halbb_gpio_setting_init(bb);
	halbb_get_efuse_init(bb);

	halbb_supportability_init(bb);
	halbb_physts_parsing_init(bb);
	halbb_cmn_rpt_init(bb);
	halbb_dbg_setting_init(bb);

	#ifdef HALBB_PWR_CTRL_SUPPORT
	halbb_macid_ctrl_init(bb);
	#endif
	#ifdef HALBB_STATISTICS_SUPPORT
	halbb_statistics_init(bb);
	#endif
	#ifdef HALBB_LA_MODE_SUPPORT
	halbb_la_init(bb);
	#endif
	#ifdef HALBB_PSD_SUPPORT
	halbb_psd_init(bb);
	#endif
	#ifdef HALBB_EDCCA_SUPPORT
	halbb_edcca_init(bb);
	#endif
	#ifdef HALBB_DFS_SUPPORT
	halbb_dfs_init(bb);
	#endif
	#ifdef HALBB_DIG_SUPPORT
	halbb_dig_init(bb);
	#endif
	#ifdef HALBB_CFO_TRK_SUPPORT
	halbb_cfo_trk_init(bb);
	#endif
	#ifdef HALBB_RA_SUPPORT
	halbb_ra_init(bb);
	#endif
	#ifdef HALBB_ENV_MNTR_SUPPORT
	halbb_env_mntr_init(bb);
	#endif
	#ifdef HALBB_PWR_CTRL_SUPPORT
	halbb_pwr_ctrl_init(bb);
	#endif
	#ifdef HALBB_RUA_SUPPORT
	halbb_rua_tbl_init(bb);
	#endif
	#ifdef HALBB_ANT_DIV_SUPPORT
	halbb_antdiv_init(bb);
	#endif
	#ifdef HALBB_CH_INFO_SUPPORT
	halbb_ch_info_init(bb);
	#endif
	halbb_reset_adc(bb);

	bb->bb_dm_init_ready = true;
	BB_DBG(bb, DBG_INIT, "bb_init_ready = %d\n", bb->bb_dm_init_ready);

	return hal_status;
}

void halbb_cr_cfg_init(struct bb_info *bb)
{
	halbb_cr_cfg_dbg_init(bb);
	halbb_cr_cfg_physts_init(bb);
	#ifdef HALBB_STATISTICS_SUPPORT
	halbb_cr_cfg_stat_init(bb);
	#endif
	#ifdef HALBB_PSD_SUPPORT
	halbb_cr_cfg_psd_init(bb);
	#endif
	#ifdef HALBB_DIG_SUPPORT
	halbb_cr_cfg_dig_init(bb);
	#endif
	#ifdef HALBB_ENV_MNTR_SUPPORT
	halbb_cr_cfg_env_mntr_init(bb);
	#endif
	#ifdef HALBB_EDCCA_SUPPORT
	halbb_cr_cfg_edcca_init(bb);
	#endif
	#ifdef HALBB_DFS_SUPPORT
	halbb_cr_cfg_dfs_init(bb);
	#endif
	#ifdef HALBB_LA_MODE_SUPPORT
	halbb_cr_cfg_la_init(bb);
	#endif
	#ifdef HALBB_ANT_DIV_SUPPORT
	halbb_cr_cfg_antdiv_init(bb);
	#endif
	#ifdef HALBB_PMAC_TX_SUPPORT
	halbb_cr_cfg_plcp_init(bb);
	#endif
	halbb_cr_cfg_mp_init(bb);
	#ifdef HALBB_CH_INFO_SUPPORT
	halbb_cr_cfg_ch_info_init(bb);
	#endif
}

#endif // NEO

void halbb_buffer_deinit(struct rtw_phl_com_t *phl_com,
			 struct rtw_hal_com_t *hal_com, void *bb_phy_0)
{
	struct bb_info *bb = (struct bb_info *)bb_phy_0;

	/*Deinit phy-cmn*/
	if (bb->bb_cmn_hooker) {
		BB_DBG(bb, DBG_INIT, "deinit bb_cmn_hooker");
		hal_mem_free(hal_com, bb->bb_cmn_hooker, sizeof(struct bb_cmn_info));
	}

	/*Deinit phy-0*/
	if (bb) {
		BB_DBG(bb, DBG_INIT, "deinit phy-%d", bb->bb_phy_idx);
		hal_mem_free(hal_com, bb, sizeof(struct bb_info));
	}
}


u32
halbb_buffer_init(struct rtw_phl_com_t *phl_com,
			struct rtw_hal_com_t *hal_com, void **bb_out_addr)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_SUCCESS;
	struct bb_info *bb_0 = NULL;
	struct bb_cmn_info *bb_cmn = NULL;

	bb_0 = hal_mem_alloc(hal_com, sizeof(struct bb_info));

	if (!bb_0) {
		BB_WARNING("*bb = NULL\n");
		return RTW_HAL_STATUS_BB_INIT_FAILURE;
	}

	RTW_INFO("%s NEO TODO\n", __func__);
	*bb_out_addr = bb_0;

	bb_0->phl_com = phl_com;/*shared memory for all components*/
	bb_0->hal_com = hal_com;/*shared memory for phl and hal*/
	bb_0->bb_phy_idx = HW_PHY_0;

	bb_cmn = hal_mem_alloc(hal_com, sizeof(struct bb_cmn_info));
	if (!bb_cmn) {
		BB_WARNING("*bb_cmn = NULL\n");
		return RTW_HAL_STATUS_BB_INIT_FAILURE;
	}
	
	bb_0->bb_cmn_hooker = bb_cmn;

	halbb_dbg_comp_init(bb_0);
#if 0 // NEO
	halbb_hw_init(bb_0);
	halbb_cr_cfg_init(bb_0);
#endif // NEO

	BB_DBG(bb_0, DBG_INIT, "[%s]\n", __func__);
	BB_DBG(bb_0, DBG_INIT, "  %-35s: %s\n", "Code Base:", HLABB_CODE_BASE);
	BB_DBG(bb_0, DBG_INIT, "  %-35s: %s\n", "Code Release Date", HALBB_RELEASE_DATE);
	return (u32)hal_status;
}

