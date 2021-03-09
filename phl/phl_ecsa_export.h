/******************************************************************************
 *
 * Copyright(c) 2020 Realtek Corporation.
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
#ifndef _PHL_ECSA_EXPORT_H_
#define _PHL_ECSA_EXPORT_H_

#define CHANNEL_SWITCH_MODE_NORMAL 0
#define CHANNEL_SWITCH_MODE_STOP_TX 1

enum phl_ecsa_param_flag{
	ECSA_PARAM_FLAG_APPEND_BCN = BIT0,
	ECSA_PARAM_FLAG_APPEND_PROBERSP = BIT1,
};

enum phl_ecsa_start_reason{
	ECSA_START_MCC_24G_TO_24G = 0,
	ECSA_START_MCC_5G_TO_5G = 1,
	ECSA_START_MCC_24G_TO_5G = 2,
	ECSA_START_MCC_5G_TO_24G = 3,
	ECSA_START_CORE_TRIGGER = 4,
	ECSA_START_UNKNOWN
};

struct rtw_phl_ecsa_param{
	u8 flag;
	u8 mode; /* CHANNEL_SWITCH_MODE_NORMAL or CHANNEL_SWITCH_MODE_STOP_TX */
	u8 op_class;
	u8 ch;
	u8 count;
	u32 delay_start_ms;
	struct rtw_chan_def new_chan_def;
};

/*
 * priv: ops private, define by core layer
 * update_beacon: Notify core to construct and update beacon
 * update_chan_info: Notify core to update AP mode channel information
 * check_ecsa_allow: Provide reason and ask core if ecsa is allowed or not,
 * 	core can provide a delay time(ms) to delay start ECSA
 */
struct rtw_phl_ecsa_ops{
	void *priv;
	void (*update_beacon)(void *priv, struct rtw_wifi_role_t *role);
	void (*update_chan_info)(void *priv,
				 struct rtw_wifi_role_t *role,
				 struct rtw_chan_def chan_def);
	bool (*check_ecsa_allow)(void *priv,
				 struct rtw_wifi_role_t *role,
				 struct rtw_chan_def chan_def,
				 enum phl_ecsa_start_reason reason,
				 u32 *delay_start_ms);
};

#ifdef CONFIG_PHL_ECSA
enum rtw_phl_status
rtw_phl_ecsa_init_ops(
	void *phl,
	struct rtw_phl_ecsa_ops *ops
	);

enum rtw_phl_status
rtw_phl_ecsa_cmd_request(
	void *phl,
	struct rtw_wifi_role_t *role
	);

enum rtw_phl_status
rtw_phl_ecsa_start(
	void *phl,
	struct rtw_wifi_role_t *role,
	struct rtw_phl_ecsa_param *param
	);

enum rtw_phl_status
rtw_phl_ecsa_cancel(
	void *phl,
	struct rtw_wifi_role_t *role
	);

enum rtw_phl_status
rtw_phl_ecsa_get_param(
	void *phl,
	struct rtw_phl_ecsa_param **param
	);

bool
rtw_phl_ecsa_check_allow(
	void *phl,
	struct rtw_wifi_role_t *role,
	struct rtw_chan_def chan_def,
	enum phl_ecsa_start_reason reason,
	u32 *delay_start_ms
	);
#else
#define rtw_phl_ecsa_init_ops(_phl, _ops) RTW_PHL_STATUS_SUCCESS
#define rtw_phl_ecsa_cmd_request(_phl, _role) RTW_PHL_STATUS_SUCCESS
#define rtw_phl_ecsa_start(_phl, _role, _param) RTW_PHL_STATUS_SUCCESS
#define rtw_phl_ecsa_cancel(_phl, _role) RTW_PHL_STATUS_SUCCESS
#define rtw_phl_ecsa_get_param(_phl, _param) RTW_PHL_STATUS_SUCCESS
#define rtw_phl_ecsa_check_allow(_phl, _role, _chan_def, _reason, _delay_start_ms) false
#endif

#endif /*_PHL_ECSA_EXPORT_H_*/
