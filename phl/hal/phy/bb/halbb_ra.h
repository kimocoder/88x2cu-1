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
#ifndef _HALBB_RA_H_
#define _HALBB_RA_H_

/*@--------------------------[extern] ---------------------------------------*/
extern const u16 bb_phy_rate_table[LEGACY_RATE_NUM + HE_RATE_NUM_4SS];
/*@--------------------------[Define] ---------------------------------------*/
#define VHT_2_HE32_RATE(X) ((((X) << 3) + (X) + 4) >> 3) /*= Round(X * 1.125)*/
#define HE32_2_HE16_RATE(X) ((((X) << 3) + (X) + 4) >> 3) /*= Round(X * 1.125)*/
#define HE32_2_HE08_RATE(X) ((((X) << 4) + ((X) << 1) + (X) + 8) >> 4) /*= Round(X * 1.1875)*/

#define RAMASK_B	0x000000000000000f
#define RAMASK_AG	0x0000000000000ff0
#define RAMASK_BG	0x0000000000000ff5
#define RAMASK_HT_2G	0x00000ffffffff015
#define RAMASK_HT_5G	0x00000ffffffff010
#define RAMASK_VHT_2G	0x000ffffffffff015
#define RAMASK_VHT_5G	0x000ffffffffff010
#define RAMASK_HE_2G	0x0ffffffffffff015
#define RAMASK_HE_5G	0x0ffffffffffff010

#define RAMASK_1SS_HT	0x00000000000fffff
#define RAMASK_2SS_HT	0x00000000ff0fffff
#define RAMASK_3SS_HT	0x00000ff0ff0fffff
#define RAMASK_4SS_HT	0x00ff0ff0ff0fffff

#define RAMASK_1SS_VHT	0x00000000003fffff
#define RAMASK_2SS_VHT	0x00000003ff3fffff
#define RAMASK_3SS_VHT	0x00003ff3ff3fffff
#define RAMASK_4SS_VHT	0x03ff3ff3ff3fffff

#define RAMASK_1SS_HE	0x0000000000ffffff
#define RAMASK_2SS_HE	0x0000000fffffffff
#define RAMASK_3SS_HE	0x0000ffffffffffff
#define RAMASK_4SS_HE	0x0fffffffffffffff

#define MAX_NSS_VHT 4
#define MAX_NSS_HT 4
#define MAX_NSS_HE 4

#define STA_NUM_RSSI_CMD PHL_MAX_STA_NUM

#define MASKRATE_AX	0x01ff
#define MASKGILTF_AX	0x0e00

/* WiFi Support Mode */
#define CCK_SUPPORT 	BIT(0)
#define OFDM_SUPPORT	BIT(1)
#define HT_SUPPORT		BIT(2)
#define VHT_SUPPORT_TX	BIT(3)
#define HE_SUPPORT		BIT(4)

#define	RA_FLOOR_TABLE_SIZE	7
#define	RA_FLOOR_UP_GAP		3


/*@--------------------------[Enum]------------------------------------------*/


enum spatial_stream_num {
	RA_1SS_MODE	= 0,
	RA_2SS_MODE	= 1,
	RA_3SS_MODE	= 2,
	RA_4SS_MODE	= 3
};

enum wifi_mode {
	RA_CCK		= 0,
	RA_non_ht	= 1,
	RA_HT		= 2,
	RA_VHT		= 3,
	RA_HE		= 4
};

enum mu_cmd_type {
	MU_ADD_ENTRY	= 0,
	MU_DEL_ENTRY	= 1,
	MU_DBG_CTRL	=2,
};

/*@--------------------------[Structure]-------------------------------------*/

struct bb_rate_info {
	u16 rate_idx_all;
	u16 rate_idx;
	enum rtw_gi_ltf gi_ltf;
	enum bb_mode_type mode; /*0:legacy, 1:HT, 2*/
	enum channel_width bw;
	u8 ss;
	u8 idx;
};

struct bb_ra_info {
	/* Config move to phl_sta_info*/
	//NEO
	//struct bb_h2c_ra_cfg_info ra_cfg;
	u8 cal_giltf;
	/* Ctrl */
	u8 drv_ractrl;
	bool fixed_rate_en;
	u8 fixed_rate; /* 7bit rate */
	u8 fixed_rat_md; /* 2bit rate_mode */
	u8 fixed_giltf; /*  3bit giltf */
	u8 fixed_bw; /* 2bit bw */
	u8 rssi; /* should not put here */
	u8 rainfo_cfg1; /* prepare for other control*/
	u8 rainfo_cfg2; /* prepare for other control*/

	u8 rssi_lv;

	/* Report */
	u8 rpt_rate; /* 7bit rate + 2bit rat_md + 3bit giltf + 2bit bw */
	u8 rpt_rat_md;
	u8 rpt_giltf;
	u8 rpt_bw;
	u8 rpt_ratio;

	u8 tmp;
	
};

/*@--------------------------[Prptotype]-------------------------------------*/

struct bb_info;
bool halbb_is_cck_rate(struct bb_info *bb, u16 rate);
bool halbb_is_ofdm_rate(struct bb_info *bb, u16 rate);
bool halbb_is_ht_rate(struct bb_info *bb, u16 rate);
bool halbb_is_vht_rate(struct bb_info *bb, u16 rate);
bool halbb_is_he_rate(struct bb_info *bb, u16 rate);
u8 halbb_legacy_rate_2_spec_rate(struct bb_info *bb, u16 rate);
u8 halbb_rate_2_rate_digit(struct bb_info *bb, u16 rate);
u8 halbb_get_rx_stream_num(struct bb_info *bb, enum rf_type type);
u8 halbb_rate_type_2_num_ss(struct bb_info *bb, enum halbb_rate_type type);
u8 halbb_rate_to_num_ss(struct bb_info *bb, u16 rate);
void halbb_print_rate_2_buff(struct bb_info *bb, u16 rate_idx, enum rtw_gi_ltf gi_ltf, char *buf, u16 buf_size);
enum bb_qam_type halbb_get_qam_order(struct bb_info *bb, u16 rate_idx);
u8 halbb_rate_order_compute(struct bb_info *bb, u16 rate_idx);
void halbb_ra_watchdog(struct bb_info *bb);
bool halbb_raupdate_mask(struct bb_info *bb, struct rtw_phl_stainfo_t *phl_sta_i);

void halbb_ra_init(struct bb_info *bb);
void halbb_ra_dbg(struct bb_info *bb, char input[][16], u32 *_used,
			 char *output, u32 *_out_len);
void halbb_rate_idx_parsor(struct bb_info *bb, u16 rate_idx, enum rtw_gi_ltf gi_ltf, struct bb_rate_info *ra_i);
u32 halbb_get_fw_ra_rpt(struct bb_info *bb, u16 len, u8 *c2h);
u32 halbb_get_txsts_rpt(struct bb_info *bb, u16 len, u8 *c2h);

#endif
