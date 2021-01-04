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
#ifndef _PHL_STA_H_
#define _PHL_STA_H_

/*********** macid ctrl section ***********/
enum rtw_phl_status
phl_macid_ctrl_init(struct phl_info_t *phl);

enum rtw_phl_status
phl_macid_ctrl_deinit(struct phl_info_t *phl);

u16
rtw_phl_get_macid_max_num(void *phl);

u16 
rtw_phl_wrole_bcmc_id_get(void *phl, struct rtw_wifi_role_t *wrole);

u8 
rtw_phl_macid_is_bmc(void *phl, u16 macid);

u8 
rtw_phl_macid_is_used(void *phl, u16 macid);

static inline bool
phl_macid_is_valid(struct phl_info_t *phl_info, u16 macid)
{
	return (macid < phl_info->macid_ctrl.max_num) ? true : false;
}

/*********** stainfo_ctrl section ***********/
enum rtw_phl_status
phl_stainfo_ctrl_init(struct phl_info_t *phl_info);

enum rtw_phl_status
phl_stainfo_ctrl_deinie(struct phl_info_t *phl_info);

enum rtw_phl_status
phl_alloc_stainfo_hw(struct phl_info_t *phl_info, struct rtw_phl_stainfo_t *sta);

enum rtw_phl_status
phl_free_stainfo_hw(struct phl_info_t *phl_info, struct rtw_phl_stainfo_t *sta);

#ifdef DBG_PHL_STAINFO
void phl_dump_stactrl(const char *caller, const int line, bool show_caller,
						struct phl_info_t *phl_info);
#define PHL_DUMP_STACTRL(_phl_info) phl_dump_stactrl(__FUNCTION__, __LINE__, false, _phl_info);
#define PHL_DUMP_STACTRL_EX(_phl_info) phl_dump_stactrl(__FUNCTION__, __LINE__, true, _phl_info);

void phl_dump_stainfo_all(const char *caller, const int line, bool show_caller,
				struct phl_info_t *phl_info);
#define PHL_DUMP_STAINFO(_phl_info) phl_dump_stainfo_all(__FUNCTION__, __LINE__, false, _phl_info);
#define PHL_DUMP_STAINFO_EX(_phl_info) phl_dump_stainfo_all(__FUNCTION__, __LINE__, true, _phl_info);

void phl_dump_stainfo_per_role(const char *caller, const int line, bool show_caller,
				struct phl_info_t *phl_info, struct rtw_wifi_role_t *wrole);
#define PHL_DUMP_ROLE_STAINFO(_phl_info, wrole) phl_dump_stainfo_per_role(__FUNCTION__, __LINE__, false, _phl_info, wrole);
#define PHL_DUMP_ROLE_STAINFO_EX(_phl_info, wrole) phl_dump_stainfo_per_role(__FUNCTION__, __LINE__, true, _phl_info, wrole);
#else
#define PHL_DUMP_STACTRL(_phl_info)
#define PHL_DUMP_STACTRL_EX(_phl_info)

#define PHL_DUMP_STAINFO(_phl_info)
#define PHL_DUMP_STAINFO_EX(_phl_info)

#define PHL_DUMP_ROLE_STAINFO(_phl_info, wrole)
#define PHL_DUMP_ROLE_STAINFO_EX(_phl_info, wrole)
#endif
/*********** phl stainfo section ***********/
struct rtw_phl_stainfo_t *
rtw_phl_alloc_stainfo(void *phl, u8 *sta_addr,
			   struct rtw_wifi_role_t *wrole);

enum rtw_phl_status
rtw_phl_free_stainfo(void *phl, struct rtw_phl_stainfo_t *sta);

enum rtw_phl_status
rtw_phl_free_stainfo_hw(void *phl, struct rtw_phl_stainfo_t *sta);


enum rtw_phl_status
phl_wifi_role_free_stainfo(struct phl_info_t *phl_info,
					struct rtw_wifi_role_t *role);

enum rtw_phl_status
rtw_phl_update_media_status(void *phl, struct rtw_phl_stainfo_t *sta,
			u8 *sta_addr, bool is_connect);

struct rtw_phl_stainfo_t *
rtw_phl_get_stainfo_by_macid(void *phl, u16 macid);

struct rtw_phl_stainfo_t *
rtw_phl_get_stainfo_by_addr(void *phl, struct rtw_wifi_role_t *wrole, u8 *addr);


struct rtw_phl_stainfo_t *
rtw_phl_get_stainfo_self(void *phl, struct rtw_wifi_role_t *wrole);

void
rtw_phl_stainfo_link_notify(void *phl, struct rtw_wifi_role_t *wrole, bool add, u16 macid);

enum rtw_phl_status
rtw_phl_alloc_stainfo_hw(void *phl, struct rtw_phl_stainfo_t *sta);

enum rtw_phl_status
phl_change_stainfo(struct phl_info_t *phl_info, struct rtw_phl_stainfo_t *sta,
				enum phl_upd_mode mode);

#endif	/*_PHL_STA_H_*/

