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
#ifndef __PHL_PS_FSM_H__
#define __PHL_PS_FSM_H__

enum PS_EV_ID {
	PS_EV_REQ_NOTIFY,
	PS_EV_PERIODIC_ALARM,
	PS_EV_PERIODIC_CHK,
	PS_EV_ROLE_NOTIFY,
	PS_EV_ROLE_TYPE_NOTIFY,
	PS_EV_ISSUE_CMD,
	PS_EV_CANCEL_CMD,
	PM_EV_ISSUE_CMD,
	PM_EV_CANCEL_CMD,
	PM_EV_SET_RADIO,
	PM_EV_ISSUE_CAP,
	PM_EV_CANCEL_CAP,
	IPS_EV_ISSUE_CMD,
	IPS_EV_CANCEL_CMD,
	LPS_EV_ISSUE_CMD,
	LPS_EV_CANCEL_CMD,
	LPS_EV_ADD_ROLE,
	LPS_EV_DEL_ROLE,
	PS_EV_MAX
};

enum ps_protocol {
	PS_PTCL_IPS = 0,
	PS_PTCL_LPS = 1,
	PS_PTCL_MAX = 2,
};

#define PS_PTCL_NUM PS_PTCL_MAX

struct ps_ptcl_info {
	/* token for leave protocol */
	u32 token;

	/* whether leave protocol */
	enum rtw_phl_status status;
};

struct ps_role_info {
	/* role state */
	enum role_state rstate;

	/* core role type */
	enum role_type rtype;

	/* whether req ps */
	u8 ps_req;

	/* token for leave all ps */
	u32 ps_token;
};

struct ps_token_list {
	/* watchdog */
	u32 token_watchdog;

	/* create_mac */
	u32 token_create_mac;

	/* delete_mac */
	u32 token_delete_mac;

	/* change_port */
	u32 token_change_port;

	/* connect */
	u32 token_connect;

	/* disassociate */
	u32 token_disassociate;

	/* start_ap */
	u32 token_start_ap;
};

struct ps_obj {
	_os_mutex mux;
	struct fsm_main *fsm;
	struct fsm_obj *fsm_obj;
	struct phl_info_t *phl_info;

	void *pm_obj;
	void *ips_obj;
	void *lps_obj;

	/* periodic timer */
	bool ptmr_init;
	bool ptmr_stop;
	_os_timer ptmr; /* unit in ms */

	_os_lock ntfy_lock;

	/* req_q for saving pending request */
	struct	list_head req_q;
	u8 req_num;
	_os_lock req_q_lock;

	/* cmd_q for saving requested command
	 * Allow all ps: cmd q is empty
	 * Leave all ps: cmd q is not empty
	 */
	struct list_head cmd_q;
	u32 cmd_cnt;

	/* generate token */
	u32 cur_seq;

	/* token for leave ips */
	u32 ips_token;

	struct ps_role_info role_info[MAX_WIFI_ROLE_NUMBER];

	/* All procotol information */
	struct ps_ptcl_info ptcl_info[PS_PTCL_NUM];

	/* req = 0: cmd q is empty
	 * req = 1: cmd q is not empty
	 */
	u8 req : 1;

 	/* leave_all_ptcl = 1: all protocol cannot enter ps */
	u8 leave_all_ptcl : 1;

	/* whether req ips */
	u8 ips_req : 1;
	u8 rsvd : 5;

	/* The list to save token temporarily */
	struct ps_token_list token_list;

	bool ser_ongoing;
};

#ifdef CONFIG_PS

/* fsm private struct */
struct fsm_root;
struct fsm_main;

/* power save fsm init api */
enum rtw_phl_status
phl_ps_init(struct phl_info_t *phl_info);
void phl_ps_deinit(struct phl_info_t *phl_info);
enum rtw_phl_status
phl_ps_fsm_start(struct phl_info_t *phl_info);
void phl_ps_fsm_stop(struct phl_info_t *phl_info);

enum rtw_phl_status
phl_ps_send_msg(struct phl_info_t *phl, void *param, u32 sz, u16 event);
u32 phl_ps_get_token(void *ps_obj);
u8 phl_ps_test_pwr_level(struct phl_info_t *phl, u32 test_comp);
enum rtw_phl_status
phl_ps_issue_pm_cmd(struct phl_info_t *phl, u32 *token, u32 comp);
void phl_ps_cancel_pm_cmd(struct phl_info_t *phl, u32 token);
enum rtw_phl_status
phl_ps_issue_pm_cap(struct phl_info_t *phl, u32 *token, u32 cap);
void phl_ps_cancel_pm_cap(struct phl_info_t *phl, u32 token);
void phl_ps_ntfy_completion(struct phl_info_t *phl, struct ps_ntfy *ntfy);


#else
#define phl_ps_init(_phl_info) RTW_PHL_STATUS_SUCCESS
#define phl_ps_deinit(_phl_info) do {} while (0)
#define phl_ps_fsm_start(_phl_info) RTW_PHL_STATUS_SUCCESS
#define phl_ps_fsm_stop(_phl_info) do {} while (0)
#define phl_ps_send_msg(_phl, _param, _sz, _event) RTW_PHL_STATUS_SUCCESS
#define phl_ps_get_token(_ps_obj) 0
#define phl_ps_ntfy_completion(_phl, _ntfy)
#endif /*CONFIG_PS */

#endif /* __PHL_PS_FSM_H__ */

