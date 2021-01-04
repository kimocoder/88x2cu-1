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
#ifndef _PHL_MCC_DEF_H_
#define _PHL_MCC_DEF_H_
/* MCC definition for public usage */
#ifdef CONFIG_MCC_SUPPORT

#define TU 1024 /* Time Unit (TU): 1024 us*/

struct phl_com_mcc_info {
	struct rtw_phl_mcc_ops ops;
};

void rtw_phl_mcc_watchdog(struct phl_info_t *phl, u8 band_idx);

void rtw_phl_mcc_client_link_notify_for_ap(struct phl_info_t *phl,
					struct rtw_phl_stainfo_t *sta);

enum rtw_phl_status rtw_phl_mcc_duration_change(struct phl_info_t *phl,
				struct rtw_wifi_role_t *wrole, u8 duration);

enum rtw_phl_status rtw_phl_mcc_bt_duration_change(struct phl_info_t *phl,
						u8 dur, u8 band_idx);

bool rtw_phl_mcc_inprogress(struct phl_info_t *phl, u8 band_idx);

enum rtw_phl_status rtw_phl_mcc_enable(struct phl_info_t *phl,
					struct rtw_wifi_role_t *cur_role);

enum rtw_phl_status rtw_phl_mcc_disable(struct phl_info_t *phl,
					struct rtw_wifi_role_t *spec_role);

enum rtw_phl_status rtw_phl_mcc_init_ops(struct phl_info_t *phl, struct rtw_phl_mcc_ops *ops);

enum rtw_phl_status rtw_phl_mcc_init(struct phl_info_t *phl);

void rtw_phl_mcc_deinit(struct phl_info_t *phl);

#else /* CONFIG_MCC_SUPPORT ==0 */
#define rtw_phl_mcc_watchdog(_phl, _band_idx)
#define rtw_phl_mcc_client_link_notify_for_ap(_phl, _sta)
#define rtw_phl_mcc_enable(_phl,_cur_role) RTW_PHL_STATUS_FAILURE
#define rtw_phl_mcc_disable(_phl,_spec_role) RTW_PHL_STATUS_FAILURE
#define rtw_phl_mcc_init_ops(_phl, _ops) RTW_PHL_STATUS_FAILURE
#define rtw_phl_mcc_init(_phl) RTW_PHL_STATUS_FAILURE
#define rtw_phl_mcc_deinit(_phl)
#define rtw_phl_mcc_inprogress(_phl, _band_idx) false
#define rtw_phl_mcc_duration_change(_phl, _wrole, _duration) RTW_PHL_STATUS_FAILURE
#define rtw_phl_mcc_bt_duration_change(_phl, _dur, _band_idx) RTW_PHL_STATUS_FAILURE

#endif /* CONFIG_MCC_SUPPORT */
#endif /*_PHL_MCC_DEF_H_*/
