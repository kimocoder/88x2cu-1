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

/*@
 * ============================================================
 * include files
 * ============================================================
 */

#include "mp_precomp.h"
#include "phydm_precomp.h"

#if defined(CONFIG_PHYDM_DFS_MASTER)

boolean phydm_dfs_is_meteorology_channel(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	u8 ch = *dm->channel;
	u8 bw = *dm->band_width;

	return ((bw  == CHANNEL_WIDTH_80 && (ch) >= 116 && (ch) <= 128) ||
		(bw  == CHANNEL_WIDTH_40 && (ch) >= 116 && (ch) <= 128) ||
		(bw  == CHANNEL_WIDTH_20 && (ch) >= 120 && (ch) <= 128));
}

void phydm_dfs_segment_distinguish(void *dm_void, enum rf_syn syn_path)
{
}

void phydm_dfs_segment_flag_reset(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

}

void phydm_radar_detect_reset(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	odm_set_bb_reg(dm, R_0xa40, BIT(15), 0);
	odm_set_bb_reg(dm, R_0xa40, BIT(15), 1);
}

void phydm_radar_detect_disable(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	odm_set_bb_reg(dm, R_0xa40, BIT(15), 0);

	PHYDM_DBG(dm, DBG_DFS, "\n");
}

static void phydm_radar_detect_with_dbg_parm(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	odm_set_bb_reg(dm, R_0xa40, MASKDWORD,
		       dm->radar_detect_reg_a40);
	odm_set_bb_reg(dm, R_0xa44, MASKDWORD,
		       dm->radar_detect_reg_a44);
	odm_set_bb_reg(dm, R_0xa48, MASKDWORD,
		       dm->radar_detect_reg_a48);
	odm_set_bb_reg(dm, R_0xa4c, MASKDWORD,
		       dm->radar_detect_reg_a4c);
	odm_set_bb_reg(dm, R_0xa50, MASKDWORD,
		       dm->radar_detect_reg_a50);
	odm_set_bb_reg(dm, R_0xa54, MASKDWORD,
		       dm->radar_detect_reg_a54);
}

/* @Init radar detection parameters, called after ch, bw is set */

void phydm_radar_detect_enable(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _DFS_STATISTICS *dfs = &dm->dfs;
	u8 region_domain = dm->dfs_region_domain;
	u8 c_channel = *dm->channel;
	u8 band_width = *dm->band_width;
	u8 enable = 0, i;
	u8 short_pw_upperbound = 0;

	PHYDM_DBG(dm, DBG_DFS, "test, region_domain = %d\n", region_domain);
	if (region_domain == PHYDM_DFS_DOMAIN_UNKNOWN) {
		PHYDM_DBG(dm, DBG_DFS, "PHYDM_DFS_DOMAIN_UNKNOWN\n");
		goto exit;
	}

	if (dm->radar_detect_dbg_parm_en) {
		phydm_radar_detect_with_dbg_parm(dm);
		enable = 1;
		goto exit;
	}
	if (region_domain == PHYDM_DFS_DOMAIN_ETSI) {
		odm_set_bb_reg(dm, R_0xa40, MASKDWORD, 0xb359c5bd);
		odm_set_bb_reg(dm, R_0xa44, MASKDWORD, 0x3033bebd);
		odm_set_bb_reg(dm, R_0xa48, MASKDWORD, 0x2a521254);
		odm_set_bb_reg(dm, R_0xa4c, MASKDWORD, 0xa2533345);
		odm_set_bb_reg(dm, R_0xa50, MASKDWORD, 0x605be003);
		odm_set_bb_reg(dm, R_0xa54, MASKDWORD, 0x500089e8);
	} else if (region_domain == PHYDM_DFS_DOMAIN_MKK) {
		odm_set_bb_reg(dm, R_0xa40, MASKDWORD, 0xb359c5bd);
		odm_set_bb_reg(dm, R_0xa44, MASKDWORD, 0x3033bebd);
		odm_set_bb_reg(dm, R_0xa48, MASKDWORD, 0x2a521254);
		odm_set_bb_reg(dm, R_0xa4c, MASKDWORD, 0xa2533345);
		odm_set_bb_reg(dm, R_0xa50, MASKDWORD, 0x605be003);
		odm_set_bb_reg(dm, R_0xa54, MASKDWORD, 0x500089e8);
	} else if (region_domain == PHYDM_DFS_DOMAIN_FCC) {
		odm_set_bb_reg(dm, R_0xa40, MASKDWORD, 0xb359c5bd);
		odm_set_bb_reg(dm, R_0xa44, MASKDWORD, 0x3033bebd);
		odm_set_bb_reg(dm, R_0xa48, MASKDWORD, 0x2a521254);
		odm_set_bb_reg(dm, R_0xa4c, MASKDWORD, 0xa2533345);
		odm_set_bb_reg(dm, R_0xa50, MASKDWORD, 0x605be003);
		odm_set_bb_reg(dm, R_0xa54, MASKDWORD, 0x500089e8);
	} else {
		/* not supported */
		PHYDM_DBG(dm, DBG_DFS,
			  "Unsupported dfs_region_domain:%d\n",
			  region_domain);
		goto exit;
	}

	enable = 1;

	dfs->st_l2h_cur = (u8)odm_get_bb_reg(dm, R_0xa40, 0x00007f00);
	dfs->pwdb_th_cur = (u8)odm_get_bb_reg(dm, R_0xa50, 0x000000f0);
	dfs->peak_th = (u8)odm_get_bb_reg(dm, R_0xa48, 0x00c00000);
	dfs->short_pulse_cnt_th = (u8)odm_get_bb_reg(dm, R_0xa50,
						     0x00f00000);
	dfs->long_pulse_cnt_th = (u8)odm_get_bb_reg(dm, R_0xa4c,
						    0xf0000000);
	dfs->peak_window = (u8)odm_get_bb_reg(dm, R_0xa40, 0x00030000);
	dfs->three_peak_opt = (u8)odm_get_bb_reg(dm, R_0xa40,
						 0x30000000);
	dfs->three_peak_th2 = (u8)odm_get_bb_reg(dm, R_0xa44,
						 0x00000007);

	phydm_dfs_parameter_init(dm);

exit:
	if (enable) {
		phydm_radar_detect_reset(dm);
		PHYDM_DBG(dm, DBG_DFS, "on cch:%u, bw:%u\n", c_channel,
			  band_width);
	} else
		phydm_radar_detect_disable(dm);
}

void phydm_dfs_parameter_init(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _DFS_STATISTICS *dfs = &dm->dfs;

	u8 i;
	for (i = 0; i < 5; i++) {
		dfs->pulse_flag_hist[i] = 0;
		dfs->pulse_type_hist[i] = 0;
		dfs->radar_det_mask_hist[i] = 0;
		dfs->fa_inc_hist[i] = 0;
	}

	/*@for dfs mode*/
	dfs->force_TP_mode = 0;
	dfs->sw_trigger_mode = 0;
	dfs->det_print = 0;
	dfs->det_print2 = 0;
	dfs->print_hist_rpt = 0;
	dfs->hist_cond_on = 0;

	/*@for dynamic dfs*/
	dfs->pwdb_th = 8;
	dfs->fa_mask_th = 30 * (dfs->dfs_polling_time) / 100;
	dfs->st_l2h_min = 0x20;
	dfs->st_l2h_max = 0x4e;
	dfs->pwdb_scalar_factor = 12;

	/*@for dfs histogram*/
	dfs->pri_hist_th = 5;
	dfs->pri_sum_g1_th = 9;
	dfs->pri_sum_g5_th = 5;
	dfs->pri_sum_g1_fcc_th = 4;		/*@FCC Type6*/
	dfs->pri_sum_g3_fcc_th = 6;
	dfs->pri_sum_safe_th = 50;
	dfs->pri_sum_safe_fcc_th = 110;		/*@30 for AP*/
	dfs->pri_sum_type4_th = 16;
	dfs->pri_sum_type6_th = 12;
	dfs->pri_sum_g5_under_g1_th = 4;
	dfs->pri_pw_diff_th = 4;
	dfs->pri_pw_diff_fcc_th = 8;
	dfs->pri_pw_diff_fcc_idle_th = 2;
	dfs->pri_pw_diff_w53_th = 10;
	dfs->pw_std_th = 7;			/*@FCC Type4*/
	dfs->pw_std_idle_th = 10;
	dfs->pri_std_th = 6;			/*@FCC Type3,4,6*/
	dfs->pri_std_idle_th = 10;
	dfs->pri_type1_upp_fcc_th = 110;
	dfs->pri_type1_low_fcc_th = 50;
	dfs->pri_type1_cen_fcc_th = 70;
	dfs->pw_g0_th = 8;
	dfs->pw_long_lower_th = 6;		/*@7->6*/
	dfs->pri_long_upper_th = 30;
	dfs->pw_long_lower_20m_th = 7;		/*@7 for AP*/
	dfs->pw_long_sum_upper_th = 60;
	dfs->type4_pw_max_cnt = 7;
	dfs->type4_safe_pri_sum_th = 5;
}

void phydm_dfs_dynamic_setting(
	void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _DFS_STATISTICS *dfs = &dm->dfs;

	u8 peak_th_cur = 0, short_pulse_cnt_th_cur = 0;
	u8 long_pulse_cnt_th_cur = 0, three_peak_opt_cur = 0;
	u8 three_peak_th2_cur = 0;
	u8 peak_window_cur = 0;
	u8 region_domain = dm->dfs_region_domain;
	u8 c_channel = *dm->channel;

	if (dm->rx_tp + dm->tx_tp <= 2) {
		dfs->idle_mode = 1;
		if (dfs->force_TP_mode)
			dfs->idle_mode = 0;
	} else {
		dfs->idle_mode = 0;
	}

	if (dfs->idle_mode == 1) { /*@idle (no traffic)*/
		peak_th_cur = 3;
		short_pulse_cnt_th_cur = 6;
		long_pulse_cnt_th_cur = 9;
		peak_window_cur = 2;
		three_peak_opt_cur = 0;
		three_peak_th2_cur = 2;
		if (region_domain == PHYDM_DFS_DOMAIN_MKK) {
			if (c_channel >= 52 && c_channel <= 64) {
				short_pulse_cnt_th_cur = 14;
				long_pulse_cnt_th_cur = 15;
				three_peak_th2_cur = 0;
			} else {
				short_pulse_cnt_th_cur = 6;
				three_peak_th2_cur = 0;
				long_pulse_cnt_th_cur = 10;
			}
		} else if (region_domain == PHYDM_DFS_DOMAIN_FCC) {
			three_peak_th2_cur = 0;
		} else if (region_domain == PHYDM_DFS_DOMAIN_ETSI) {
			long_pulse_cnt_th_cur = 15;
			if (phydm_dfs_is_meteorology_channel(dm)) {
			/*need to add check cac end condition*/
				peak_th_cur = 2;
				three_peak_opt_cur = 0;
				three_peak_th2_cur = 0;
				short_pulse_cnt_th_cur = 7;
			} else {
				three_peak_opt_cur = 0;
				three_peak_th2_cur = 0;
				short_pulse_cnt_th_cur = 7;
			}
		} else /*@default: FCC*/
			three_peak_th2_cur = 0;

	} else { /*@in service (with TP)*/
		peak_th_cur = 2;
		short_pulse_cnt_th_cur = 6;
		long_pulse_cnt_th_cur = 7;
		peak_window_cur = 2;
		three_peak_opt_cur = 0;
		three_peak_th2_cur = 2;
		if (region_domain == PHYDM_DFS_DOMAIN_MKK) {
			if (c_channel >= 52 && c_channel <= 64) {
				long_pulse_cnt_th_cur = 15;
				/*@for high duty cycle*/
				short_pulse_cnt_th_cur = 5;
				three_peak_th2_cur = 0;
			} else {
				three_peak_opt_cur = 0;
				three_peak_th2_cur = 0;
				long_pulse_cnt_th_cur = 8;
			}
		} else if (region_domain == PHYDM_DFS_DOMAIN_FCC) {
			long_pulse_cnt_th_cur = 5;	/*for 80M FCC*/
			short_pulse_cnt_th_cur = 5;	/*for 80M FCC*/
		} else if (region_domain == PHYDM_DFS_DOMAIN_ETSI) {
			long_pulse_cnt_th_cur = 15;
			short_pulse_cnt_th_cur = 5;
			three_peak_opt_cur = 0;
		}
	}

	if (dfs->peak_th != peak_th_cur)
		odm_set_bb_reg(dm, R_0xa48, 0x00c00000, peak_th_cur);
	if (dfs->short_pulse_cnt_th != short_pulse_cnt_th_cur)
		odm_set_bb_reg(dm, R_0xa50, 0x00f00000,
			       short_pulse_cnt_th_cur);
	if (dfs->long_pulse_cnt_th != long_pulse_cnt_th_cur)
		odm_set_bb_reg(dm, R_0xa4c, 0xf0000000,
			       long_pulse_cnt_th_cur);
	if (dfs->peak_window != peak_window_cur)
		odm_set_bb_reg(dm, R_0xa40, 0x00030000,
			       peak_window_cur);
	if (dfs->three_peak_opt != three_peak_opt_cur)
		odm_set_bb_reg(dm, R_0xa40, 0x30000000,
			       three_peak_opt_cur);
	if (dfs->three_peak_th2 != three_peak_th2_cur)
		odm_set_bb_reg(dm, R_0xa44, 0x00000007,
			       three_peak_th2_cur);

	dfs->peak_th = peak_th_cur;
	dfs->short_pulse_cnt_th = short_pulse_cnt_th_cur;
	dfs->long_pulse_cnt_th = long_pulse_cnt_th_cur;
	dfs->peak_window = peak_window_cur;
	dfs->three_peak_opt = three_peak_opt_cur;
	dfs->three_peak_th2 = three_peak_th2_cur;
}

boolean
phydm_radar_detect_dm_check(
	void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _DFS_STATISTICS *dfs = &dm->dfs;
	u8 region_domain = dm->dfs_region_domain, index = 0;

	u16 i = 0, j = 0, fa_count_cur = 0, fa_count_inc = 0;
	u16 total_fa_in_hist = 0, total_pulse_count_inc = 0;
	u16 short_pulse_cnt_inc = 0, short_pulse_cnt_cur = 0;
	u16 long_pulse_cnt_inc = 0, long_pulse_cnt_cur = 0;
	u32 regf98_value = 0, reg918_value = 0, reg91c_value = 0;
	u32 reg920_value = 0, reg924_value = 0, radar_rpt_reg_value = 0;
	u32 regf54_value = 0, regf58_value = 0, regf5c_value = 0;
	u32 regdf4_value = 0, regf70_value = 0, regf74_value = 0;
	#if (RTL8812F_SUPPORT || RTL8822C_SUPPORT || RTL8814B_SUPPORT)
	u32 rega40_value = 0, rega44_value = 0, rega48_value = 0;
	u32 rega4c_value = 0, rega50_value = 0, rega54_value = 0;
	#endif
	#if (RTL8721D_SUPPORT)
	u32 reg908_value = 0, regdf4_value = 0;
	u32 regf54_value = 0, regf58_value = 0, regf5c_value = 0;
	u32 regf70_value = 0, regf74_value = 0;
	#endif
	boolean tri_short_pulse = 0, tri_long_pulse = 0, radar_type = 0;
	boolean fault_flag_det = 0, fault_flag_psd = 0, fa_flag = 0;
	boolean radar_detected = 0;
	u8 st_l2h_new = 0, fa_mask_th = 0, k = 0, sum = 0;
	u8 c_channel = *dm->channel;

	/*@Get FA count during past 100ms, R_0xf48 for AC series*/
	fa_count_cur = (u16)odm_get_bb_reg(dm, R_0x2d00, MASKLWORD);

	if (dfs->fa_count_pre == 0)
		fa_count_inc = 0;
	else if (fa_count_cur >= dfs->fa_count_pre)
		fa_count_inc = fa_count_cur - dfs->fa_count_pre;
	else
		fa_count_inc = fa_count_cur;
	dfs->fa_count_pre = fa_count_cur;

	dfs->fa_inc_hist[dfs->mask_idx] = fa_count_inc;

	for (i = 0; i < 5; i++)
		total_fa_in_hist = total_fa_in_hist + dfs->fa_inc_hist[i];

	if (dfs->mask_idx >= 2)
		index = dfs->mask_idx - 2;
	else
		index = 5 + dfs->mask_idx - 2;

	radar_rpt_reg_value = odm_get_bb_reg(dm, R_0x2e00, 0xffffffff);
	short_pulse_cnt_cur = (u16)((radar_rpt_reg_value & 0x000ff800)
				    >> 11);
	long_pulse_cnt_cur = (u16)((radar_rpt_reg_value & 0x0fc00000)
				    >> 22);

	/*@Get short pulse count, need carefully handle the counter overflow*/
	if (short_pulse_cnt_cur >= dfs->short_pulse_cnt_pre) {
		short_pulse_cnt_inc = short_pulse_cnt_cur -
				      dfs->short_pulse_cnt_pre;
	} else {
		short_pulse_cnt_inc = short_pulse_cnt_cur;
	}
	dfs->short_pulse_cnt_pre = short_pulse_cnt_cur;

	/*@Get long pulse count, need carefully handle the counter overflow*/
	if (long_pulse_cnt_cur >= dfs->long_pulse_cnt_pre) {
		long_pulse_cnt_inc = long_pulse_cnt_cur -
				     dfs->long_pulse_cnt_pre;
	} else {
		long_pulse_cnt_inc = long_pulse_cnt_cur;
	}
	dfs->long_pulse_cnt_pre = long_pulse_cnt_cur;

	total_pulse_count_inc = short_pulse_cnt_inc + long_pulse_cnt_inc;

	if (dfs->det_print) {
		PHYDM_DBG(dm, DBG_DFS,
			  "===============================================\n");
		PHYDM_DBG(dm, DBG_DFS, "FA_count_inc[%d]\n", fa_count_inc);
		PHYDM_DBG(dm, DBG_DFS,
			  "Init_Gain[%x] st_l2h_cur[%x] 0x2dbc[%08x] short_pulse_cnt_inc[%d] long_pulse_cnt_inc[%d]\n",
			  dfs->igi_cur, dfs->st_l2h_cur,
			  radar_rpt_reg_value, short_pulse_cnt_inc,
			  long_pulse_cnt_inc);
		rega40_value = odm_get_bb_reg(dm, R_0xa40, MASKDWORD);
		rega44_value = odm_get_bb_reg(dm, R_0xa44, MASKDWORD);
		rega48_value = odm_get_bb_reg(dm, R_0xa48, MASKDWORD);
		rega4c_value = odm_get_bb_reg(dm, R_0xa4c, MASKDWORD);
		rega50_value = odm_get_bb_reg(dm, R_0xa50, MASKDWORD);
		rega54_value = odm_get_bb_reg(dm, R_0xa54, MASKDWORD);
		PHYDM_DBG(dm, DBG_DFS,
			  "0xa40[%08x] 0xa44[%08x] 0xa48[%08x] 0xa4c[%08x] 0xa50[%08x] 0xa54[%08x]\n",
			  rega40_value, rega44_value, rega48_value,
			  rega4c_value, rega50_value, rega54_value);
		PHYDM_DBG(dm, DBG_DFS, "Throughput: %dMbps\n",
			  (dm->rx_tp + dm->tx_tp));

		PHYDM_DBG(dm, DBG_DFS,
			  "dfs_regdomain = %d, dbg_mode = %d, idle_mode = %d, print_hist_rpt = %d, hist_cond_on = %d\n",
			  region_domain, dfs->dbg_mode,
			  dfs->idle_mode, dfs->print_hist_rpt,
			  dfs->hist_cond_on);
	}
	tri_short_pulse = (radar_rpt_reg_value & BIT(20)) ? 1 : 0;
	tri_long_pulse = (radar_rpt_reg_value & BIT(28)) ? 1 : 0;

	if (tri_short_pulse) {
		phydm_radar_detect_reset(dm);
	}
	if (tri_long_pulse) {
		phydm_radar_detect_reset(dm);
		if (region_domain == PHYDM_DFS_DOMAIN_MKK) {
			if (c_channel >= 52 && c_channel <= 64) {
				tri_long_pulse = 0;
			}
		}
		if (region_domain == PHYDM_DFS_DOMAIN_ETSI) {
			tri_long_pulse = 0;
		}
	}

	st_l2h_new = dfs->st_l2h_cur;
	dfs->pulse_flag_hist[dfs->mask_idx] = tri_short_pulse | tri_long_pulse;
	dfs->pulse_type_hist[dfs->mask_idx] = (tri_long_pulse) ? 1 : 0;

	/* PSD(not ready) */

	fault_flag_det = 0;
	fault_flag_psd = 0;
	fa_flag = 0;
	if (region_domain == PHYDM_DFS_DOMAIN_ETSI) {
		fa_mask_th = dfs->fa_mask_th + 20;
	} else {
		fa_mask_th = dfs->fa_mask_th;
	}
	if (total_fa_in_hist >= fa_mask_th || dfs->igi_cur >= 0x30) {
		/* st_l2h_new = dfs->st_l2h_max; */
		dfs->radar_det_mask_hist[index] = 1;
		if (dfs->pulse_flag_hist[index] == 1) {
			dfs->pulse_flag_hist[index] = 0;
			if (dfs->det_print2) {
				PHYDM_DBG(dm, DBG_DFS,
					  "Radar is masked : FA mask\n");
			}
		}
		fa_flag = 1;
	} else {
		dfs->radar_det_mask_hist[index] = 0;
	}

	if (dfs->det_print) {
		PHYDM_DBG(dm, DBG_DFS, "mask_idx: %d\n", dfs->mask_idx);
		PHYDM_DBG(dm, DBG_DFS, "radar_det_mask_hist: ");
		for (i = 0; i < 5; i++)
			PHYDM_DBG(dm, DBG_DFS, "%d ",
				  dfs->radar_det_mask_hist[i]);
		PHYDM_DBG(dm, DBG_DFS, "pulse_flag_hist: ");
		for (i = 0; i < 5; i++)
			PHYDM_DBG(dm, DBG_DFS, "%d ", dfs->pulse_flag_hist[i]);
		PHYDM_DBG(dm, DBG_DFS, "fa_inc_hist: ");
		for (i = 0; i < 5; i++)
			PHYDM_DBG(dm, DBG_DFS, "%d ", dfs->fa_inc_hist[i]);
		PHYDM_DBG(dm, DBG_DFS,
			  "\nfa_mask_th: %d, total_fa_in_hist: %d ",
			  fa_mask_th, total_fa_in_hist);
	}

	sum = 0;
	for (k = 0; k < 5; k++) {
		if (dfs->radar_det_mask_hist[k] == 1)
			sum++;
	}

	if (dfs->mask_hist_checked <= 5)
		dfs->mask_hist_checked++;

	if (dfs->mask_hist_checked >= 5 && dfs->pulse_flag_hist[index]) {
		if (sum <= 2) {
			if (dfs->hist_cond_on) {
				/*return the value from hist_radar_detected*/
				radar_detected = phydm_dfs_hist_log(dm, index);
			} else {
				if (dfs->pulse_type_hist[index] == 0)
					dfs->radar_type = 0;
				else if (dfs->pulse_type_hist[index] == 1)
					dfs->radar_type = 1;
				radar_detected = 1;
				PHYDM_DBG(dm, DBG_DFS,
					  "Detected type %d radar signal!\n",
					  dfs->radar_type);
			}
		} else {
			fault_flag_det = 1;
			if (dfs->det_print2) {
				PHYDM_DBG(dm, DBG_DFS,
					  "Radar is masked : mask_hist large than thd\n");
			}
		}
	}

	dfs->mask_idx++;
	if (dfs->mask_idx == 5)
		dfs->mask_idx = 0;

	if (fault_flag_det == 0 && fault_flag_psd == 0 && fa_flag == 0) {
		if (dfs->igi_cur < 0x30) {
			st_l2h_new = dfs->st_l2h_min;
		}
	}

	if (st_l2h_new != dfs->st_l2h_cur) {
		if (st_l2h_new < dfs->st_l2h_min) {
			dfs->st_l2h_cur = dfs->st_l2h_min;
		} else if (st_l2h_new > dfs->st_l2h_max)
			dfs->st_l2h_cur = dfs->st_l2h_max;
		else
			dfs->st_l2h_cur = st_l2h_new;
		/*odm_set_bb_reg(dm, R_0x91c, 0xff, dfs->st_l2h_cur);*/

		dfs->pwdb_th_cur = ((int)dfs->st_l2h_cur - (int)dfs->igi_cur)
				    / 2 + dfs->pwdb_scalar_factor;

		/*@limit the pwdb value to absolute lower bound 8*/
		dfs->pwdb_th_cur = MAX_2(dfs->pwdb_th_cur, (int)dfs->pwdb_th);

		/*@limit the pwdb value to absolute upper bound 0x1f*/
		dfs->pwdb_th_cur = MIN_2(dfs->pwdb_th_cur, 0x1f);

		odm_set_bb_reg(dm, R_0xa50, 0x000000f0,
			       dfs->pwdb_th_cur);
	}

	if (dfs->det_print) {
		PHYDM_DBG(dm, DBG_DFS,
			  "fault_flag_det[%d], fault_flag_psd[%d], DFS_detected [%d]\n",
			  fault_flag_det, fault_flag_psd, radar_detected);
	}

	return radar_detected;
}

boolean phydm_dfs_hist_log(void *dm_void, u8 index)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _DFS_STATISTICS *dfs = &dm->dfs;
	u8 i = 0, j = 0;
	boolean hist_radar_detected = 0;

	if (dfs->pulse_type_hist[index] == 0) {
		dfs->radar_type = 0;
		if (dfs->pw_flag && dfs->pri_flag &&
		    dfs->pri_type3_4_flag) {
			hist_radar_detected = 1;
			PHYDM_DBG(dm, DBG_DFS,
				  "Detected type %d radar signal!\n",
				  dfs->radar_type);
			if (dfs->det_print2) {
				PHYDM_DBG(dm, DBG_DFS,
					  "hist_idx= %d\n",
					  (dfs->hist_idx + 3) % 4);
				for (j = 0; j < 4; j++) {
				for (i = 0; i < 6; i++) {
					PHYDM_DBG(dm, DBG_DFS,
						  "pri_hold = %d ",
						  dfs->pri_hold[j][i]);
				}
				PHYDM_DBG(dm, DBG_DFS, "\n");
				}
				PHYDM_DBG(dm, DBG_DFS, "\n");
				for (j = 0; j < 4; j++) {
				for (i = 0; i < 6; i++) {
					PHYDM_DBG(dm, DBG_DFS, "pw_hold = %d ",
						  dfs->pw_hold[j][i]);
				}
					PHYDM_DBG(dm, DBG_DFS, "\n");
				}
				PHYDM_DBG(dm, DBG_DFS, "\n");
				PHYDM_DBG(dm, DBG_DFS, "idle_mode = %d\n",
					  dfs->idle_mode);
				PHYDM_DBG(dm, DBG_DFS,
					  "pw_hold_sum = %d %d %d %d %d %d\n",
					  dfs->pw_hold_sum[0],
					  dfs->pw_hold_sum[1],
					  dfs->pw_hold_sum[2],
					  dfs->pw_hold_sum[3],
					  dfs->pw_hold_sum[4],
					  dfs->pw_hold_sum[5]);
				PHYDM_DBG(dm, DBG_DFS,
					  "pri_hold_sum = %d %d %d %d %d %d\n",
					  dfs->pri_hold_sum[0],
					  dfs->pri_hold_sum[1],
					  dfs->pri_hold_sum[2],
					  dfs->pri_hold_sum[3],
					  dfs->pri_hold_sum[4],
					  dfs->pri_hold_sum[5]);
			}
		} else {
		if (dfs->det_print2) {
			if (dfs->pulse_flag_hist[index] &&
			    dfs->pri_flag == 0) {
				PHYDM_DBG(dm, DBG_DFS, "pri_variation = %d\n",
					  dfs->pri_std);
				PHYDM_DBG(dm, DBG_DFS,
					  "PRI criterion is not satisfied!\n");
				if (dfs->pri_cond1 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "pri_cond1 is not satisfied!\n");
				if (dfs->pri_cond2 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "pri_cond2 is not satisfied!\n");
				if (dfs->pri_cond3 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "pri_cond3 is not satisfied!\n");
				if (dfs->pri_cond4 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "pri_cond4 is not satisfied!\n");
				if (dfs->pri_cond5 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "pri_cond5 is not satisfied!\n");
			}
			if (dfs->pulse_flag_hist[index] &&
			    dfs->pw_flag == 0) {
				PHYDM_DBG(dm, DBG_DFS, "pw_variation = %d\n",
					  dfs->pw_std);
				PHYDM_DBG(dm, DBG_DFS,
					  "PW criterion is not satisfied!\n");
				if (dfs->pw_cond1 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "pw_cond1 is not satisfied!\n");
				if (dfs->pw_cond2 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "pw_cond2 is not satisfied!\n");
				if (dfs->pw_cond3 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "pw_cond3 is not satisfied!\n");
			}
			if (dfs->pulse_flag_hist[index] &&
			    (dfs->pri_type3_4_flag == 0)) {
				PHYDM_DBG(dm, DBG_DFS,
					  "pri_type3_4 criterion is not satisfied!\n");
				if (dfs->pri_type3_4_cond1 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "pri_type3_4_cond1 is not satisfied!\n");
				if (dfs->pri_type3_4_cond2 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "pri_type3_4_cond2 is not satisfied!\n");
			}
			PHYDM_DBG(dm, DBG_DFS, "hist_idx= %d\n",
				  (dfs->hist_idx + 3) % 4);
			for (j = 0; j < 4; j++) {
				for (i = 0; i < 6; i++) {
					PHYDM_DBG(dm, DBG_DFS,
						  "pri_hold = %d ",
						  dfs->pri_hold[j][i]);
				}
				PHYDM_DBG(dm, DBG_DFS, "\n");
			}
			PHYDM_DBG(dm, DBG_DFS, "\n");
			for (j = 0; j < 4; j++) {
				for (i = 0; i < 6; i++)
					PHYDM_DBG(dm, DBG_DFS,
						  "pw_hold = %d ",
						  dfs->pw_hold[j][i]);
				PHYDM_DBG(dm, DBG_DFS, "\n");
			}
			PHYDM_DBG(dm, DBG_DFS, "\n");
			PHYDM_DBG(dm, DBG_DFS, "idle_mode = %d\n",
				  dfs->idle_mode);
			PHYDM_DBG(dm, DBG_DFS,
				  "pw_hold_sum = %d %d %d %d %d %d\n",
				  dfs->pw_hold_sum[0], dfs->pw_hold_sum[1],
				  dfs->pw_hold_sum[2], dfs->pw_hold_sum[3],
				  dfs->pw_hold_sum[4], dfs->pw_hold_sum[5]);
			PHYDM_DBG(dm, DBG_DFS,
				  "pri_hold_sum = %d %d %d %d %d %d\n",
				  dfs->pri_hold_sum[0], dfs->pri_hold_sum[1],
				  dfs->pri_hold_sum[2], dfs->pri_hold_sum[3],
				  dfs->pri_hold_sum[4], dfs->pri_hold_sum[5]);
		}
		}
	} else {
		dfs->radar_type = 1;
		if (dfs->det_print2) {
			PHYDM_DBG(dm, DBG_DFS, "\n");
			PHYDM_DBG(dm, DBG_DFS, "idle_mode = %d\n",
				  dfs->idle_mode);
		}
		/* @Long radar should satisfy three conditions */
		if (dfs->long_radar_flag == 1) {
			hist_radar_detected = 1;
			PHYDM_DBG(dm, DBG_DFS,
				  "Detected type %d radar signal!\n",
				  dfs->radar_type);
		} else {
			if (dfs->det_print2) {
				if (dfs->pw_long_cond1 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "--pw_long_cond1 is not satisfied!--\n");
				if (dfs->pw_long_cond2 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "--pw_long_cond2 is not satisfied!--\n");
				if (dfs->pri_long_cond1 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "--pri_long_cond1 is not satisfied!--\n");
			}
		}
	}
	return hist_radar_detected;
}

boolean phydm_radar_detect(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _DFS_STATISTICS *dfs = &dm->dfs;
	boolean radar_detected = false;

	dfs->igi_cur = (u8)odm_get_bb_reg(dm, R_0x1d70, 0x0000007f);
	dfs->st_l2h_cur = (u8)odm_get_bb_reg(dm, R_0xa40, 0x00007f00);

	/* @dynamic pwdb calibration */
	if (dfs->igi_pre != dfs->igi_cur) {
		dfs->pwdb_th_cur = ((int)dfs->st_l2h_cur - (int)dfs->igi_cur)
				    / 2 + dfs->pwdb_scalar_factor;

		/* @limit the pwdb value to absolute lower bound 0xa */
		dfs->pwdb_th_cur = MAX_2(dfs->pwdb_th_cur, (int)dfs->pwdb_th);
		/* @limit the pwdb value to absolute upper bound 0x1f */
		dfs->pwdb_th_cur = MIN_2(dfs->pwdb_th_cur, 0x1f);

		odm_set_bb_reg(dm, R_0xa50, 0x000000f0,
			       dfs->pwdb_th_cur);
	}
	dfs->igi_pre = dfs->igi_cur;

	phydm_dfs_dynamic_setting(dm);
	radar_detected = phydm_radar_detect_dm_check(dm);

	if (radar_detected) {
		PHYDM_DBG(dm, DBG_DFS,
			  "Radar detect: %d\n", radar_detected);
		phydm_radar_detect_reset(dm);
		if (dfs->dbg_mode == 1) {
			PHYDM_DBG(dm, DBG_DFS,
				  "Radar is detected in DFS dbg mode.\n");
			radar_detected = 0;
		}
	}

	if (dfs->sw_trigger_mode) {
		radar_detected = 1;
		PHYDM_DBG(dm, DBG_DFS,
			  "Radar is detected in DFS SW trigger mode.\n");
	}

	return radar_detected;
}

void phydm_dfs_hist_dbg(void *dm_void, char input[][16], u32 *_used,
			char *output, u32 *_out_len)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _DFS_STATISTICS *dfs = &dm->dfs;
	char help[] = "-h";
	u32 argv[5] = {0};
	u32 used = *_used;
	u32 out_len = *_out_len;
	u8 i;

	if ((strcmp(input[1], help) == 0)) {
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{0} pri_hist_th = %d\n", dfs->pri_hist_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{1} pri_sum_g1_th = %d\n", dfs->pri_sum_g1_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{2} pri_sum_g5_th = %d\n", dfs->pri_sum_g5_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{3} pri_sum_g1_fcc_th = %d\n",
			 dfs->pri_sum_g1_fcc_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{4} pri_sum_g3_fcc_th = %d\n",
			 dfs->pri_sum_g3_fcc_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{5} pri_sum_safe_fcc_th = %d\n",
			 dfs->pri_sum_safe_fcc_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{6} pri_sum_type4_th = %d\n", dfs->pri_sum_type4_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{7} pri_sum_type6_th = %d\n", dfs->pri_sum_type6_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{8} pri_sum_safe_th = %d\n", dfs->pri_sum_safe_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{9} pri_sum_g5_under_g1_th = %d\n",
			 dfs->pri_sum_g5_under_g1_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{10} pri_pw_diff_th = %d\n", dfs->pri_pw_diff_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{11} pri_pw_diff_fcc_th = %d\n",
			 dfs->pri_pw_diff_fcc_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{12} pri_pw_diff_fcc_idle_th = %d\n",
			 dfs->pri_pw_diff_fcc_idle_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{13} pri_pw_diff_w53_th = %d\n",
			 dfs->pri_pw_diff_w53_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{14} pri_type1_low_fcc_th = %d\n",
			 dfs->pri_type1_low_fcc_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{15} pri_type1_upp_fcc_th = %d\n",
			 dfs->pri_type1_upp_fcc_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{16} pri_type1_cen_fcc_th = %d\n",
			 dfs->pri_type1_cen_fcc_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{17} pw_g0_th = %d\n", dfs->pw_g0_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{18} pw_long_lower_20m_th = %d\n",
			 dfs->pw_long_lower_20m_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{19} pw_long_lower_th = %d\n",
			 dfs->pw_long_lower_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{20} pri_long_upper_th = %d\n",
			 dfs->pri_long_upper_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{21} pw_long_sum_upper_th = %d\n",
			 dfs->pw_long_sum_upper_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{22} pw_std_th = %d\n", dfs->pw_std_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{23} pw_std_idle_th = %d\n", dfs->pw_std_idle_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{24} pri_std_th = %d\n", dfs->pri_std_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{25} pri_std_idle_th = %d\n", dfs->pri_std_idle_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{26} type4_pw_max_cnt = %d\n", dfs->type4_pw_max_cnt);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{27} type4_safe_pri_sum_th = %d\n",
			 dfs->type4_safe_pri_sum_th);
	} else {
		PHYDM_SSCANF(input[1], DCMD_DECIMAL, &argv[0]);

		for (i = 1; i < 5; i++) {
			PHYDM_SSCANF(input[i + 1], DCMD_DECIMAL,
				     &argv[i]);
		}
		if (argv[0] == 0) {
			dfs->pri_hist_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_hist_th = %d\n",
				 dfs->pri_hist_th);
		} else if (argv[0] == 1) {
			dfs->pri_sum_g1_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_sum_g1_th = %d\n",
				 dfs->pri_sum_g1_th);
		} else if (argv[0] == 2) {
			dfs->pri_sum_g5_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_sum_g5_th = %d\n",
				 dfs->pri_sum_g5_th);
		} else if (argv[0] == 3) {
			dfs->pri_sum_g1_fcc_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_sum_g1_fcc_th = %d\n",
				 dfs->pri_sum_g1_fcc_th);
		} else if (argv[0] == 4) {
			dfs->pri_sum_g3_fcc_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_sum_g3_fcc_th = %d\n",
				 dfs->pri_sum_g3_fcc_th);
		} else if (argv[0] == 5) {
			dfs->pri_sum_safe_fcc_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_sum_safe_fcc_th = %d\n",
				 dfs->pri_sum_safe_fcc_th);
		} else if (argv[0] == 6) {
			dfs->pri_sum_type4_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_sum_type4_th = %d\n",
				 dfs->pri_sum_type4_th);
		} else if (argv[0] == 7) {
			dfs->pri_sum_type6_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_sum_type6_th = %d\n",
				 dfs->pri_sum_type6_th);
		} else if (argv[0] == 8) {
			dfs->pri_sum_safe_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_sum_safe_th = %d\n",
				 dfs->pri_sum_safe_th);
		} else if (argv[0] == 9) {
			dfs->pri_sum_g5_under_g1_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_sum_g5_under_g1_th = %d\n",
				 dfs->pri_sum_g5_under_g1_th);
		} else if (argv[0] == 10) {
			dfs->pri_pw_diff_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_pw_diff_th = %d\n",
				 dfs->pri_pw_diff_th);
		} else if (argv[0] == 11) {
			dfs->pri_pw_diff_fcc_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_pw_diff_fcc_th = %d\n",
				 dfs->pri_pw_diff_fcc_th);
		} else if (argv[0] == 12) {
			dfs->pri_pw_diff_fcc_idle_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_pw_diff_fcc_idle_th = %d\n",
				 dfs->pri_pw_diff_fcc_idle_th);
		} else if (argv[0] == 13) {
			dfs->pri_pw_diff_w53_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_pw_diff_w53_th = %d\n",
				 dfs->pri_pw_diff_w53_th);
		} else if (argv[0] == 14) {
			dfs->pri_type1_low_fcc_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_type1_low_fcc_th = %d\n",
				 dfs->pri_type1_low_fcc_th);
		} else if (argv[0] == 15) {
			dfs->pri_type1_upp_fcc_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_type1_upp_fcc_th = %d\n",
				 dfs->pri_type1_upp_fcc_th);
		} else if (argv[0] == 16) {
			dfs->pri_type1_cen_fcc_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_type1_cen_fcc_th = %d\n",
				 dfs->pri_type1_cen_fcc_th);
		} else if (argv[0] == 17) {
			dfs->pw_g0_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pw_g0_th = %d\n",
				 dfs->pw_g0_th);
		} else if (argv[0] == 18) {
			dfs->pw_long_lower_20m_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pw_long_lower_20m_th = %d\n",
				 dfs->pw_long_lower_20m_th);
		} else if (argv[0] == 19) {
			dfs->pw_long_lower_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pw_long_lower_th = %d\n",
				 dfs->pw_long_lower_th);
		} else if (argv[0] == 20) {
			dfs->pri_long_upper_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_long_upper_th = %d\n",
				 dfs->pri_long_upper_th);
		} else if (argv[0] == 21) {
			dfs->pw_long_sum_upper_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pw_long_sum_upper_th = %d\n",
				 dfs->pw_long_sum_upper_th);
		} else if (argv[0] == 22) {
			dfs->pw_std_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pw_std_th = %d\n",
				 dfs->pw_std_th);
		} else if (argv[0] == 23) {
			dfs->pw_std_idle_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pw_std_idle_th = %d\n",
				 dfs->pw_std_idle_th);
		} else if (argv[0] == 24) {
			dfs->pri_std_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_std_th = %d\n",
				 dfs->pri_std_th);
		} else if (argv[0] == 25) {
			dfs->pri_std_idle_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_std_idle_th = %d\n",
				 dfs->pri_std_idle_th);
		} else if (argv[0] == 26) {
			dfs->type4_pw_max_cnt = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "type4_pw_max_cnt = %d\n",
				 dfs->type4_pw_max_cnt);
		} else if (argv[0] == 27) {
			dfs->type4_safe_pri_sum_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "type4_safe_pri_sum_th = %d\n",
				 dfs->type4_safe_pri_sum_th);
		}
	}
	*_used = used;
	*_out_len = out_len;
}

void phydm_dfs_debug(void *dm_void, char input[][16], u32 *_used,
		     char *output, u32 *_out_len)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _DFS_STATISTICS *dfs = &dm->dfs;
	u32 used = *_used;
	u32 out_len = *_out_len;
	u32 argv[10] = {0};
	u8 i, input_idx = 0;

	for (i = 0; i < 7; i++) {
		PHYDM_SSCANF(input[i + 1], DCMD_HEX, &argv[i]);
		input_idx++;
	}

	if (input_idx == 0)
		return;

	dfs->dbg_mode = (boolean)argv[0];
	dfs->sw_trigger_mode = (boolean)argv[1];
	dfs->force_TP_mode = (boolean)argv[2];
	dfs->det_print = (boolean)argv[3];
	dfs->det_print2 = (boolean)argv[4];
	dfs->print_hist_rpt = (boolean)argv[5];
	dfs->hist_cond_on = (boolean)argv[6];

	PDM_SNPF(out_len, used, output + used, out_len - used,
		 "dbg_mode: %d, sw_trigger_mode: %d, force_TP_mode: %d, det_print: %d,det_print2: %d, print_hist_rpt: %d, hist_cond_on: %d\n",
		 dfs->dbg_mode, dfs->sw_trigger_mode, dfs->force_TP_mode,
		 dfs->det_print, dfs->det_print2, dfs->print_hist_rpt,
		 dfs->hist_cond_on);
}

u8 phydm_dfs_polling_time(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _DFS_STATISTICS *dfs = &dm->dfs;

	dfs->dfs_polling_time = 100;

	return dfs->dfs_polling_time;
}

#endif /* @defined(CONFIG_PHYDM_DFS_MASTER) */

boolean
phydm_is_dfs_band(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	if (((*dm->channel >= 52) && (*dm->channel <= 64)) ||
	    ((*dm->channel >= 100) && (*dm->channel <= 144)))
		return true;
	else
		return false;
}

boolean
phydm_dfs_master_enabled(void *dm_void)
{
#ifdef CONFIG_PHYDM_DFS_MASTER
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	boolean ret_val = false;

	if (dm->dfs_master_enabled) /*pointer protection*/
		ret_val = *dm->dfs_master_enabled ? true : false;

	return ret_val;
#else
	return false;
#endif
}

#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
#ifdef PHYDM_IC_JGR3_SERIES_SUPPORT
void phydm_dfs_ap_reset_radar_detect_counter_and_flag(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	/* @Clear Radar Counter and Radar flag */
	odm_set_bb_reg(dm, R_0xa40, BIT(15), 0);
	odm_set_bb_reg(dm, R_0xa40, BIT(15), 1);

	/* RT_TRACE(COMP_DFS, DBG_LOUD, ("[DFS], After reset radar counter, 0xcf8 = 0x%x, 0xcf4 = 0x%x\n", */
	/* PHY_QueryBBReg(Adapter, 0xcf8, bMaskDWord), */
	/* PHY_QueryBBReg(Adapter, 0xcf4, bMaskDWord))); */
}
#endif
#endif
