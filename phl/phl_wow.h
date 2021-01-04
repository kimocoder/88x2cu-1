/******************************************************************************
 *
 * Copyright(c)2019 Realtek Corporation.
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
#ifndef _PHL_WOW_H_
#define _PHL_WOW_H_

#define WOW_HUBMSG_MAXLEN 50

#define PHL_WOW_ERR_DL_FW BIT0
#define PHL_WOW_ERR_PKT_OFLD BIT1
#define PHL_WOW_ERR_FUNC_EN BIT2
#define PHL_WOW_ERR_FUNC_DIS BIT3
#define PHL_WOW_ERR_MAC BIT4
#define PHL_WOW_ERR_TRX BIT5
#define PHL_WOW_ERR_HW BIT6

#define phl_to_wow_info(_phl) ((struct phl_wow_info *)(_phl->phl_com->wow_info))
#define get_wow_pairwise_algo_type(_wow_info) (_wow_info->wow_wake_info.pairwise_sec_algo)
#define get_wow_group_algo_type(_wow_info) (_wow_info->wow_wake_info.group_sec_algo)

struct phl_wow_error {
	u16 init;
	u16 deinit;
};

struct phl_wow_stat {
	/* init */
	u8 enter_wow;
	u8 func_en;
	u8 keep_alive_en;
	u8 disc_det_en;
	u8 arp_en;
	u8 ndp_en;
	u8 gtk_en;
	/* deinit */
	enum rtw_wow_wake_reason wake_rsn;
	u8 mac_pwr;
	/* common */
	struct phl_wow_error err;
};

struct phl_wow_info {
	/* common */
	struct phl_info_t *phl_info;
	_os_lock wow_lock;
	u8 wow_msg[WOW_HUBMSG_MAXLEN];
	struct phl_wow_stat wow_stat;

	/* general info, should reset */
	u8 enter_wow;
	u8 func_en;
	struct phl_wow_error err;
	struct rtw_phl_stainfo_t *sta;
	u8 pwr_saving;
	u8 mac_pwr;

	/* pkt ofld token */
	u32 null_pkt_token;
	u32 arp_pkt_token;
	u32 ndp_pkt_token;
	u32 eapol_key_pkt_token;
	u32 sa_query_pkt_token;

	/* func */
	struct rtw_keep_alive_info keep_alive_info;
	struct rtw_disc_det_info disc_det_info;
	struct rtw_nlo_info nlo_info;
	struct rtw_arp_ofld_info arp_ofld_info;
	struct rtw_ndp_ofld_info ndp_ofld_info;
	struct rtw_gtk_ofld_info gtk_ofld_info;
	struct rtw_realwow_info realwow_info;
	struct rtw_wow_wake_info wow_wake_info;
	struct rtw_pattern_match_info pattern_match_info;

	/* info to core */
	enum rtw_wow_wake_reason wake_rsn;
	struct rtw_aoac_report aoac_info;
};

enum rtw_phl_status phl_wow_mdl_init(struct phl_info_t* phl_info);
void phl_wow_mdl_deinit(struct phl_info_t* phl_info);

#ifdef CONFIG_WOWLAN

void phl_record_wow_stat(struct phl_info_t *phl_info);

enum rtw_phl_status phl_wow_init_precfg(struct phl_info_t *phl_info);

enum rtw_phl_status phl_wow_init_postcfg(struct phl_info_t *phl_info);

enum rtw_phl_status phl_wow_deinit_precfg(struct phl_info_t *phl_info);

enum rtw_phl_status phl_wow_deinit_postcfg(struct phl_info_t *phl_info);

void phl_reset_wow_info(struct phl_info_t *phl_info);

u8 phl_wow_nlo_exist(struct phl_info_t *phl_info);

enum rtw_phl_status phl_wow_func_en(struct phl_info_t *phl_info, struct rtw_phl_stainfo_t *sta);

enum rtw_phl_status phl_wow_func_dis(struct phl_info_t *phl_info, struct rtw_phl_stainfo_t *sta);

void phl_wow_pwr_cfg(struct phl_info_t *phl_info, struct rtw_phl_stainfo_t *sta, u8 enter_wow);

#endif /* CONFIG_WOWLAN */

#endif /* _PHL_WOW_H_ */
