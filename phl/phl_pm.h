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
#ifndef __PHL_PM_H__
#define __PHL_PM_H__

#ifdef CONFIG_PS_PM

#define PHL_PM_STR_BUF_LEN 100

#define case_pm_cap(src) \
	case HWPS_CAP_##src : return #src

#define case_pm_comp(src) \
	case PWRCMD_COMP_##src : return #src


enum {
	PM_STR_CAP = 0,
	PM_STR_COMP
};

enum pm_pwr_lvl {
	PM_PWR_LVL_PWROFF	= 0, /* driver hal deinit*/
	PM_PWR_LVL_PWR_GATED	= 1, /* FW control*/
	PM_PWR_LVL_CLK_GATED	= 2, /* FW control*/
	PM_PWR_LVL_RF_OFF	= 3, /* FW control*/
	PM_PWR_LVL_PWRON	= 4,
	PM_PWR_LVL_MAX,
};

enum rtw_phl_status
phl_pm_wait_pwrcmd_sync(void *pm_obj, void *d, u32 *token);

/* power manager fsm init api */
enum rtw_phl_status
phl_pm_init(void **pm_obj, struct phl_info_t *phl_info);
void phl_pm_deinit(void *pm_obj);
void phl_pm_start(void *pm_obj);
void phl_pm_stop(void *pm_obj);
void phl_pm_periodic_chk(void *pm_obj);
u8 phl_pm_is_low_power(void *pm_obj);
u8 phl_pm_test_pwr_level(void *pm_obj, u32 test_comp);
enum rtw_phl_status
phl_pm_issue_pwr_cmd(void *pm_obj, struct ps_ntfy *ntfy);
void phl_pm_cancel_pwr_cmd(void *pm_obj, struct ps_ntfy *ntfy);
enum rtw_phl_status
phl_pm_issue_pwr_cap(void *pm_obj, struct ps_ntfy *ntfy);
void phl_pm_cancel_pwr_cap(void *pm_obj, struct ps_ntfy *ntfy);
enum rtw_phl_status
phl_pm_set_radio_state(void *pm_obj, struct ps_ntfy *ntfy);
enum rtw_phl_status
phl_pm_wow_cfg(void *pm_obj, u8 wow_cap);
void phl_pm_force_power_on(void *pm_obj);
void phl_pm_dbg_dump_obj(void *pm_obj, u32 *used, char input[][MAX_ARGV],
			u32 input_num, char *output, u32 out_len);
void phl_pm_watchdog(void *pm_obj);

#else
#define phl_pm_init(_pm_obj, _phl_info) RTW_PHL_STATUS_SUCCESS
#define phl_pm_deinit(_pm_obj) do {} while (0)
#define phl_pm_start(_pm_obj)
#define phl_pm_stop(_pm_obj)
#define phl_pm_periodic_chk(_pm_obj)
#define phl_pm_is_low_power(_pm_obj) RTW_PHL_STATUS_SUCCESS
#define phl_pm_test_pwr_level(_pm_obj, _test_comp) RTW_PHL_STATUS_SUCCESS
#define phl_pm_issue_pwr_cmd(_pm_obj, _ntfy) RTW_PHL_STATUS_SUCCESS
#define phl_pm_cancel_pwr_cmd(_pm_obj, _ntfy)
#define phl_pm_issue_pwr_cap(_pm_obj, _ntfy) RTW_PHL_STATUS_SUCCESS
#define phl_pm_cancel_pwr_cap(_pm_obj, _ntfy)
#define phl_pm_set_radio_state(_pm_obj, _ntfy) RTW_PHL_STATUS_SUCCESS
#define phl_pm_wow_cfg(_pm_obj, _wow_cap) RTW_PHL_STATUS_SUCCESS
#define phl_pm_force_power_on(_pm_obj);
#define phl_pm_dbg_dump_obj(_pm_obj, _used, _input,	\
				_input_num, _output, _out_len);
#define phl_pm_watchdog(_pm_obj)
#endif /* CONFIG_PS_PM */

#endif /* __PHL_PM_H__ */

