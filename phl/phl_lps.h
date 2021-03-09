/******************************************************************************
 *
 * Copyright(c) 2019 - 2020 Realtek Corporation.
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
#ifndef __PHL_LPS_H__
#define __PHL_LPS_H__

enum phl_lps_role_state {
	LPS_ROLE_ST_IDLE,
	LPS_ROLE_ST_LPS,
};

enum phl_lps_listern_bcn_mode {
	LPS_RLBM_MIN         = 0,
	LPS_RLBM_MAX         = 1,
	LPS_RLBM_USERDEFINE  = 2,
};

enum phl_lps_smart_ps_mode {
	LPS_LEGACY_PWR1      = 0,
	LPS_TRX_PWR0         = 1,
};

#ifdef CONFIG_PS_LPS

/* legacy power save fsm init api */
enum rtw_phl_status
phl_lps_init(void **lps_obj, struct phl_info_t *phl_info);
void phl_lps_deinit(void *lps_obj);
void phl_lps_periodic_chk(void *lps_obj);
u8 phl_lps_is_ongoing(void *lps_obj);
enum rtw_phl_status
phl_lps_issue_cmd(void *lps_obj, struct ps_ntfy *ntfy);
void phl_lps_cancel_cmd(void *lps_obj, struct ps_ntfy *ntfy);
enum rtw_phl_status
phl_lps_add_role(void *lps_obj, struct ps_ntfy *ntfy);
enum rtw_phl_status
phl_lps_del_role(void *lps_obj, struct ps_ntfy *ntfy);
enum rtw_phl_status
phl_lps_wow_cfg(void *lps_obj, bool lps_en, u16 macid);
void phl_lps_dbg_dump_obj(void *lps_obj, u32 *used, char input[][MAX_ARGV],
			u32 input_num, char *output, u32 out_len);
void phl_lps_watchdog(void *lps_obj);

#else
#define phl_lps_init(_lps_obj, _phl_info) RTW_PHL_STATUS_SUCCESS
#define phl_lps_deinit(_lps_obj) do {} while (0)
#define phl_lps_periodic_chk(_lps_obj)
#define phl_lps_is_ongoing(_lps_obj) false
#define phl_lps_issue_cmd(_lps_obj, _ntfy) RTW_PHL_STATUS_SUCCESS
#define phl_lps_cancel_cmd(_lps_obj, _ntfy)
#define phl_lps_add_role(_lps_obj, _ntfy) RTW_PHL_STATUS_SUCCESS
#define phl_lps_del_role(_lps_obj, _ntfy) RTW_PHL_STATUS_SUCCESS
#define phl_lps_wow_cfg(_lps_obj, _lps_en, _macid) RTW_PHL_STATUS_SUCCESS
#define phl_lps_dbg_dump_obj(_lps_obj, _used, _input,	\
				_input_num, _output, _out_len)
#define phl_lps_watchdog(_lps_obj)
#endif /* CONFIG_PS_LPS */

#endif /* __PHL_LPS_H__ */

