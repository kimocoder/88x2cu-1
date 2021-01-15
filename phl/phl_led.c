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
#define _PHL_LED_C_
#include "phl_headers.h"

#define PHL_LED_LOW 0
#define PHL_LED_HIGH 1

#define PHL_LED_EVENT_STATE_ARGS_ARR_LEN_MAX 4
#define PHL_LED_INTERVALS_LEN_MAX 4

struct phl_led_event_state_args_t {
	enum rtw_led_state state_condition;
	enum rtw_led_action led_action_arr[RTW_LED_ID_LENGTH];
	u32 intervals_arr[RTW_LED_ID_LENGTH][PHL_LED_INTERVALS_LEN_MAX];
	u8 intervals_len_arr[RTW_LED_ID_LENGTH];
};

struct phl_led_event_args_t {
	struct phl_led_event_state_args_t
	    event_state_args_arr[PHL_LED_EVENT_STATE_ARGS_ARR_LEN_MAX];
	u8 event_state_args_arr_len;
};

struct phl_led_led_ctrl_t {
	enum rtw_led_ctrl_mode ctrl_mode_arr[RTW_LED_ID_LENGTH];
	struct phl_led_event_args_t event_args_arr[RTW_LED_EVENT_LENGTH];
	enum rtw_led_state state;
};


enum rtw_phl_status phl_led_ctrl_init(struct phl_info_t *phl_info)
{
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_led_led_ctrl_t *led_ctrl =
	    _os_mem_alloc(drv_priv, sizeof(struct phl_led_led_ctrl_t));

	enum rtw_led_id led_id;
	enum rtw_led_event event_id;

	PHL_TRACE(COMP_PHL_LED, _PHL_INFO_, "===> phl_led_ctrl_init()\n");

	if (led_ctrl == NULL) {
		phl_info->led_ctrl = NULL;
		return RTW_PHL_STATUS_FAILURE;
	}

	phl_info->led_ctrl = led_ctrl;

	/* set default value */
	led_ctrl->state = 0;

	for (led_id = 0; led_id < RTW_LED_ID_LENGTH; led_id++) {
		led_ctrl->ctrl_mode_arr[led_id] = RTW_LED_CTRL_NOT_SUPPORT;
	}

	for (led_id = 0; led_id < RTW_LED_ID_LENGTH; led_id++)
		for (event_id = 0; event_id < RTW_LED_EVENT_LENGTH;
		     event_id++) {
			led_ctrl->event_args_arr[event_id]
			    .event_state_args_arr_len = 0;
		}

	return RTW_PHL_STATUS_SUCCESS;
}

void phl_led_ctrl_deinit(struct phl_info_t *phl_info)
{
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_led_led_ctrl_t *led_ctrl =
	    (struct phl_led_led_ctrl_t *)(phl_info->led_ctrl);

	PHL_TRACE(COMP_PHL_LED, _PHL_INFO_, "===> phl_led_ctrl_deinit()\n");

	if (led_ctrl == NULL)
		return;

	_os_mem_free(drv_priv, led_ctrl, sizeof(struct phl_led_led_ctrl_t));

	phl_info->led_ctrl = NULL;
}

#if 0 // NEO

static enum rtw_phl_status
_phl_led_ctrl_action_handler(void *hal, enum rtw_led_id led_id,
			     enum rtw_led_action action, u32 *intervals,
			     u8 intervals_len)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;

	PHL_TRACE(
	    COMP_PHL_LED, _PHL_INFO_,
	    "_phl_led_ctrl_action_handler(): led_id == %d, action == 0X%X\n",
	    led_id, action);

	switch (action) {
	case RTW_LED_ACTION_LOW:
		if (rtw_hal_led_control(hal, led_id, PHL_LED_LOW) !=
		    RTW_HAL_STATUS_SUCCESS)
			status = RTW_PHL_STATUS_FAILURE;
		break;

	case RTW_LED_ACTION_HIGH:
		if (rtw_hal_led_control(hal, led_id, PHL_LED_HIGH) !=
		    RTW_HAL_STATUS_SUCCESS)
			status = RTW_PHL_STATUS_FAILURE;
		break;

	case RTW_LED_ACTION_TOGGLE:
		break;

	default:
		break;
	}

	return status;
}

static enum rtw_phl_status
_phl_led_ctrl_event_handler(void *hal, struct phl_led_led_ctrl_t *led_ctrl,
			    enum rtw_led_event event)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	struct phl_led_event_args_t *event_args =
	    &(led_ctrl->event_args_arr[event]);

	struct phl_led_event_state_args_t *event_state_args;
	u8 args_idx;
	enum rtw_led_id led_id;

	PHL_TRACE(COMP_PHL_LED, _PHL_INFO_,
		  "_phl_led_ctrl_event_handler() : event == 0X%X\n", event);

	/* set state */
	switch (event) {
	case RTW_LED_EVENT_SW_RF_ON:
		PHL_TRACE(
		    COMP_PHL_LED, _PHL_INFO_,
		    "_phl_led_ctrl_event_handler() : set state sw rf on\n");
		led_ctrl->state |= RTW_LED_STATE_SW_RF_ON;
		break;

	case RTW_LED_EVENT_SW_RF_OFF:
		PHL_TRACE(
		    COMP_PHL_LED, _PHL_INFO_,
		    "_phl_led_ctrl_event_handler() : set state sw rf off\n");
		led_ctrl->state &= ~RTW_LED_STATE_SW_RF_ON;
		break;

	default:
		break;
	}

	/* handle event */
	for (args_idx = 0; args_idx < event_args->event_state_args_arr_len;
	     args_idx++) {
		event_state_args =
		    &(event_args->event_state_args_arr[args_idx]);

		if (!(event_state_args->state_condition &
		      (led_ctrl->state | RTW_LED_STATE_IGNORE)))
			continue;

		for (led_id = 0; led_id < RTW_LED_ID_LENGTH; led_id++) {
			if (led_ctrl->ctrl_mode_arr[led_id] !=
				RTW_LED_CTRL_SW_PP_MODE &&
			    led_ctrl->ctrl_mode_arr[led_id] !=
				RTW_LED_CTRL_SW_OD_MODE)
				continue;

			if (RTW_PHL_STATUS_SUCCESS !=
			    _phl_led_ctrl_action_handler(
				hal, led_id,
				event_state_args->led_action_arr[led_id],
				event_state_args->intervals_arr[led_id],
				event_state_args->intervals_len_arr[led_id]))
				status = RTW_PHL_STATUS_FAILURE;
		}
	}

	return status;
}

static enum phl_mdl_ret_code _phl_led_module_init(void *phl, void *dispr,
						  void **priv)
{
	*priv = phl;
	return MDL_RET_SUCCESS;
}

static void _phl_led_module_deinit(void *dispr, void *priv) {}

static enum phl_mdl_ret_code _phl_led_module_start(void *dispr, void *priv)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)priv;
	struct phl_led_led_ctrl_t *led_ctrl =
	    (struct phl_led_led_ctrl_t *)(phl_info->led_ctrl);

	enum phl_mdl_ret_code ret = MDL_RET_SUCCESS;
	enum rtw_led_id led_id = RTW_LED_ID_0;

	PHL_TRACE(COMP_PHL_LED, _PHL_INFO_, "===> _phl_led_module_start()\n");

	if (led_ctrl == NULL) {
		PHL_TRACE(COMP_PHL_LED, _PHL_INFO_,
			  "_phl_led_module_start() : led_ctrl == NULL\n");
		return MDL_RET_FAIL;
	}

	while (led_id < RTW_LED_ID_LENGTH) {

		if (RTW_HAL_STATUS_SUCCESS !=
		    rtw_hal_led_set_ctrl_mode(phl_info->hal, led_id,
					      led_ctrl->ctrl_mode_arr[led_id]))
			ret = MDL_RET_FAIL;

		led_id++;
	}

	if (RTW_PHL_STATUS_SUCCESS !=
	    _phl_led_ctrl_event_handler(phl_info->hal, led_ctrl,
					RTW_LED_EVENT_PHL_START))
		ret = MDL_RET_FAIL;

	if (RTW_PHL_STATUS_SUCCESS !=
	    _phl_led_ctrl_event_handler(phl_info->hal, led_ctrl,
					RTW_LED_EVENT_SW_RF_ON))
		ret = MDL_RET_FAIL;

	return ret;
}

static enum phl_mdl_ret_code _phl_led_module_stop(void *dispr, void *priv)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)priv;
	struct phl_led_led_ctrl_t *led_ctrl =
	    (struct phl_led_led_ctrl_t *)(phl_info->led_ctrl);

	enum phl_mdl_ret_code ret = MDL_RET_SUCCESS;

	PHL_TRACE(COMP_PHL_LED, _PHL_INFO_, "===> _phl_led_module_stop()\n");

	if (led_ctrl == NULL) {
		PHL_TRACE(COMP_PHL_LED, _PHL_INFO_,
			  "_phl_led_module_stop() : led_ctrl == NULL\n");
		return MDL_RET_FAIL;
	}

	if (RTW_PHL_STATUS_SUCCESS !=
	    _phl_led_ctrl_event_handler(phl_info->hal, led_ctrl,
					RTW_LED_EVENT_SW_RF_OFF))
		ret = MDL_RET_FAIL;

	if (RTW_PHL_STATUS_SUCCESS !=
	    _phl_led_ctrl_event_handler(phl_info->hal, led_ctrl,
					RTW_LED_EVENT_PHL_STOP))
		ret = MDL_RET_FAIL;

	return ret;
}

static enum phl_mdl_ret_code _phl_led_module_msg_hdlr(void *dispr, void *priv,
						      struct phl_msg *msg)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)priv;
	struct phl_led_led_ctrl_t *led_ctrl =
	    (struct phl_led_led_ctrl_t *)(phl_info->led_ctrl);
	enum rtw_led_event event = MSG_EVT_ID_FIELD(msg->msg_id);

	PHL_TRACE(COMP_PHL_LED, _PHL_INFO_,
		  "===> _phl_led_module_msg_hdlr()\n");

	if (IS_MSG_IN_PRE_PHASE(msg->msg_id))
		return MDL_RET_SUCCESS;

	if (MSG_MDL_ID_FIELD(msg->msg_id) != PHL_MDL_LED)
		return MDL_RET_IGNORE;

	if (led_ctrl == NULL) {
		PHL_TRACE(COMP_PHL_LED, _PHL_INFO_,
			  "_phl_led_module_msg_hdlr() : led_ctrl == NULL\n");
		return MDL_RET_FAIL;
	}

	if (RTW_PHL_STATUS_SUCCESS !=
	    _phl_led_ctrl_event_handler(phl_info->hal, led_ctrl, event))
		return MDL_RET_FAIL;

	return MDL_RET_SUCCESS;
}

static enum phl_mdl_ret_code
_phl_led_module_set_info(void *dispr, void *priv,
			 struct phl_module_op_info *info)
{
	return MDL_RET_SUCCESS;
}

static enum phl_mdl_ret_code
_phl_led_module_query_info(void *dispr, void *priv,
			   struct phl_module_op_info *info)
{
	return MDL_RET_SUCCESS;
}

enum rtw_phl_status phl_register_led_module(struct phl_info_t *phl_info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;

	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);

	struct phl_bk_module_ops bk_ops;
	bk_ops.init = _phl_led_module_init;
	bk_ops.deinit = _phl_led_module_deinit;
	bk_ops.start = _phl_led_module_start;
	bk_ops.stop = _phl_led_module_stop;
	bk_ops.msg_hdlr = _phl_led_module_msg_hdlr;
	bk_ops.set_info = _phl_led_module_set_info;
	bk_ops.query_info = _phl_led_module_query_info;

	phl_status = phl_disp_eng_register_module(phl_info, HW_BAND_0,
						  PHL_MDL_LED, &bk_ops);

	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s register LED module in cmd disp failed\n",
			__func__);
		phl_status = RTW_PHL_STATUS_FAILURE;
	}

	return phl_status;
}

void rtw_phl_led_set_ctrl_mode(void *phl, enum rtw_led_id led_id,
			       enum rtw_led_ctrl_mode ctrl_mode)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_led_led_ctrl_t *led_ctrl =
	    (struct phl_led_led_ctrl_t *)phl_info->led_ctrl;

	PHL_TRACE(COMP_PHL_LED, _PHL_INFO_,
		  "===> rtw_phl_led_set_ctrl_mode()\n");

	if (led_ctrl == NULL) {
		PHL_TRACE(COMP_PHL_LED, _PHL_INFO_,
			  "rtw_phl_led_set_ctrl_mode() : led_ctrl == NULL\n");
		return;
	}

	led_ctrl->ctrl_mode_arr[led_id] = ctrl_mode;
}

void rtw_phl_led_set_action(void *phl, enum rtw_led_event event,
			    enum rtw_led_state state_condition,
			    enum rtw_led_id led_id,
			    enum rtw_led_action led_action, u32 *intervals,
			    u8 intervals_len)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_led_led_ctrl_t *led_ctrl =
	    (struct phl_led_led_ctrl_t *)phl_info->led_ctrl;
	void *drv_priv = phl_to_drvpriv(phl_info);

	struct phl_led_event_args_t *event_args;
	struct phl_led_event_state_args_t *event_state_args;

	PHL_TRACE(COMP_PHL_LED, _PHL_INFO_, "===> rtw_phl_led_set_action()\n");

	if (led_ctrl == NULL) {
		PHL_TRACE(COMP_PHL_LED, _PHL_INFO_,
			  "rtw_phl_led_set_action() : led_ctrl == NULL\n");
		return;
	}

	event_args = &(led_ctrl->event_args_arr[event]);

	if (event_args->event_state_args_arr_len ==
	    PHL_LED_EVENT_STATE_ARGS_ARR_LEN_MAX) {
		PHL_TRACE(COMP_PHL_LED, _PHL_INFO_,
			  "rtw_phl_led_set_action() : event_state_args_arr_len "
			  "== PHL_LED_EVENT_STATE_ARGS_ARR_LEN_MAX\n");
		return;
	}

	if (intervals_len > PHL_LED_INTERVALS_LEN_MAX) {
		PHL_TRACE(COMP_PHL_LED, _PHL_INFO_,
			  "rtw_phl_led_set_action() : intervals_len > "
			  "PHL_LED_INTERVALS_LEN_MAX\n");
		return;
	}

	event_state_args =
	    &(event_args
		  ->event_state_args_arr[event_args->event_state_args_arr_len]);
	event_args->event_state_args_arr_len++;

	event_state_args->state_condition = state_condition;
	event_state_args->led_action_arr[led_id] = led_action;

	if (intervals_len > 0) {
		_os_mem_cpy(drv_priv, event_state_args->intervals_arr[led_id],
			    intervals, intervals_len * sizeof(u32));
	}
	event_state_args->intervals_len_arr[led_id] = intervals_len;
}

void phl_led_control(struct phl_info_t *phl_info, enum rtw_led_event led_event)
{
	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);
	struct phl_msg msg = {0};
	struct phl_msg_attribute attr = {0};

	PHL_TRACE(COMP_PHL_LED, _PHL_INFO_, "===> rtw_phl_led_control()\n");

	SET_MSG_MDL_ID_FIELD(msg.msg_id, PHL_MDL_LED);

	/*
	 * led_event here is passed via the msg_evt_id field instead of
	 * msg_evt_id due to the following reason:
	 *
	 * (a) led_event is used for mapping LED events with LED actions, and
	 *     the mapping can be configured in core layer according to the
	 *     customized LED table.
	 *
	 * (b) LED module inside uses led_event as the index of led action
	 *     arrays, and hence it would be inappropriate to directly replace
	 *     led_event with msg_evt_id which is not continuous and does not
	 *     start from zero.
	 *
	 * (c) It is not worth it to use inbuf with the overhead of dynamic
	 *     allocation and completion callback only for a number.
	 */
	SET_MSG_EVT_ID_FIELD(msg.msg_id, led_event);

	phl_dispr_send_msg(disp_eng->dispatcher[0], &msg, &attr, NULL);
}

void rtw_phl_led_control(void *phl, enum rtw_led_event led_event)
{
	phl_led_control((struct phl_info_t *)phl, led_event);
}

#endif // if 0
