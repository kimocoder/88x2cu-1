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
#ifndef _PHL_ROLE_H_
#define _PHL_ROLE_H_

enum rtw_phl_status
phl_register_mrc_module(struct phl_info_t *phl_info);

struct rtw_wifi_role_t *
phl_get_wrole_by_ridx(struct phl_info_t *phl_info, u8 rold_idx);

struct rtw_wifi_role_t *
phl_get_wrole_by_addr(struct phl_info_t *phl_info, u8 *mac_addr);

enum rtw_phl_status
phl_role_notify(struct phl_info_t *phl_info, struct rtw_wifi_role_t *wrole);

#ifdef RTW_WKARD_RADIO_IPS_FLOW
enum rtw_phl_status
phl_role_recover(struct phl_info_t *phl_info);

enum rtw_phl_status
phl_role_suspend(struct phl_info_t *phl_info);
#endif

#ifdef RTW_WKARD_LPS_ROLE_CONFIG
void phl_role_recover_unused_role(struct phl_info_t *phl_info,
	struct rtw_wifi_role_t *cur_wrole);
void phl_role_suspend_unused_role(struct phl_info_t *phl_info,
	struct rtw_wifi_role_t *cur_wrole);
#endif

#ifdef RTW_PHL_BCN
enum rtw_phl_status
rtw_phl_free_bcn_entry(void *phl, struct rtw_wifi_role_t *wrole);
#endif

#endif  /*_PHL_ROLE_H_*/
