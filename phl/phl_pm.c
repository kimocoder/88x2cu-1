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
#include "phl_headers.h"
#include "phl_ps_api.h"
#include "phl_pm.h"
#include "test/phl_ps_dbg_cmd.h"

#ifdef CONFIG_PS_PM

struct pm_obj {
	_os_mutex mux;
	struct phl_info_t *phl_info;

	bool time_arrived_en_pm;
	u32 start_time;
	u32 init_rf_time;

	struct list_head cmd_q;
	u32 cmd_cnt;
	struct list_head cap_q;
	u32 cap_cnt;
	u8 cur_lvl;

	enum rtw_rf_state rf_state;
};

struct pwr_cmd_entry {
	struct list_head list;
	u32 comp;
	u32 token;
	u8 enq : 1;
	u8 rsvd : 7;
	char type_s[50];
};

struct pwr_cap_entry {
	struct list_head list;
	u32 cap;
	u32 token;
	u8 enq : 1;
	u8 rsvd : 7;
};
static char*
_phl_pm_id_to_str(u8 type, u32 id)
{
	switch (type) {

	case PM_STR_CAP:

		switch (id) {
		case_pm_cap(PWROFF);
		case_pm_cap(PWR_GATED);
		case_pm_cap(CLK_GATED);
		case_pm_cap(RF_OFF);
		case_pm_cap(PWRON);
		default:
			return "UNKNOWN";
		};

	case PM_STR_COMP:

		switch (id) {
		case_pm_comp(IO_MAC);
		case_pm_comp(IO_BB);
		case_pm_comp(IO_RF);
		case_pm_comp(IO_AFE);
		case_pm_comp(IO_SIE);
		case_pm_comp(IO_EFUSE);
		case_pm_comp(IO_HCI);
		default:
			return "UNKNOWN";
		};

	};

	return "UNDEFINED";
}

static char*
_phl_pm_convert_caps_to_str(u32 cap, char *str)
{
	u32 pos = 0;
	u32 idx = BIT0;


	while (1) {

		if (HWPS_CAP_MAX == idx)
			break;

		if (cap & idx) {

			if (0 != pos) {
				_os_strcpy(str+pos, "|");
				pos = pos + _os_strlen((u8 *)"|");
			}

			_os_strcpy(str+pos, _phl_pm_id_to_str(PM_STR_CAP, idx));
			pos = pos + _os_strlen(
				(u8 *)_phl_pm_id_to_str(PM_STR_CAP, idx));

		}

		idx = idx << 1;

	}

	if (0 == pos)
		_os_strcpy(str, "NONE");

	return str;
}

static char*
_phl_pm_convert_comps_to_str(u32 comp, char *str)
{
	u32 pos = 0;
	u32 idx = BIT0;

	while (1) {

		if (PWRCMD_COMP_IO_MAX == idx)
			break;

		if (comp & idx) {

			if (0 != pos) {
				_os_strcpy(str+pos, "|");
				pos = pos + _os_strlen((u8 *)"|");
			}

			_os_strcpy(str+pos, _phl_pm_id_to_str(PM_STR_COMP, idx));
			pos = pos + _os_strlen(
				(u8 *)_phl_pm_id_to_str(PM_STR_COMP, idx));

		}

		idx = idx << 1;

	}

	if (0 == pos)
		_os_strcpy(str, "NONE");

	return str;
}


static void
_phl_pm_show_all_cmds(struct pm_obj *pm)
{
	struct pwr_cmd_entry *pos = NULL;
	void *d = phl_to_drvpriv(pm->phl_info);
	char str[PHL_PM_STR_BUF_LEN] = {0};

	_os_mutex_lock(d, &pm->mux);

	phl_list_for_loop(pos, struct pwr_cmd_entry, &pm->cmd_q, list) {


		_phl_pm_convert_comps_to_str(pos->comp, str);

		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
			"[PM] %s Cmd => token: %u type : %s comp: %x (%s)\n",
			__func__, pos->token, pos->type_s, pos->comp, str);
	}

	_os_mutex_unlock(d, &pm->mux);

}

static void
_phl_pm_show_all_caps(struct pm_obj *pm)
{
	struct pwr_cap_entry *pos = NULL;
	void *d = phl_to_drvpriv(pm->phl_info);
	char str[PHL_PM_STR_BUF_LEN] = {0};

	_os_mutex_lock(d, &pm->mux);

	phl_list_for_loop(pos, struct pwr_cap_entry, &pm->cap_q, list) {

		_phl_pm_convert_caps_to_str(pos->cap, str);

		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
			"[PM] %s cap => token: %u cap %x (%s) \n", __func__,
				pos->token, pos->cap, str);
	}

	_os_mutex_unlock(d, &pm->mux);

}


static void
_phl_pm_dbg_dump_cmd_q(struct pm_obj *pm, u32 *used,
	char input[][MAX_ARGV], u32 input_num, char *output, u32 out_len)
{
	struct pwr_cmd_entry *cmd = NULL;
	void *d = phl_to_drvpriv(pm->phl_info);
	u32 count = 0;
	char str[PHL_PM_STR_BUF_LEN];

	PS_CNSL(out_len, *used, output + *used, out_len - *used,
		"Current existing commands:\n");

	_os_mutex_lock(d, &pm->mux);

	phl_list_for_loop(cmd, struct pwr_cmd_entry, &pm->cmd_q, list) {

		_phl_pm_convert_comps_to_str(cmd->comp, str);

		PS_CNSL(out_len, *used, output + *used, out_len - *used,
			"Cmd => seq: %-11d type %s comp: %x (%s)\n",
			(int)cmd->token, cmd->type_s,
			(unsigned int)cmd->comp, str);
		count++;
	}

	_os_mutex_unlock(d, &pm->mux);

	if (0 == count) {

		PS_CNSL(out_len, *used, output + *used, out_len - *used, "Empty \n");
	}
}

static void
_phl_pm_dbg_dump_cap_q(struct pm_obj *pm, u32 *used,
	char input[][MAX_ARGV], u32 input_num, char *output, u32 out_len)
{
	struct pwr_cap_entry *cap = NULL;
	void *d = phl_to_drvpriv(pm->phl_info);
	char str[PHL_PM_STR_BUF_LEN];
	u32 count = 0;

	PS_CNSL(out_len, *used, output + *used, out_len - *used,
		"Current existing capabilities request:\n");

	_os_mutex_lock(d, &pm->mux);

	phl_list_for_loop(cap, struct pwr_cap_entry, &pm->cap_q, list) {

		_phl_pm_convert_caps_to_str(cap->cap, str);
		PS_CNSL(out_len, *used, output + *used, out_len - *used,
			"Cap => seq: %-11d cap: %x (%s) \n",
			(int)cap->token, (unsigned int)cap->cap, str);
		count++;
	}

	_os_mutex_unlock(d, &pm->mux);

	if (0 == count) {

		PS_CNSL(out_len, *used, output + *used, out_len - *used, "Empty \n");
	}
}

static void
_phl_pm_ntfy_before_pwr_cfg(struct pm_obj *pm, u8 req_pwr_lvl)
{
	/* From power on to low power */
	if (pm->cur_lvl == PM_PWR_LVL_PWRON) {
		if (req_pwr_lvl == PM_PWR_LVL_PWROFF) {
			#ifdef CONFIG_BTCOEX
			rtw_hal_btc_radio_state_ntfy(pm->phl_info->hal,
				BTC_RFCTRL_WL_OFF);
			#endif
			#if defined(CONFIG_PCI_HCI) && defined(RTW_WKARD_DYNAMIC_LTR)
			rtw_hal_ltr_sw_ctrl_ntfy(pm->phl_info->hal, false);
			#endif
		} else if (req_pwr_lvl <= PM_PWR_LVL_RF_OFF) {
			#ifdef CONFIG_BTCOEX
			rtw_hal_btc_radio_state_ntfy(pm->phl_info->hal,
				BTC_RFCTRL_FW_CTRL);
			#endif
			#if defined(CONFIG_PCI_HCI) && defined(RTW_WKARD_DYNAMIC_LTR)
			if (req_pwr_lvl == PM_PWR_LVL_PWR_GATED)
				rtw_hal_ltr_sw_ctrl_ntfy(pm->phl_info->hal, false);
			#endif
		}
	}
}

static void
_phl_pm_ntfy_after_pwr_cfg(struct pm_obj *pm)
{
	/* From low power to power on */
	if(pm->cur_lvl == PM_PWR_LVL_PWRON) {
		#ifdef CONFIG_BTCOEX
		rtw_hal_btc_radio_state_ntfy(pm->phl_info->hal,
			BTC_RFCTRL_WL_ON);
		#endif
		#if defined(CONFIG_PCI_HCI) && defined(RTW_WKARD_DYNAMIC_LTR)
		rtw_hal_ltr_sw_ctrl_ntfy(pm->phl_info->hal, true);
		#endif
	}
}

static u8
_phl_pm_get_all_cap(struct pm_obj *pm)
{
	u8 cap = HWPS_CAP_MAX - 1;
	struct pwr_cap_entry *pos = NULL;
	void *d = phl_to_drvpriv(pm->phl_info);
	char str[PHL_PM_STR_BUF_LEN] = {0};
	_os_mutex_lock(d, &pm->mux);
	if (list_empty(&pm->cap_q)) {
		PHL_TRACE(COMP_PHL_PS, _PHL_DEBUG_,
			"[PM] CapQ is empty, use PWRON as default\n");
		cap = HWPS_CAP_PWRON;
	} else {
		phl_list_for_loop(pos, struct pwr_cap_entry, &pm->cap_q, list)
			cap &= pos->cap;
	}
	_os_mutex_unlock(d, &pm->mux);

	_phl_pm_convert_caps_to_str(cap, str);

	PHL_TRACE(COMP_PHL_PS, _PHL_DEBUG_,
		"[PM] %s: cap = %x (%s)\n", __func__, cap, str);

	return cap;
}


static u32
_phl_pm_get_all_pwrcomp(struct pm_obj *pm)
{
	u32 comp = 0;
	struct pwr_cmd_entry *pos = NULL;
	void *d = phl_to_drvpriv(pm->phl_info);
	char str[PHL_PM_STR_BUF_LEN] = {0};

	_os_mutex_lock(d, &pm->mux);
	phl_list_for_loop(pos, struct pwr_cmd_entry, &pm->cmd_q, list)
		comp |= pos->comp;
	_os_mutex_unlock(d, &pm->mux);

	_phl_pm_convert_comps_to_str(comp, str);

	PHL_TRACE(COMP_PHL_PS, _PHL_DEBUG_,
		"[PM] %s: comp = %x (%s)\n", __func__, comp, str);

	return comp;
}

static void
_phl_pm_force_pwr_lvl(struct pm_obj *pm, u8 cap)
{
	u8 pwr_lvl;

	pwr_lvl = rtw_hal_ps_pwr_lvl_judge(pm->phl_info->hal, cap, (u32)0);
	rtw_hal_ps_pwr_lvl_cfg(pm->phl_info->phl_com, pm->phl_info->hal, pm->cur_lvl, pwr_lvl);
	pm->cur_lvl = pwr_lvl;
}

u8 _phl_pm_is_rf_off(void *pm_obj)
{
	struct pm_obj *pm = (struct pm_obj *)pm_obj;

	if(pm->rf_state == RTW_RF_OFF)
		return true;
	else
		return false;
}

#ifdef RTW_WKARD_RADIO_IPS_FLOW
static u8
_phl_pm_delay_is_exceeded(struct pm_obj *pm)
{
	u32 cur_time = _os_get_cur_time_ms();
	u32 dif_time;

	if (pm->time_arrived_en_pm == true)
		return true;

	if (cur_time >= pm->start_time) {
		dif_time = cur_time - pm->start_time;
	} else {
		dif_time = cur_time + (RTW_U32_MAX - pm->start_time);
	}

	if(dif_time > pm->init_rf_time) {
		pm->time_arrived_en_pm = true;
		return true;
	} else {
		return false;
	}
}
#endif

static void
_phl_pm_judge_lvl(struct pm_obj *pm)
{
	u8 ps_cap = _phl_pm_get_all_cap(pm);
	u32 cmd_comp = _phl_pm_get_all_pwrcomp(pm);
	u8 pwr_lvl;

#ifdef RTW_WKARD_RADIO_IPS_FLOW
	if (_phl_pm_delay_is_exceeded(pm) == false)
		return;
#endif

	if(_phl_pm_is_rf_off(pm))
	{
		_phl_pm_force_pwr_lvl(pm, HWPS_CAP_PWROFF);
		return;
	}

	/* TODO: Select the a power level to satisfy all power command */
	pwr_lvl = rtw_hal_ps_pwr_lvl_judge(pm->phl_info->hal, ps_cap, cmd_comp);

	PHL_TRACE(COMP_PHL_PS, _PHL_DEBUG_,
		"[PM] pwr_lvl = %x\n", pwr_lvl);

	if (pwr_lvl != pm->cur_lvl) {
		_phl_pm_ntfy_before_pwr_cfg(pm, pwr_lvl);

		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
			"[PM] cur_lvl(%x) => pwr_lvl(%x)\n", pm->cur_lvl, pwr_lvl);
		rtw_hal_ps_pwr_lvl_cfg(pm->phl_info->phl_com, pm->phl_info->hal, pm->cur_lvl, pwr_lvl);
		pm->cur_lvl = pwr_lvl;

		_phl_pm_ntfy_after_pwr_cfg(pm);
	}
}

static void
_phl_pm_exec_cb(struct pm_obj *pm, struct ps_ntfy *ntfy, u32 status)
{
	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[PM] Call callback of hdl(%p), ctx: %p, status %x\n",
		ntfy, ntfy->ctx, status);
	ntfy->cb(pm->phl_info, ntfy, ntfy->ctx, status);
}

static void
_phl_pm_insert_cmd(struct pm_obj *pm, struct pwr_cmd_entry *cmd)
{
	void *d = phl_to_drvpriv(pm->phl_info);

	_os_mutex_lock(d, &pm->mux);
	list_add(&cmd->list, &pm->cmd_q);
	pm->cmd_cnt++;
	_os_mutex_unlock(d, &pm->mux);

	cmd->enq = true;
}

static u8
_phl_pm_remove_cmd(struct pm_obj *pm, u32 token)
{
	struct pwr_cmd_entry *pos = NULL;
	void *d = phl_to_drvpriv(pm->phl_info);
	u8 find = false;

	PHL_TRACE(COMP_PHL_PS, _PHL_DEBUG_,
		"[PM] pm_remove_cmd, token = %d\n", token);

	_os_mutex_lock(d, &pm->mux);
	phl_list_for_loop(pos, struct pwr_cmd_entry, &pm->cmd_q, list) {
		if (pos->token == token) {
			PHL_TRACE(COMP_PHL_PS, _PHL_DEBUG_,
				"[PM] %s: Find matched cmd! type (%s) \n",
				__func__, pos->type_s);
			list_del(&pos->list);
			pos->enq = false;
			pm->cmd_cnt--;
			_os_kmem_free(d, pos, sizeof(*pos));
			find = true;
			break;
		}
	}
	_os_mutex_unlock(d, &pm->mux);

	return find;
}

static void
_phl_pm_insert_cap(struct pm_obj *pm, struct pwr_cap_entry *cap)
{
	void *d = phl_to_drvpriv(pm->phl_info);

	_os_mutex_lock(d, &pm->mux);
	list_add(&cap->list, &pm->cap_q);
	cap->enq = true;
	pm->cap_cnt++;
	_os_mutex_unlock(d, &pm->mux);
}

static u8
_phl_pm_remove_cap(struct pm_obj *pm, u32 token)
{
	struct pwr_cap_entry *pos = NULL;
	void *d = phl_to_drvpriv(pm->phl_info);
	u8 find = false;

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[PM] %s: token = %d\n", __func__, token);

	_os_mutex_lock(d, &pm->mux);
	phl_list_for_loop(pos, struct pwr_cap_entry, &pm->cap_q, list) {
		if (pos->token == token) {
			PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
				"[PM] Find matched cap!\n");
			list_del(&pos->list);
			pos->enq = false;
			pm->cap_cnt--;
			_os_kmem_free(d, pos, sizeof(*pos));
			find = true;
			break;
		}
	}
	_os_mutex_unlock(d, &pm->mux);

	return find;
}

static void
_phl_pm_init_obj(struct pm_obj *pm, struct phl_info_t *phl_info)
{
	void *d = phl_to_drvpriv(phl_info);

	/* init obj local use variable */
	pm->phl_info = phl_info;
	pm->init_rf_time = 10000;
	pm->cur_lvl = 4;
	pm->rf_state = RTW_RF_ON;

	INIT_LIST_HEAD(&pm->cmd_q);
	INIT_LIST_HEAD(&pm->cap_q);

	_os_mutex_init(d, &pm->mux);
}

static void
_phl_pm_deinit_obj(struct pm_obj *pm, void *d)
{
	struct pwr_cmd_entry *cmd_pos = NULL;
	struct pwr_cmd_entry *cmd_n = NULL;
	struct pwr_cap_entry *cap_pos = NULL;
	struct pwr_cap_entry *cap_n = NULL;

	_os_mutex_lock(d, &pm->mux);

	phl_list_for_loop_safe(cmd_pos, cmd_n, struct pwr_cmd_entry,
				&pm->cmd_q, list) {

		list_del(&cmd_pos->list);
		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_, "[PM] del cmd %p\n", cmd_pos);
		_os_kmem_free(d, cmd_pos, sizeof(*cmd_pos));
	}

	phl_list_for_loop_safe(cap_pos, cap_n, struct pwr_cap_entry,
				&pm->cap_q, list) {

		list_del(&cap_pos->list);
		PHL_TRACE(COMP_PHL_PS, _PHL_INFO_, "[PM] del cap %p\n", cap_pos);
		_os_kmem_free(d, cap_pos, sizeof(*cap_pos));
	}

	_os_mutex_unlock(d, &pm->mux);
	_os_mutex_deinit(d, &pm->mux);
}

/* Create power manager object
 * @fsm: FSM main structure which created by phl_pm_new_fsm()
 * @phl_info: private data structure to invoke hal/phl function
 *
 * return
 * pm_obj: structure of command object (Do NOT expose)
 */
static struct pm_obj *
_phl_pm_new_obj(struct phl_info_t *phl_info)
{
	struct pm_obj *pm;
	void *d = phl_to_drvpriv(phl_info);

	pm = (struct pm_obj *)_os_kmem_alloc(d, sizeof(*pm));
	if (pm == NULL) {
		PHL_ERR("pm: malloc obj fail.\n");
		return NULL;
	}

	_os_mem_set(d, pm, 0, sizeof(*pm));

	return pm;
}

/* Destory pm object */
/* @pm: local created pm object
 */
static void _phl_pm_free_obj(struct pm_obj *pm, void *d)
{
	/* free pm_obj */
	_os_kmem_free(d, pm, sizeof(*pm));
}

/* For EXTERNAL application to initialize power manager
 * @phl_info: private data structure to invoke hal/phl function
 *
 * return: rtw_phl_status
 */
enum rtw_phl_status
phl_pm_init(void **pm_obj, struct phl_info_t *phl_info)
{
	*pm_obj = _phl_pm_new_obj(phl_info);
	if (*pm_obj == NULL) {
		return RTW_PHL_STATUS_FAILURE;
	}

	_phl_pm_init_obj(*pm_obj, phl_info);

	return RTW_PHL_STATUS_SUCCESS;
}

/* For EXTERNAL application to deinitialize power manager
 * @phl_info: private data structure to invoke hal/phl function
 *
 */
void phl_pm_deinit(void *pm_obj)
{
	struct pm_obj *pm = (struct pm_obj *)pm_obj;
	void *d;

	if (pm == NULL)
		return;

	d = phl_to_drvpriv(pm->phl_info);
	_phl_pm_deinit_obj(pm, d);
	_phl_pm_free_obj(pm, d);
	pm = NULL;
}

void phl_pm_start(void *pm_obj)
{
	struct pm_obj *pm = (struct pm_obj *)pm_obj;
	struct rtw_phl_com_t *phl_com = pm->phl_info->phl_com;
	struct rtw_ps_cap_t *ps_cap = &phl_com->dev_sw_cap.ps_cap;

	pm->rf_state = ps_cap->init_rf_state;
	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[PM] Init radio state: %d\n", pm->rf_state);
	#ifdef RTW_WKARD_RADIO_IPS_FLOW
	pm->time_arrived_en_pm = false;
	pm->start_time = _os_get_cur_time_ms();
	#endif
}

void phl_pm_stop(void *pm_obj)
{
	struct pm_obj *pm = (struct pm_obj *)pm_obj;
	struct rtw_phl_com_t *phl_com = pm->phl_info->phl_com;
	struct rtw_ps_cap_t *ps_cap = &phl_com->dev_sw_cap.ps_cap;

	if (true == TEST_STATUS_FLAG(phl_com->dev_state,
	                             RTW_DEV_SURPRISE_REMOVAL)) {
		PHL_WARN("%s(): Device has removed, skip HW stop functions!\n", __func__);
		return;
	}

	_phl_pm_force_pwr_lvl(pm, HWPS_CAP_PWRON);
	ps_cap->init_rf_state = pm->rf_state;
}

void phl_pm_periodic_chk(void *pm_obj)
{
	struct pm_obj *pm = (struct pm_obj *)pm_obj;

#ifdef RTW_WKARD_RADIO_IPS_FLOW
	if (pm->time_arrived_en_pm == false)
		_phl_pm_judge_lvl(pm);
#endif
}

u8 phl_pm_is_low_power(void *pm_obj)
{
	struct pm_obj *pm = (struct pm_obj *)pm_obj;

	if (pm->cur_lvl > PM_PWR_LVL_PWROFF &&
		pm->cur_lvl <= PM_PWR_LVL_CLK_GATED) {
		return true;
	} else {
		return false;
	}
}

u8 phl_pm_test_pwr_level(void *pm_obj, u32 test_comp)
{
	struct pm_obj *pm = (struct pm_obj *)pm_obj;
	u8 ps_cap = _phl_pm_get_all_cap(pm);
	u32 cmd_comp = _phl_pm_get_all_pwrcomp(pm);
	u8 pwr_lvl;

	cmd_comp |= test_comp;
	pwr_lvl = rtw_hal_ps_pwr_lvl_judge(pm->phl_info->hal, ps_cap, cmd_comp);

	if (pwr_lvl > pm->cur_lvl) {
		PHL_TRACE(COMP_PHL_PS, _PHL_DEBUG_,
			"[PM] test_lvl(%x) > cur_lvl(%x)\n",
			pwr_lvl, pm->cur_lvl);
		return false;
	} else {
		return true;
	}
}

enum rtw_phl_status
phl_pm_issue_pwr_cmd(void *pm_obj, struct ps_ntfy *ntfy)
{
	struct pm_obj *pm = (struct pm_obj *)pm_obj;
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	struct pwr_cmd_entry *cmd = NULL;
	void *d = phl_to_drvpriv(pm->phl_info);

	do {
		cmd = _os_kmem_alloc(d, sizeof(*cmd));
		if (cmd == NULL) {
			PHL_ERR("%s: alloc cmd fail.\n", __func__);
			status = RTW_PHL_STATUS_RESOURCE;
			break;
		}

		INIT_LIST_HEAD(&cmd->list);
		cmd->token = ntfy->token;
		cmd->comp = ntfy->u.pm_comp;
		_os_mem_cpy(d, cmd->type_s, ntfy->type_s, 50);

		PHL_TRACE(COMP_PHL_PS, _PHL_DEBUG_,
			"[PM] New cmd %p, comp %x, type %s \n", cmd, cmd->comp,
			cmd->type_s);

		_phl_pm_insert_cmd(pm, cmd);
		_phl_pm_judge_lvl(pm);

	} while(0);

	if (ntfy->cb)
		_phl_pm_exec_cb(pm, ntfy, status);

	phl_ps_ntfy_completion(pm->phl_info, ntfy);

	return status;
}

void phl_pm_cancel_pwr_cmd(void *pm_obj, struct ps_ntfy *ntfy)
{
	struct pm_obj *pm = (struct pm_obj *)pm_obj;

	if (_phl_pm_remove_cmd(pm, ntfy->token))
		_phl_pm_judge_lvl(pm);

	_phl_pm_show_all_cmds(pm);

	phl_ps_ntfy_completion(pm->phl_info, ntfy);
}

enum rtw_phl_status
phl_pm_issue_pwr_cap(void *pm_obj, struct ps_ntfy *ntfy)
{
	struct pm_obj *pm = (struct pm_obj *)pm_obj;
	struct pwr_cap_entry *cap = NULL;
	void *d = phl_to_drvpriv(pm->phl_info);
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;

	do {
		cap = _os_kmem_alloc(d, sizeof(*cap));
		if (cap == NULL) {
			PHL_ERR("%s: alloc cap fail.\n", __func__);
			pstatus = RTW_PHL_STATUS_RESOURCE;
			break;
		}


		INIT_LIST_HEAD(&cap->list);
		cap->token = ntfy->token;
		cap->cap = ntfy->u.pm_cap;

		_phl_pm_insert_cap(pm, cap);
		_phl_pm_judge_lvl(pm);

	} while (0);

	phl_ps_ntfy_completion(pm->phl_info, ntfy);

	return pstatus;
}

void phl_pm_cancel_pwr_cap(void *pm_obj, struct ps_ntfy *ntfy)
{
	struct pm_obj *pm = (struct pm_obj *)pm_obj;

	if(_phl_pm_remove_cap(pm, ntfy->token))
		_phl_pm_judge_lvl(pm);

	_phl_pm_show_all_caps(pm);

	phl_ps_ntfy_completion(pm->phl_info, ntfy);

}

enum rtw_phl_status
phl_pm_set_radio_state(void *pm_obj, struct ps_ntfy *ntfy)
{
	struct pm_obj *pm = (struct pm_obj *)pm_obj;
	void *d = phl_to_drvpriv(pm->phl_info);

	pm->rf_state = ntfy->u.rf_state;

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
		"[PM] Set radio state: %d.\n", pm->rf_state);

	_phl_pm_judge_lvl(pm);

	phl_ps_ntfy_completion(pm->phl_info, ntfy);

	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status
phl_pm_wow_cfg(void *pm_obj, u8 wow_cap)
{
	struct pm_obj *pm = (struct pm_obj *)pm_obj;
	enum rtw_phl_status status;
	u8 pwr_lvl;

	pwr_lvl = rtw_hal_ps_pwr_lvl_judge(pm->phl_info->hal, wow_cap, (u32)0);
	status = rtw_hal_ps_pwr_lvl_cfg(pm->phl_info->phl_com,
				pm->phl_info->hal, pm->cur_lvl, pwr_lvl);
	if (status == RTW_PHL_STATUS_SUCCESS)
		pm->cur_lvl = pwr_lvl;

	return status;
}

void phl_pm_force_power_on(void *pm_obj)
{
	struct pm_obj *pm = (struct pm_obj *)pm_obj;

	_phl_pm_force_pwr_lvl(pm, HWPS_CAP_PWRON);
}

void phl_pm_dbg_dump_obj(void *pm_obj, u32 *used, char input[][MAX_ARGV],
			u32 input_num, char *output, u32 out_len)
{
	struct pm_obj *pm = (struct pm_obj *)pm_obj;

	PS_CNSL(out_len, *used, output + *used, out_len - *used,
		"========== PHL PM Info ==========\n");

	PS_CNSL(out_len, *used, output + *used, out_len - *used,
		"power level: %s, rf state: %s\n",
		phl_ps_id_to_str(PS_STR_PWR_LVL, (u32)pm->cur_lvl),
		phl_ps_id_to_str(PS_STR_RADIO, (u32)pm->rf_state));

	_phl_pm_dbg_dump_cmd_q(pm, used, input, input_num,
					output, out_len);
	_phl_pm_dbg_dump_cap_q(pm, used, input, input_num,
					output, out_len);
}

void phl_pm_watchdog(void *pm_obj)
{
	struct pm_obj *pm = (struct pm_obj *)pm_obj;

	PHL_TRACE(COMP_PHL_PS, _PHL_INFO_,
			"[PM] power level: %s, rf state: %s \n",
			phl_ps_id_to_str(PS_STR_PWR_LVL, (u32)pm->cur_lvl),
			phl_ps_id_to_str(PS_STR_RADIO, (u32)pm->rf_state));

}

#endif /* CONFIG_PS_PM */
