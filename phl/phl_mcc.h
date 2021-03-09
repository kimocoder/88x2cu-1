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
#ifndef _PHL_MCC_H_
#define _PHL_MCC_H_
/* MCC definition for private usage */

#define phl_to_com_mcc_info(_phl) ((struct phl_com_mcc_info *)(phl_to_mr_ctrl(_phl)->com_mcc))
#define get_mcc_info(_phl, _band) ((struct phl_mcc_info *)((get_band_ctrl(_phl, _band)->mcc_info)))
#define set_mcc_init_state(_phl, _state) (((struct mr_ctl_t *)phl_to_mr_ctrl(_phl))->init_mcc = _state)
#define is_mcc_init(_phl) (((struct mr_ctl_t *)phl_to_mr_ctrl(_phl))->init_mcc == true)
#define get_ref_role(_en_info) ((struct rtw_phl_mcc_role *)&(_en_info->mcc_role[_en_info->ref_role_idx]))

#define EARLY_TX_BCN_T 10
#define EARLY_RX_BCN_T 5
#define MIN_RX_BCN_T 10
#define MIN_GO_STA_OFFSET_T 15
#define MIN_CLIENT_DUR (EARLY_RX_BCN_T + MIN_RX_BCN_T)
#define MIN_AP_DUR (EARLY_TX_BCN_T + MIN_GO_STA_OFFSET_T - EARLY_RX_BCN_T)
#define MIN_BCNS_OFFSET (EARLY_RX_BCN_T + MIN_RX_BCN_T)
#define MIN_MCC_GROUP_ROLE 2
#define MAX_MCC_GROUP_ROLE 2
#define DEFAULT_AP_DUR 60
#define DEFAULT_CLIENT_DUR 40
#define MCC_DUR_NONSPECIFIC 0xff
#define CLIENTS_WORSECASE_REF_TOA 30
#define CLIENTS_WORSECASE_SMALL_DUR 60
#define CLIENTS_WORSECASE_LARGE_DUR 90
#define WORSECASE_INTVL 150
#define MIN_TRIGGER_MCC_TIME 300/*TU*/
#define CLIENTS_TRACKING_TH 3
#define CLIENTS_TRACKING_WORSECASE_TH 3
#define CLIENTS_TRACKING_CRITICAL_POINT_TH 2
#define HANDLE_BCN_INTVL 100
#define BT_DUR_SEG_TH 20
#define AP_CLIENT_OFFSET 40
#define REF_ROLE_IDX 0

enum _mcc_role_cat {
	MCC_ROLE_NONE = 0,
	MCC_ROLE_AP_CAT,
	MCC_ROLE_CLIENT_CAT
};

enum phl_mcc_state {
	MCC_NONE = 0,
	MCC_TRIGGER_FW_EN,
	MCC_FW_EN_FAIL,
	MCC_RUNING,
	MCC_TRIGGER_FW_DIS,
	MCC_FW_DIS_FAIL,
	MCC_STOP
};

struct phl_mcc_info {
	struct rtw_phl_mcc_en_info en_info;
	enum rtw_phl_mcc_mode mcc_mode;
	enum phl_mcc_state state;
	enum rtw_phl_mcc_coex_mode coex_mode;
	struct rtw_phl_mcc_bt_info bt_info;
};



#endif /*_PHL_MCC_H_*/
