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
#ifndef _PHL_LED_DEF_H_
#define _PHL_LED_DEF_H_

#define PHL_RADIO_ON_OFF_NOT_READY

enum rtw_led_ctrl_mode {
	RTW_LED_CTRL_NOT_SUPPORT,
	RTW_LED_CTRL_HW_TRX_MODE,
	RTW_LED_CTRL_HW_TX_MODE,
	RTW_LED_CTRL_HW_RX_MODE,
	RTW_LED_CTRL_SW_PP_MODE,
	RTW_LED_CTRL_SW_OD_MODE,
};

enum rtw_led_id { RTW_LED_ID_0, RTW_LED_ID_1, RTW_LED_ID_LENGTH };

/*
 * led_event here is not integrated with msg_evt_id due to the following reason:
 *
 * (a) led_event is used for mapping LED events with LED actions, and
 *     the mapping can be configured in core layer according to the
 *     customized LED table.
 *
 * (b) LED module inside uses led_event as the index of led action
 *     arrays, and hence it would be inappropriate to directly replace
 *     led_event with msg_evt_id which is not continuous and does not
 *     start from zero.
 */
enum rtw_led_event {
	RTW_LED_EVENT_PHL_START,
	RTW_LED_EVENT_PHL_STOP,
	RTW_LED_EVENT_SW_RF_ON,
	RTW_LED_EVENT_SW_RF_OFF,
	RTW_LED_EVENT_SCAN,
	RTW_LED_EVENT_TX,
	RTW_LED_EVENT_RX,
	RTW_LED_EVENT_LENGTH
};

enum rtw_led_state {
	RTW_LED_STATE_IGNORE = BIT0,
	RTW_LED_STATE_SW_RF_ON = BIT1,
};

enum rtw_led_action {
	RTW_LED_ACTION_LOW,
	RTW_LED_ACTION_HIGH,
	RTW_LED_ACTION_TOGGLE,
};

#endif /*_PHL_LED_DEF_H_*/
