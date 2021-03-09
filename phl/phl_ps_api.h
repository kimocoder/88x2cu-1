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
#ifndef __PHL_PS_API_H__
#define __PHL_PS_API_H__

#define PS_TEST 0

enum PHL_PS_NOTIFY_TYPE {

	PHL_PS_NTFY_TYPE_WATCHDOG = 1,
	PHL_PS_NTFY_TYPE_BATTERY_CHG,
	PHL_PS_NTFY_TYPE_CREATE_MAC,
	PHL_PS_NTFY_TYPE_DELETE_MAC,
	PHL_PS_NTFY_TYPE_CHANGE_PORT,
	PHL_PS_NTFY_TYPE_CONNECT,
	PHL_PS_NTFY_TYPE_DISASSOCIATE,
	PHL_PS_NTFY_TYPE_START_AP,
	PHL_PS_NTFY_TYPE_DRIVER_UNLOAD,
	PHL_PS_NTFY_TYPE_WOWLAN,
	PHL_PS_NTFY_TYPE_SCAN,
	PHL_PS_NTFY_TYPE_CMD_IO,
	PHL_PS_NTFY_TYPE_ROLE,
	PHL_PS_NTFY_TYPE_PS_MODULE,
	PHL_PS_NTFY_TYPE_LPS_MODULE,
	PHL_PS_NTFY_TYPE_SIMPLE_WATCHDOG,
	PHL_PS_NTFY_TYPE_MAX
};

enum PHL_PS_NOTIFY {
	PHL_PS_NTFY_WATCHDOG_START,
	PHL_PS_NTFY_WATCHDOG_STOP,
	PHL_PS_NTFY_BATTERY_CHG,
	PHL_PS_NTFY_CREATE_MAC_START,
	PHL_PS_NTFY_CREATE_MAC_END,
	PHL_PS_NTFY_DELETE_MAC_START,
	PHL_PS_NTFY_DELETE_MAC_END,
	PHL_PS_NTFY_CHANGE_PORT_START,
	PHL_PS_NTFY_CHANGE_PORT_END,
	PHL_PS_NTFY_CONNECT_START,
	PHL_PS_NTFY_CONNECT_STOP,
	PHL_PS_NTFY_DISASSOCIATE_START,
	PHL_PS_NTFY_DISASSOCIATE_STOP,
	PHL_PS_NTFY_START_AP_START,
	PHL_PS_NTFY_START_AP_END,
	PHL_PS_NTFY_DRIVER_UNLOAD,
	PHL_PS_NTFY_SIMPLE_WATCHDOG_START,
	PHL_PS_NTFY_SIMPLE_WATCHDOG_END,
	PHL_PS_NTFY_MAX
};

struct ps_role_info_param {
	u8 role_id;
	u16 macid;
	enum role_state rstate;
	enum role_type rtype;
};

struct lps_role_info_param {
	u16 macid;
	u32 cap;
	u8 awake_interval;
	u8 listen_bcn_mode;
	u8 smart_ps_mode;
	u8 (*chk_lps_state_chg_cb)(void *phl, u16 macid, u8 state);
};

struct ps_ntfy {
	_os_list list;
	bool free_ntfy;
	u32 token;

	/* real parameter for different notifications */
	union {
		struct ps_role_info_param ps_role;
		u32 pm_comp;
		u32 pm_cap;
		enum rtw_rf_state rf_state;
		struct lps_role_info_param lps_role;
	} u;

	bool wait_done;
	_os_event *done;
	void *ctx;
	void (*cb)(void *phl, void *hdl, void *ctx, enum rtw_phl_status stat);

	char type_s[50];
};

#ifdef CONFIG_PS

#define phl_ps_leave_cmd_sync(_phl, _token, type)	\
	phl_ps_issue_cmd(_phl, _token, NULL, NULL, true, type)
#define phl_ps_leave_cmd_async(_phl, _token, _cb, _ctx, type)	\
	phl_ps_issue_cmd(_phl, _token, _cb, _ctx, false, type)
#define phl_ps_leave_cmd_cancel(_phl, _token)	\
	phl_ps_cancel_cmd(_phl, _token)

void
rtw_phl_ps_type_convert_to_string(enum PHL_PS_NOTIFY_TYPE type, char *buf);

enum rtw_phl_status
rtw_phl_ps_send_battery_chg_hub_msg(void *phl, bool ips_allow, bool lps_allow);
enum rtw_phl_status
rtw_phl_ps_notify(void *phl, enum PHL_PS_NOTIFY notify, void *param);
enum rtw_phl_status
phl_ps_role_notify(void *phl, u8 *buf);
void phl_ps_ser_notify(void *phl, bool ser_start);
#ifdef RTW_WKARD_LPS_P2P_ROLE_TYPE
enum rtw_phl_status
rtw_phl_ps_role_type_notify(void *phl, struct rtw_wifi_role_t *wrole,
	enum role_type rtype);
#endif
enum rtw_phl_status
rtw_phl_ps_is_valid_access(void *phl, u32 offset);
enum rtw_phl_status
phl_ps_issue_cmd(void *phl, u32 *token,
	void (*cb)(void *phl, void *hdl, void *ctx, enum rtw_phl_status stat),
	void *ctx, u8 wait, enum PHL_PS_NOTIFY_TYPE type);
enum rtw_phl_status
phl_ps_cancel_cmd(void *phl, u32 token);
u8 rtw_phl_ps_reject_roaming(struct phl_info_t *phl_info);
enum rtw_phl_status
phl_ps_wow_link_ptcl_cfg(struct phl_info_t *phl_info, bool en, u16 macid);
enum rtw_phl_status
phl_ps_wow_link_pwr_cfg(struct phl_info_t *phl_info);
enum rtw_phl_status
phl_ps_force_hw_leave_low_pwr(struct phl_info_t *phl_info);
void phl_ps_watchdog(struct phl_info_t *phl_info);

#else
#define rtw_phl_ps_type_convert_to_string(type, buf)
#define rtw_phl_ps_send_battery_chg_hub_msg(_phl, _ips_auto, _lps_auto)
#define rtw_phl_ps_notify(_phl, _notify, _param) RTW_PHL_STATUS_SUCCESS
#define phl_ps_role_notify(_phl, _buf)
#define phl_ps_ser_notify(_phl, _ser_start)
#ifdef RTW_WKARD_LPS_P2P_ROLE_TYPE
#define rtw_phl_ps_role_type_notify(_phl, _wrole, _rtype) RTW_PHL_STATUS_SUCCESS
#endif
#define rtw_phl_ps_is_valid_access(_phl, _offset) RTW_PHL_STATUS_SUCCESS
#define phl_ps_leave_all_cmd_sync(_phl, _token)
#define phl_ps_leave_cmd_sync(_phl, _token, _task_type) RTW_PHL_STATUS_SUCCESS
#define phl_ps_leave_cmd_async(_phl, _token, _cb, _ctx)	\
	RTW_PHL_STATUS_SUCCESS
#define phl_ps_leave_cmd_cancel(_phl, _token)
#define rtw_phl_ps_reject_roaming(phl_info) false
#define phl_ps_wow_link_ptcl_cfg(_phl_info, _en, _macid) RTW_PHL_STATUS_SUCCESS
#define phl_ps_wow_link_pwr_cfg(_phl_info) RTW_PHL_STATUS_SUCCESS
#define phl_ps_force_hw_leave_low_pwr(_phl_info) RTW_PHL_STATUS_SUCCESS
#define phl_ps_watchdog(_phl_info)

#endif /*CONFIG_PS */

#ifdef CONFIG_PS_PM

#define phl_ps_req_pwr_sync(_phl, _token, _comp, _test, _type)	\
	phl_ps_issue_pwr_cmd(_phl, _token, _comp, NULL, NULL, _test, true, _type)
#define phl_ps_req_pwr_async(_phl, _token, _comp, _cb, _ctx, _test, _type)	\
	phl_ps_issue_pwr_cmd(_phl, _token, _comp, _cb, _ctx, _test, false, _type)
#define phl_ps_cancel_pwr_req(_phl, _token)	\
	phl_ps_cancel_pwr_cmd(_phl, _token)

enum rtw_phl_status
phl_ps_issue_pwr_cmd(void *phl, u32 *token, u32 comp,
	void (*cb)(void *phl, void *hdl, void *ctx, enum rtw_phl_status stat),
	void *ctx, u8 chk_rtn, u8 wait, enum PHL_PS_NOTIFY_TYPE type);
enum rtw_phl_status
phl_ps_cancel_pwr_cmd(void *phl, u32 token);
enum rtw_phl_status
phl_ps_set_radio_state(void *phl, enum rtw_rf_state rf_state);

#if PS_TEST
enum rtw_phl_status
phl_ps_issue_pwr_cap(void *phl, u32 *token, u32 cap);
enum rtw_phl_status
phl_ps_cancel_pwr_cap(void *phl, u32 token);
#endif /* PS_TEST */

#else
#define phl_ps_req_pwr_sync(_phl, _token, _comp, _test, type) RTW_PHL_STATUS_SUCCESS
#define phl_ps_req_pwr_async(_phl, _token, _comp, _cb, _ctx, _test, type)	\
	RTW_PHL_STATUS_SUCCESS
#define phl_ps_cancel_pwr_req(_phl, _token) RTW_PHL_STATUS_SUCCESS
#define phl_ps_set_radio_state(_phl, _rf_state) RTW_PHL_STATUS_SUCCESS

#endif /* CONFIG_PS_PM */

#ifdef CONFIG_PS_IPS

#define rtw_phl_ps_leave_ips_sync(_phl, _token, _type)	\
	rtw_phl_ps_issue_ips_cmd(_phl, _token, NULL, NULL, true, _type)
#define rtw_phl_ps_leave_ips_async(_phl, _token, _cb, _ctx, _type)	\
	rtw_phl_ps_issue_ips_cmd(_phl, _token, _cb, _ctx, false, _type)
#define rtw_phl_ps_leave_ips_cancel(_phl, _token)	\
	rtw_phl_ps_cancel_ips_cmd(_phl, _token)

enum rtw_phl_status
rtw_phl_ps_issue_ips_cmd(void *phl, u32 *token,
	void (*cb)(void *phl, void *hdl, void *ctx, enum rtw_phl_status stat),
	void *ctx, u8 wait, enum PHL_PS_NOTIFY_TYPE type);
enum rtw_phl_status
rtw_phl_ps_cancel_ips_cmd(void *phl, u32 token);

#else
#define rtw_phl_ps_leave_ips_sync(_phl, _token, _type)
#define rtw_phl_ps_leave_ips_async(_phl, _token, _cb, _ctx, _type)	\
	RTW_PHL_STATUS_SUCCESS
#define rtw_phl_ps_leave_ips_cancel(_phl, _token)

#endif /* CONFIG_PS_IPS */

#ifdef CONFIG_PS_LPS

#define rtw_phl_ps_leave_lps_sync(_phl, _token, _type)	\
	rtw_phl_ps_issue_lps_cmd(_phl, _token, NULL, NULL, true, _type)
#define rtw_phl_ps_leave_lps_async(_phl, _token, _cb, _ctx, _type)	\
	rtw_phl_ps_issue_lps_cmd(_phl, _token, _cb, _ctx, false, _type)
#define rtw_phl_ps_leave_lps_cancel(_phl, _token)	\
	rtw_phl_ps_cancel_lps_cmd(_phl, _token)

enum rtw_phl_status
rtw_phl_ps_issue_lps_cmd(void *phl, u32 *token,
	void (*cb)(void *phl, void *hdl, void *ctx, enum rtw_phl_status stat),
	void *ctx, u8 wait, enum PHL_PS_NOTIFY_TYPE type);
enum rtw_phl_status
rtw_phl_ps_cancel_lps_cmd(void *phl, u32 token);

#else
#define rtw_phl_ps_leave_lps_sync(_phl, _token, _type)
#define rtw_phl_ps_leave_lps_async(_phl, _token, _cb, _ctx, _type)	\
	RTW_PHL_STATUS_SUCCESS
#define rtw_phl_ps_leave_lps_cancel(_phl, _token)

#endif /* CONFIG_PS_LPS */

#endif /* __PHL_PS_API_H__ */

