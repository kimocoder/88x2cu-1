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

#ifdef CONFIG_CMD_DISP
enum rtw_phl_status
phl_register_mrc_module(struct phl_info_t *phl_info);
#endif

struct rtw_wifi_role_t *
phl_get_wrole_by_ridx(struct phl_info_t *phl_info, u8 rold_idx);

struct rtw_wifi_role_t *
phl_get_wrole_by_addr(struct phl_info_t *phl_info, u8 *mac_addr);

enum rtw_phl_status
phl_role_notify(struct phl_info_t *phl_info, struct rtw_wifi_role_t *wrole);

enum rtw_phl_status
phl_role_recover(struct phl_info_t *phl_info);
enum rtw_phl_status
phl_cmd_role_recover(struct phl_info_t *phl_info);
enum rtw_phl_status
phl_role_suspend(struct phl_info_t *phl_info);
enum rtw_phl_status
phl_cmd_role_suspend(struct phl_info_t *phl_info);

#ifdef RTW_WKARD_LPS_ROLE_CONFIG
void phl_role_recover_unused_role(struct phl_info_t *phl_info,
	struct rtw_wifi_role_t *cur_wrole);
void phl_role_suspend_unused_role(struct phl_info_t *phl_info,
	struct rtw_wifi_role_t *cur_wrole);
#endif

#ifdef RTW_PHL_BCN
enum rtw_phl_status
rtw_phl_free_bcn_entry(void *phl, struct rtw_wifi_role_t *wrole);
#ifdef CONFIG_CMD_DISP
enum rtw_phl_status
phl_cmd_issue_bcn_hdl(struct phl_info_t *phl_info, u8 *param);
enum rtw_phl_status
phl_cmd_stop_bcn_hdl(struct phl_info_t *phl_info, u8 *param);
#endif
#endif

#ifdef CONFIG_CMD_DISP
enum rtw_phl_status
phl_wifi_role_start_hdl(struct phl_info_t *phl_info, u8 *param);
enum rtw_phl_status
phl_wifi_role_chg_hdl(struct phl_info_t *phl_info, u8 *param);
enum rtw_phl_status
phl_wifi_role_stop_hdl(struct phl_info_t *phl_info, u8 *param);
#endif

enum rtw_phl_status
phl_wifi_role_macid_all_pause(struct phl_info_t *phl_info, struct rtw_wifi_role_t *wrole, bool pause);
#endif  /*_PHL_ROLE_H_*/
