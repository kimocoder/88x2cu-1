
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
#define _PHL_CMD_SER_C_
#include "phl_headers.h"
#include "phl_api.h"

#define CMD_SER_L0 0x00000001
#define CMD_SER_L1 0x00000002
#define CMD_SER_L2 0x00000004

enum _CMD_SER_EVENT_SOURCE {
	CMD_SER_SRC_UNKNOWN = 0,
	CMD_SER_SRC_INT = BIT0, // ser event from interrupt
	CMD_SER_SRC_POLL = BIT1, // ser event by polling io
};

enum _CMD_SER_TIMER_STATE {
	CMD_SER_NOT_OCCUR = 0,
	CMD_SER_M1 = BIT0, //POLL_IO
	CMD_SER_M2 = BIT1, //POLL_FW
	CMD_SER_M3 = BIT2,
	CMD_SER_M4 = BIT3,
	CMD_SER_M5 = BIT4,
	CMD_SER_M9 = BIT5,
};

#define CMD_SER_FW_TIMEOUT 1000 /* ms */
#define CMD_SER_POLLING_INTERVAL 10 /* ms */
#define CMD_SER_USB_POLLING_INTERVAL_IDL 1000 /* ms */
#define CMD_SER_USB_POLLING_INTERVAL_ACT 10 /* ms */

#define CMD_SER_POLL_IO_TIMES 200
#define CMD_SER_USB_POLL_IO_TIMES 300

#define CMD_SER_LOG_SIZE 10

struct sts_l2 {
	_os_list list;
	u8 idx;
	u8 ser_log;
};

struct cmd_ser {
	struct phl_info_t *phl_info;
	void* dispr;
	u8 state;
	_os_lock _lock;

	u8 evtsrc;
	int poll_cnt;
	_os_timer poll_timer;

	/* L2 log :
	//    If L2 triggered, set ser_log = state-of-cmd_ser
	*/
	struct phl_queue stslist;
	struct sts_l2 stsl2[CMD_SER_LOG_SIZE];
	u8 bserl2;
	u8 (*ser_l2_hdlr)(void *drv);
};


#if 0 // NEO mark off first

void _ser_dump_stsl2(struct cmd_ser *cser)
{
	u8 idx =0;

	for(idx = 0; idx < CMD_SER_LOG_SIZE; idx++) {
		if(cser->stsl2[idx].ser_log || cser->stsl2[idx].idx >= CMD_SER_LOG_SIZE) {
			PHL_ERR("%s :: [%d] %d - ser_log = 0x%X \n", __func__,
				idx, cser->stsl2[idx].idx, cser->stsl2[idx].ser_log);
		}
	}
}

void _ser_reset_status(struct cmd_ser *cser)
{
	void *drv = phl_to_drvpriv(cser->phl_info);

	_os_spinlock(drv, &cser->_lock, _bh, NULL);
	cser->state = CMD_SER_NOT_OCCUR;
	_os_spinunlock(drv, &cser->_lock, _bh, NULL);
}


void _ser_set_status(struct cmd_ser *cser, u8 serstatus)
{
	void *drv = phl_to_drvpriv(cser->phl_info);

	_os_spinlock(drv, &cser->_lock, _bh, NULL);
	cser->state |= serstatus;
	_os_spinunlock(drv, &cser->_lock, _bh, NULL);
}

void _ser_clear_status(struct cmd_ser *cser, u8 serstatus)
{
	void *drv = phl_to_drvpriv(cser->phl_info);

	_os_spinlock(drv, &cser->_lock, _bh, NULL);
	cser->state &= ~(serstatus);
	_os_spinunlock(drv, &cser->_lock, _bh, NULL);
}

void _ser_polling_event(struct cmd_ser *cser)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	struct phl_msg nextmsg = {0};
	u8 idx = 0xff;

	if(cser->evtsrc != CMD_SER_SRC_POLL)
		return;

	SET_MSG_MDL_ID_FIELD(nextmsg.msg_id, PHL_MDL_SER);
	SET_MSG_EVT_ID_FIELD(nextmsg.msg_id, MSG_EVT_SER_EVENT_CHK);
	nextmsg.band_idx = HW_BAND_0;
	pstatus = phl_disp_eng_send_msg(cser->phl_info, &nextmsg, NULL, NULL);
	if(pstatus != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("%s :: [_ser_polling_event] dispr_send_msg failed\n", __func__);
	}
}

static void _ser_l1_notify(struct cmd_ser *cser)
{
	struct phl_msg nextmsg = {0};

	//phl_ser_set_status(cser, CMD_SER_L1);
	SET_MSG_MDL_ID_FIELD(nextmsg.msg_id, PHL_MDL_SER);
	SET_MSG_EVT_ID_FIELD(nextmsg.msg_id, MSG_EVT_SER_L1);
	phl_msg_hub_send(cser->phl_info, NULL, &nextmsg);
}

static void _ser_l2_notify(struct cmd_ser *cser)
{
	struct phl_info_t *phl_info = cser->phl_info;
	void *drv = phl_to_drvpriv(phl_info);
	struct sts_l2 *stsl2 = NULL;
	struct phl_msg nextmsg = {0};
	_os_list* obj = NULL;

	if(pq_pop(drv, &cser->stslist, &obj, _first, _ps)) {
		stsl2 = (struct sts_l2*)obj;

		/* Rotate stslist : 0~ (CMD_SER_LOG_SIZE-1) are unused index*/
		stsl2->idx+= CMD_SER_LOG_SIZE;
		stsl2->ser_log = cser->state;
		pq_push(drv, &cser->stslist, &stsl2->list, _tail, _ps);
	}
	_ser_dump_stsl2(cser);

	//phl_ser_set_status(cser, CMD_SER_L2);
	/* L2 can't be rescued, bserl2 wouldn't reset. */
	/* comment out: wait for new ser flow to handle L2*/
	/* cser->bserl2 = true; */

	if(cser->ser_l2_hdlr)
		cser->ser_l2_hdlr(phl_to_drvpriv(phl_info));

	/* SER recovery fail: clr pending event from MDL_SER & msg return failed from others module*/
	phl_disp_eng_clr_pending_msg(cser->phl_info, HW_BAND_0);

	SET_MSG_MDL_ID_FIELD(nextmsg.msg_id, PHL_MDL_SER);
	SET_MSG_EVT_ID_FIELD(nextmsg.msg_id, MSG_EVT_SER_L2);
	phl_msg_hub_send(cser->phl_info, NULL, &nextmsg);
}

static void _ser_m2_notify(
	struct cmd_ser *cser)
{
	struct phl_info_t *phl_info = cser->phl_info;
	u32 mac_err = 0;

	_ser_clear_status(cser, CMD_SER_M1);
	_ser_set_status(cser, CMD_SER_M2);

	/* send M2 event to fw*/
	mac_err = rtw_hal_ser_set_error_status(phl_info->hal, RTW_PHL_SER_L1_DISABLE_EN);
	PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_, "_ser_m2_notify:: RTW_PHL_SER_L1_DISABLE_EN, status 0x%x\n",mac_err);
}

static void _ser_m3_m5_waiting(
	struct cmd_ser *cser)
{
	struct phl_info_t *phl_info = cser->phl_info;
	void *drv = phl_to_drvpriv(phl_info);
	int poll_cnt = 0, intvl = CMD_SER_FW_TIMEOUT;

	if(cser->evtsrc == CMD_SER_SRC_POLL) {
		/* CMD_SER_POLLING_INTERVAL = CMD_SER_FW_TIMEOUT/ CMD_SER_USB_POLLING_INTERVAL_ACT */
		poll_cnt = CMD_SER_POLLING_INTERVAL;
		intvl = CMD_SER_USB_POLLING_INTERVAL_ACT;
	}

	cser->poll_cnt = poll_cnt;
	/* wait M3 or M5*/
	_os_set_timer(drv, &cser->poll_timer, intvl);
}

static void _ser_m4_notify(
	struct cmd_ser *cser)
{
	struct phl_info_t *phl_info = cser->phl_info;
	u32 mac_err = 0;

	_ser_clear_status(cser, CMD_SER_M3);
	_ser_set_status(cser, CMD_SER_M4);

	/* send M4 event */
	mac_err = rtw_hal_ser_set_error_status(phl_info->hal, RTW_PHL_SER_L1_RCVY_EN);
	PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_, "_ser_m4_notify:: RTW_PHL_SER_L1_RCVY_EN, status 0x%x\n",mac_err);
}

static void _ser_poll_timer_cb(void *priv)
{
	struct cmd_ser *cser = (struct cmd_ser *)priv;

	struct phl_msg nextmsg = {0};
	struct phl_msg_attribute attr = {0};
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	void *dispr = NULL;

	SET_MSG_MDL_ID_FIELD(nextmsg.msg_id, PHL_MDL_SER);

	if(TEST_STATUS_FLAG(cser->state, CMD_SER_M1))
		SET_MSG_EVT_ID_FIELD(nextmsg.msg_id, MSG_EVT_SER_IO_TIMER_EXPIRE);
	else if(TEST_STATUS_FLAG(cser->state, CMD_SER_M2) &&
			(cser->poll_cnt > 0))
		SET_MSG_EVT_ID_FIELD(nextmsg.msg_id, MSG_EVT_SER_EVENT_CHK);

	else if(TEST_STATUS_FLAG(cser->state, CMD_SER_M4) &&
			(cser->poll_cnt > 0))
		SET_MSG_EVT_ID_FIELD(nextmsg.msg_id, MSG_EVT_SER_EVENT_CHK);
	else if(TEST_STATUS_FLAG(cser->state, CMD_SER_M2))
		SET_MSG_EVT_ID_FIELD(nextmsg.msg_id, MSG_EVT_SER_FW_TIMER_EXPIRE);
	else if(TEST_STATUS_FLAG(cser->state, CMD_SER_M4))
		SET_MSG_EVT_ID_FIELD(nextmsg.msg_id, MSG_EVT_SER_FW_TIMER_EXPIRE);
	else
		SET_MSG_EVT_ID_FIELD(nextmsg.msg_id, MSG_EVT_SER_EVENT_CHK);

	nextmsg.band_idx = HW_BAND_0;

	if(MSG_EVT_ID_FIELD(nextmsg.msg_id)) {
		PHL_DBG("%s :: nextmsg->msg_id= 0x%X\n", __func__, MSG_EVT_ID_FIELD(nextmsg.msg_id));
		pstatus = phl_disp_eng_send_msg(cser->phl_info, &nextmsg, &attr, NULL);
		if(pstatus != RTW_PHL_STATUS_SUCCESS)
			PHL_ERR("%s :: [SER_TIMER_CB] dispr_send_msg failed\n", __func__);
	}
}

enum phl_mdl_ret_code _ser_m1_polling_io_pcie_sdio(
	struct cmd_ser *cser, bool breset)
{
	struct phl_info_t *phl_info = cser->phl_info;
	struct phl_hci_trx_ops *ops = phl_info->hci_trx_ops;
	void *drv = phl_to_drvpriv(phl_info);

	if (false == ops->is_tx_pause(phl_info) ||
	    false == ops->is_rx_pause(phl_info)) {

		/* prevent infinite polling */
		if(breset)
			cser->poll_cnt = CMD_SER_POLL_IO_TIMES;

		/* pci/sdio: polling fail; wait for a while */
		_os_set_timer(drv, &cser->poll_timer, CMD_SER_POLLING_INTERVAL);

		return MDL_RET_PENDING;
	}

	return MDL_RET_SUCCESS;
}

enum phl_mdl_ret_code _ser_m1_polling_io_usb(
	struct cmd_ser *cser, bool breset)
{
	struct phl_info_t *phl_info = cser->phl_info;
	struct phl_hci_trx_ops *ops = phl_info->hci_trx_ops;
	void *drv = phl_to_drvpriv(phl_info);

	if (false == ops->is_tx_pause(phl_info)) {
		if(breset)
			cser->poll_cnt = CMD_SER_USB_POLL_IO_TIMES;

		_os_set_timer(drv, &cser->poll_timer, CMD_SER_USB_POLLING_INTERVAL_ACT);
		return MDL_RET_PENDING;
	}

	return MDL_RET_SUCCESS;
}

enum phl_mdl_ret_code _ser_m1_pause_trx_pcie(
	struct cmd_ser *cser)
{
	struct phl_info_t *phl_info = cser->phl_info;
	struct phl_hci_trx_ops *ops = phl_info->hci_trx_ops;

	/* SW pause */
	ops->req_tx_stop(phl_info);
	rtw_phl_tx_req_notify(phl_info);

	/* PCIE stop dma */
	if(rtw_hal_lv1_rcvy(phl_info->hal, RTW_PHL_SER_LV1_RCVY_STEP_1) != 0){
		/* Level 1 recovery failed --> L2 */

		/* SW resume */
		ops->trx_resume(phl_info, PHL_REQ_PAUSE_TX);
		rtw_phl_tx_req_notify(phl_info);

		// SER_ST_L2
		_ser_l2_notify(cser);

		/* ToDo : L2 can't be rescued */
		_ser_reset_status(cser);
		return MDL_RET_SUCCESS;
	}

	ops->req_rx_stop(phl_info);
	rtw_phl_start_rx_process(phl_info);

	/* Polling io idle state */
	if(_ser_m1_polling_io_pcie_sdio(cser, true) == MDL_RET_SUCCESS) {

		ops->trx_reset(phl_info);

		/* send M2 event to fw and wait for M3*/
		_ser_m2_notify(cser); /*set CMD_SER_M2 */
		_ser_m3_m5_waiting(cser);
	}
	return MDL_RET_SUCCESS;
}


enum phl_mdl_ret_code _ser_m1_pause_trx_usb(
	struct cmd_ser *cser)
{
	struct phl_info_t *phl_info = cser->phl_info;
	struct phl_hci_trx_ops *ops = phl_info->hci_trx_ops;

	ops->req_tx_stop(phl_info);
	rtw_phl_tx_req_notify(phl_info);

	if(_ser_m1_polling_io_usb(cser, true) == MDL_RET_SUCCESS) {

		rtw_hal_lv1_rcvy(phl_info->hal, RTW_PHL_SER_LV1_RCVY_STEP_1);

		/* send M2 event to fw and wait for M3*/
		_ser_m2_notify(cser); /*set CMD_SER_M2 */
		_ser_m3_m5_waiting(cser);
	}
	return MDL_RET_SUCCESS;

}

enum phl_mdl_ret_code _ser_m1_pause_trx_sdio(
	struct cmd_ser *cser)
{
	struct phl_info_t *phl_info = cser->phl_info;
	struct phl_hci_trx_ops *ops = phl_info->hci_trx_ops;

	ops->req_tx_stop(phl_info);
	rtw_phl_tx_req_notify(phl_info);

	ops->req_rx_stop(phl_info);
	rtw_phl_start_rx_process(phl_info);

	if(_ser_m1_polling_io_pcie_sdio(cser, true) == MDL_RET_SUCCESS) {

		rtw_hal_config_interrupt(phl_info->hal, RTW_PHL_DIS_HCI_INT);

		if (rtw_hal_lv1_rcvy(phl_info->hal, RTW_PHL_SER_LV1_RCVY_STEP_1) != 0) {

			rtw_hal_config_interrupt(phl_info->hal, RTW_PHL_EN_HCI_INT);

			_ser_l2_notify(cser);

			/* ToDo : L2 can't be rescued */
			_ser_reset_status(cser);
			return MDL_RET_SUCCESS;
		}

		rtw_hal_config_interrupt(phl_info->hal, RTW_PHL_SER_HANDSHAKE_MODE);

		/* send M2 event to fw and wait for M3*/
		_ser_m2_notify(cser); /*set CMD_SER_M2 */
		_ser_m3_m5_waiting(cser);
	}

	return MDL_RET_SUCCESS;
}

static void _ser_resume_trx_pcie(
	struct cmd_ser *cser)
{
	struct phl_info_t *phl_info = cser->phl_info;
	struct phl_hci_trx_ops *ops = phl_info->hci_trx_ops;

	ops->trx_resume(phl_info, PHL_REQ_PAUSE_RX);
	rtw_phl_start_rx_process(phl_info);
	ops->trx_resume(phl_info, PHL_REQ_PAUSE_TX);
	rtw_phl_tx_req_notify(phl_info);
}

static void _ser_resume_trx_usb(
	struct cmd_ser *cser)
{
	struct phl_info_t *phl_info = cser->phl_info;
	struct phl_hci_trx_ops *ops = phl_info->hci_trx_ops;

	ops->trx_resume(phl_info, PHL_REQ_PAUSE_TX);
	rtw_phl_tx_req_notify(phl_info);
}

static void _ser_resume_trx_sdio(
	struct cmd_ser *cser)
{
	struct phl_info_t *phl_info = cser->phl_info;
	struct phl_hci_trx_ops *ops = phl_info->hci_trx_ops;

	ops->trx_resume(phl_info, PHL_REQ_PAUSE_RX);
	rtw_phl_start_rx_process(phl_info);
	ops->trx_resume(phl_info, PHL_REQ_PAUSE_TX);
	rtw_phl_tx_req_notify(phl_info);

	rtw_hal_config_interrupt(phl_info->hal, RTW_PHL_EN_HCI_INT);
}

enum phl_mdl_ret_code _ser_fail_ev_hdlr(
	void *dispr, void *priv, struct phl_msg *msg)
{
	struct cmd_ser *cser = (struct cmd_ser *)priv;
	struct phl_info_t *phl_info = cser->phl_info;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	void *drv = phl_to_drvpriv(phl_info);

	PHL_INFO("%s :: [MSG_FAIL] MDL =%d , EVT_ID=%d\n", __func__,
		 MSG_MDL_ID_FIELD(msg->msg_id), MSG_EVT_ID_FIELD(msg->msg_id));

	if ((false == TEST_STATUS_FLAG(phl_com->dev_state,
	                               RTW_DEV_SURPRISE_REMOVAL)) &&
	    (false == IS_MSG_CANNOT_IO(msg->msg_id))) {
		// Trigger polling timer
		_ser_polling_event(cser);
	} else {
		if (phl_com->hci_type == RTW_HCI_USB)
			_os_set_timer(drv, &cser->poll_timer, CMD_SER_USB_POLLING_INTERVAL_IDL); //1000ms
	}

	return MDL_RET_SUCCESS;
}
enum phl_mdl_ret_code _ser_hdl_external_evt(
	void *dispr, void *priv, struct phl_msg *msg)
{
	struct cmd_ser *cser = (struct cmd_ser *)priv;

	/*
	1. SER inprogress: pending msg from others module
	2. SER recovery fail: clr pending event from MDL_SER & msg return failed from others module
	3. SER recovery done: clr pending event & msg return ignor from others module
	4. SER NOT OCCUR: MDL_RET_IGNORE
	*/
	if(cser->bserl2) {
		PHL_ERR("%s: [2] L2 Occured!! From others MDL =%d , EVT_ID=%d\n", __func__,
		MSG_MDL_ID_FIELD(msg->msg_id), MSG_EVT_ID_FIELD(msg->msg_id));
		return MDL_RET_FAIL;
	}
	else if(cser->state) {/* non-CMD_SER_NOT_OCCUR */
		PHL_WARN("%s: [1] Within SER!! From others MDL =%d , EVT_ID=%d\n", __func__,
		MSG_MDL_ID_FIELD(msg->msg_id), MSG_EVT_ID_FIELD(msg->msg_id));
		return MDL_RET_PENDING;
	}
	return MDL_RET_IGNORE;
}


enum phl_mdl_ret_code _ser_hdl_internal_evt_pcie(
	void *dispr, void *priv, struct phl_msg *msg)
{
	struct cmd_ser *cser = (struct cmd_ser *)priv;
	struct phl_info_t *phl_info = cser->phl_info;
	struct phl_hci_trx_ops *ops = phl_info->hci_trx_ops;
	void *drv = phl_to_drvpriv(phl_info);
	enum phl_mdl_ret_code ret = MDL_RET_SUCCESS;

	switch(MSG_EVT_ID_FIELD(msg->msg_id)) {
		case MSG_EVT_SER_EVENT_CHK:
			/* PCIE would notify(interrupt) M1 directly */
			/* ToDo: PCIE polling mode */
			PHL_DBG("PCIE: MSG_EVT_SER_EVENT_CHK \n");
			break;

		case MSG_EVT_SER_M1_PAUSE_TRX: /* M1 */
			PHL_WARN("PCIE: MSG_EVT_SER_M1_PAUSE_TRX \n");
			phl_ps_ser_notify(cser->phl_info, true);
			_ser_set_status(cser, CMD_SER_M1);
			_ser_l1_notify(cser);
			_ser_m1_pause_trx_pcie(cser);
			break;

		case MSG_EVT_SER_IO_TIMER_EXPIRE:
			PHL_INFO("PCIE: MSG_EVT_SER_IO_TIMER_EXPIRE \n");
			if (cser->poll_cnt-- > 0) {
				if(_ser_m1_polling_io_pcie_sdio(cser, false) == MDL_RET_SUCCESS) {

					ops->trx_reset(phl_info);
					_ser_m2_notify(cser); /*set CMD_SER_M2 */
					_ser_m3_m5_waiting(cser);
				}
			} else {
#ifdef RTW_WKARD_SER_L1_EXPIRE

				ops->trx_reset(phl_info);
				_ser_m2_notify(cser); /*set CMD_SER_M2 */
				_ser_m3_m5_waiting(cser);

#else
				_ser_resume_trx_pcie(cser);
				_ser_l2_notify(cser);

				/* ToDo : L2 can't be rescued */
				_ser_reset_status(cser);
#endif
			}
			break;

		case MSG_EVT_SER_FW_TIMER_EXPIRE:
			PHL_WARN("PCIE: MSG_EVT_SER_FW_TIMER_EXPIRE (%d)\n", cser->state);
			/*M3 or M5 doesn't occur */
			if(TEST_STATUS_FLAG(cser->state, CMD_SER_M2)) {
				_ser_resume_trx_pcie(cser);
				_ser_reset_status(cser);
				phl_ps_ser_notify(cser->phl_info, false);
				/* 3. SER recovery done: clr pending event */
				phl_disp_eng_clr_pending_msg(cser->phl_info, HW_BAND_0);
			}
			else if(TEST_STATUS_FLAG(cser->state, CMD_SER_M4)) {
				ops->trx_resume(phl_info, PHL_REQ_PAUSE_TX);
				rtw_phl_tx_req_notify(phl_info);

				_ser_reset_status(cser);
				phl_ps_ser_notify(cser->phl_info, false);
				/* 3. SER recovery done: clr pending event */
				phl_disp_eng_clr_pending_msg(cser->phl_info, HW_BAND_0);
			}
			break;

		case MSG_EVT_SER_M3_DO_RECOV: /* M3 */ /*SER_ST_L1_DO_HCI*/
			PHL_INFO("PCIE: MSG_EVT_SER_M3_DO_RECOV \n");
			_os_cancel_timer(drv, &cser->poll_timer);
			_ser_clear_status(cser, CMD_SER_M2);
			_ser_set_status(cser, CMD_SER_M3);
			if(rtw_hal_lv1_rcvy(phl_info->hal, RTW_PHL_SER_LV1_SER_RCVY_STEP_2) != 0){
				_ser_resume_trx_pcie(cser);

				_ser_l2_notify(cser);
				_ser_reset_status(cser);
				break;
			}
			ops->trx_resume(phl_info, PHL_REQ_PAUSE_RX);
			rtw_phl_start_rx_process(phl_info);

			/* send M4 event and wait M5*/
			_ser_m4_notify(cser);
			_ser_m3_m5_waiting(cser);
			break;

		case MSG_EVT_SER_M5_READY: /* M5 */ /* SER_ST_L1_RESUME_TRX*/
			PHL_INFO("PCIE: MSG_EVT_SER_M5_READY \n");
			/* TODO resume TRX process */
			_os_cancel_timer(drv, &cser->poll_timer);
			_ser_clear_status(cser, CMD_SER_M4);
			_ser_set_status(cser, CMD_SER_M5);
			ops->trx_resume(phl_info, PHL_REQ_PAUSE_TX);
			rtw_phl_tx_req_notify(phl_info);

			_ser_reset_status(cser);
			phl_ps_ser_notify(cser->phl_info, false);
			/* 3. SER recovery done: clr pending event */
			phl_disp_eng_clr_pending_msg(cser->phl_info, HW_BAND_0);
			break;

		case MSG_EVT_SER_M9_L2_RESET: /* M9 */

			PHL_WARN("PCIE: MSG_EVT_SER_M9_L2_RESET \n");
			_os_cancel_timer(drv, &cser->poll_timer);

			if(cser->state > CMD_SER_NOT_OCCUR) {
				if(TEST_STATUS_FLAG(cser->state, CMD_SER_M1) ||
				   TEST_STATUS_FLAG(cser->state, CMD_SER_M2)) {
					/* M1 ~ M2 */
					_ser_resume_trx_pcie(cser);
				} else {
					/* M3 ~ M5 */
					ops->trx_resume(phl_info, PHL_REQ_PAUSE_TX);
					rtw_phl_tx_req_notify(phl_info);
				}
			}
			_ser_set_status(cser, CMD_SER_M9);
			_ser_l2_notify(cser);

			/* Reset SER state*/
			_ser_reset_status(cser);
			break;
	}

	return ret;
}

enum phl_mdl_ret_code _ser_hdl_internal_evt_usb(
	void *dispr, void *priv, struct phl_msg *msg)
{
	struct cmd_ser *cser = (struct cmd_ser *)priv;
	struct phl_info_t *phl_info = cser->phl_info;
	struct phl_hci_trx_ops *ops = phl_info->hci_trx_ops;
	void *drv = phl_to_drvpriv(phl_info);
	enum phl_mdl_ret_code ret = MDL_RET_SUCCESS;

	switch(MSG_EVT_ID_FIELD(msg->msg_id)) {
		case MSG_EVT_SER_EVENT_CHK: /* SER_EV_POLL_USB_INT_EXPIRE */
			PHL_DBG("USB: MSG_EVT_SER_EVENT_CHK \n");
			if (true == rtw_hal_recognize_interrupt(phl_info->hal))
				rtw_phl_interrupt_handler(phl_info);

			cser->poll_cnt--; /*poll_cnt might be negative in CMD_SER_NOT_OCCUR*/
			_os_set_timer(drv, &cser->poll_timer, CMD_SER_USB_POLLING_INTERVAL_IDL); //1000ms

			break;

		case MSG_EVT_SER_M1_PAUSE_TRX: /* M1 */
			PHL_WARN("USB: MSG_EVT_SER_M1_PAUSE_TRX \n");
			phl_ps_ser_notify(cser->phl_info, true);
			_os_cancel_timer(drv, &cser->poll_timer);
			_ser_set_status(cser, CMD_SER_M1);
			_ser_l1_notify(cser);
			_ser_m1_pause_trx_usb(cser);
			break;

		case MSG_EVT_SER_IO_TIMER_EXPIRE:
			PHL_INFO("USB: MSG_EVT_SER_IO_TIMER_EXPIRE \n");
			if(TEST_STATUS_FLAG(cser->state, CMD_SER_M1)) {
				if (cser->poll_cnt-- > 0) {
					if(_ser_m1_polling_io_usb(cser, false) == MDL_RET_SUCCESS) {
						rtw_hal_lv1_rcvy(phl_info->hal, RTW_PHL_SER_LV1_RCVY_STEP_1);

						_ser_m2_notify(cser); /*set CMD_SER_M2 */
						_ser_m3_m5_waiting(cser);
					}
				} else {
#ifdef RTW_WKARD_SER_L1_EXPIRE

					rtw_hal_lv1_rcvy(phl_info->hal, RTW_PHL_SER_LV1_RCVY_STEP_1);

					_ser_m2_notify(cser); /*set CMD_SER_M2 */
					_ser_m3_m5_waiting(cser);
#else
					_ser_l2_notify(cser);

					/* Trigger polling timer */
					_ser_reset_status(cser);
					_ser_polling_event(cser);
#endif
				}
			}
			break;

		case MSG_EVT_SER_FW_TIMER_EXPIRE:
			PHL_WARN("USB: MSG_EVT_SER_FW_TIMER_EXPIRE \n");
			/*M3 or M5 doesn't occur */
			if(TEST_STATUS_FLAG(cser->state, CMD_SER_M2)) {
				u32 mac_err = 0;
				mac_err = rtw_hal_lv1_rcvy(phl_info->hal, RTW_PHL_SER_LV1_SER_RCVY_STEP_2);
				_ser_resume_trx_usb(cser);

				if(mac_err != 0) {
					_ser_l2_notify(cser);
					_ser_reset_status(cser);
				} else {
					_ser_reset_status(cser);
					phl_ps_ser_notify(cser->phl_info, false);
					/* 3. SER recovery done: clr pending event */
					phl_disp_eng_clr_pending_msg(cser->phl_info, HW_BAND_0);
				}
			}
			else if(TEST_STATUS_FLAG(cser->state, CMD_SER_M4)) {
				ops->trx_resume(phl_info, PHL_REQ_PAUSE_TX);
				rtw_phl_tx_req_notify(phl_info);

				_ser_reset_status(cser);
				phl_ps_ser_notify(cser->phl_info, false);
				/* 3. SER recovery done: clr pending event */
				phl_disp_eng_clr_pending_msg(cser->phl_info, HW_BAND_0);
			}

			/* Trigger polling timer */
			_ser_polling_event(cser);
			break;

		case MSG_EVT_SER_M3_DO_RECOV:	/*SER_ST_L1_DO_HCI*/
			PHL_INFO("USB: MSG_EVT_SER_M3_DO_RECOV \n");
			_os_cancel_timer(drv, &cser->poll_timer);
			_ser_clear_status(cser, CMD_SER_M2);
			_ser_set_status(cser, CMD_SER_M3);
			if (rtw_hal_lv1_rcvy(phl_info->hal, RTW_PHL_SER_LV1_SER_RCVY_STEP_2) != 0) {
				_ser_l2_notify(cser);

				/* Trigger polling timer */
				_ser_reset_status(cser);
				_ser_polling_event(cser);
				break;
			}
			ops->trx_resume(phl_info, PHL_REQ_PAUSE_RX);
			rtw_phl_start_rx_process(phl_info);

			/* send M4 event */
			_ser_m4_notify(cser);

			/* set timeout to wait M5 */
			_ser_m3_m5_waiting(cser);
			break;

		case MSG_EVT_SER_M5_READY: /* M5 */ /* SER_ST_L1_RESUME_TRX*/
			/* TODO resume TRX process */
			PHL_INFO("USB: MSG_EVT_SER_M5_READY \n");
			_os_cancel_timer(drv, &cser->poll_timer);
			_ser_clear_status(cser, CMD_SER_M4);
			_ser_set_status(cser, CMD_SER_M5);
			ops->trx_resume(phl_info, PHL_REQ_PAUSE_TX);
			rtw_phl_tx_req_notify(phl_info);

			_ser_reset_status(cser);
			phl_ps_ser_notify(cser->phl_info, false);
			/* 3. SER recovery done: clr pending event */
			phl_disp_eng_clr_pending_msg(cser->phl_info, HW_BAND_0);
			/* Trigger polling timer */
			_ser_polling_event(cser);
			break;

		case MSG_EVT_SER_M9_L2_RESET:

			PHL_WARN("USB: MSG_EVT_SER_M9_L2_RESET \n");
			_os_cancel_timer(drv, &cser->poll_timer);

			if(cser->state > CMD_SER_NOT_OCCUR) {
				if(TEST_STATUS_FLAG(cser->state, CMD_SER_M1) ||
				   TEST_STATUS_FLAG(cser->state, CMD_SER_M2)) {
					/* M1 ~ M2 */
					_ser_resume_trx_usb(cser);
				}
			}
			_ser_set_status(cser, CMD_SER_M9);
			_ser_l2_notify(cser);

			/* Trigger polling timer */
			_ser_reset_status(cser);
			_ser_polling_event(cser);
			break;
	}

	return ret;
}


enum phl_mdl_ret_code _ser_hdl_internal_evt_sdio(
	void *dispr, void *priv, struct phl_msg *msg)
{
	struct cmd_ser *cser = (struct cmd_ser *)priv;
	struct phl_info_t *phl_info = cser->phl_info;
	struct phl_hci_trx_ops *ops = phl_info->hci_trx_ops;
	void *drv = phl_to_drvpriv(phl_info);
	enum phl_mdl_ret_code ret = MDL_RET_SUCCESS;
	u8 notify = 0;

	switch(MSG_EVT_ID_FIELD(msg->msg_id)) {
		case MSG_EVT_SER_EVENT_CHK: /* from interrupt hdlr - phl_ser_event_check */
			PHL_DBG("SDIO: MSG_EVT_SER_EVENT_CHK \n");
			if (true == rtw_hal_recognize_halt_c2h_interrupt(phl_info->hal)) {
				phl_ser_event_notify(phl_info, &notify);
				if ((notify == RTW_PHL_SER_L0_RESET) || (notify == RTW_PHL_SER_L2_RESET))
					rtw_hal_config_interrupt(phl_info->hal, RTW_PHL_EN_HCI_INT);
			}
			else {
				rtw_hal_config_interrupt(phl_info->hal, RTW_PHL_EN_HCI_INT);
			}
			break;

		case MSG_EVT_SER_M1_PAUSE_TRX: /* M1 */
			PHL_WARN("SDIO: MSG_EVT_SER_M1_PAUSE_TRX \n");
			phl_ps_ser_notify(cser->phl_info, true);
			_ser_set_status(cser, CMD_SER_M1);
			_ser_l1_notify(cser);
			_ser_m1_pause_trx_sdio(cser);
			break;

		case MSG_EVT_SER_IO_TIMER_EXPIRE:
			PHL_INFO("SDIO: MSG_EVT_SER_IO_TIMER_EXPIRE \n");
			if (cser->poll_cnt-- > 0) {
				if(_ser_m1_polling_io_pcie_sdio(cser, false) == MDL_RET_SUCCESS) {
					rtw_hal_config_interrupt(phl_info->hal, RTW_PHL_DIS_HCI_INT);

					if (rtw_hal_lv1_rcvy(phl_info->hal, RTW_PHL_SER_LV1_RCVY_STEP_1) != 0) {

						rtw_hal_config_interrupt(phl_info->hal, RTW_PHL_EN_HCI_INT);
						_ser_l2_notify(cser);

						_ser_reset_status(cser);
						break;
					}
				}

				rtw_hal_config_interrupt(phl_info->hal, RTW_PHL_SER_HANDSHAKE_MODE);

				_ser_m2_notify(cser); /*set CMD_SER_M2 */
				_ser_m3_m5_waiting(cser);
			}
			else {
#ifdef RTW_WKARD_SER_L1_EXPIRE

				if (rtw_hal_lv1_rcvy(phl_info->hal, RTW_PHL_SER_LV1_RCVY_STEP_1) != 0) {

					rtw_hal_config_interrupt(phl_info->hal, RTW_PHL_EN_HCI_INT);
					_ser_l2_notify(cser);

					_ser_reset_status(cser);
					break;
				}

				rtw_hal_config_interrupt(phl_info->hal, RTW_PHL_SER_HANDSHAKE_MODE);

				_ser_m2_notify(cser); /*set CMD_SER_M2 */
				_ser_m3_m5_waiting(cser);

#else
				_ser_resume_trx_sdio(cser);
				_ser_l2_notify(cser);

				_ser_reset_status(cser);
#endif
				break;
			}
			break;

		case MSG_EVT_SER_FW_TIMER_EXPIRE:
			PHL_WARN("SDIO: MSG_EVT_SER_FW_TIMER_EXPIRE \n");
			/*M3 or M5 doesn't occur */
			if(TEST_STATUS_FLAG(cser->state, CMD_SER_M2)) {
				u32 mac_err = 0;
				mac_err = rtw_hal_lv1_rcvy(phl_info->hal, RTW_PHL_SER_LV1_SER_RCVY_STEP_2);
				_ser_resume_trx_sdio(cser);

				if (mac_err != 0) {
					_ser_l2_notify(cser);
					_ser_reset_status(cser);
				} else {
					_ser_reset_status(cser);
					phl_ps_ser_notify(cser->phl_info, false);
					/* 3. SER recovery done: clr pending event */
					phl_disp_eng_clr_pending_msg(cser->phl_info, HW_BAND_0);
				}
			}
			else if(TEST_STATUS_FLAG(cser->state, CMD_SER_M4)) {
				ops->trx_resume(phl_info, PHL_REQ_PAUSE_TX);
				rtw_phl_tx_req_notify(phl_info);
				rtw_hal_config_interrupt(phl_info->hal, RTW_PHL_EN_HCI_INT);

				_ser_reset_status(cser);
				phl_ps_ser_notify(cser->phl_info, false);
				/* 3. SER recovery done: clr pending event */
				phl_disp_eng_clr_pending_msg(cser->phl_info, HW_BAND_0);
			}

			break;

		case MSG_EVT_SER_M3_DO_RECOV:	/*SER_ST_L1_DO_HCI*/
			PHL_INFO("SDIO: MSG_EVT_SER_M3_DO_RECOV \n");
			_os_cancel_timer(drv, &cser->poll_timer);
			_ser_clear_status(cser, CMD_SER_M2);
			_ser_set_status(cser, CMD_SER_M3);
			rtw_hal_clear_interrupt(phl_info->hal);

			if (rtw_hal_lv1_rcvy(phl_info->hal, RTW_PHL_SER_LV1_SER_RCVY_STEP_2) != 0) {
				rtw_hal_config_interrupt(phl_info->hal, RTW_PHL_EN_HCI_INT);

				_ser_l2_notify(cser);

				_ser_reset_status(cser);
				break;
			}
			ops->trx_resume(phl_info, PHL_REQ_PAUSE_RX);
			rtw_phl_start_rx_process(phl_info);

			rtw_hal_config_interrupt(phl_info->hal, RTW_PHL_SER_HANDSHAKE_MODE);

			/* send M4 event */
			_ser_m4_notify(cser);

			/* set timeout to wait M5 */
			_ser_m3_m5_waiting(cser);
			break;

		case MSG_EVT_SER_M5_READY:	/* SER_ST_L1_RESUME_TRX*/
			PHL_INFO("SDIO: MSG_EVT_SER_M5_READY \n");
			_os_cancel_timer(drv, &cser->poll_timer);
			_ser_clear_status(cser, CMD_SER_M4);
			_ser_set_status(cser, CMD_SER_M5);
			rtw_hal_config_interrupt(phl_info->hal, RTW_PHL_EN_HCI_INT);
			/* TODO resume TRX process */
			ops->trx_resume(phl_info, PHL_REQ_PAUSE_TX);
			rtw_phl_tx_req_notify(phl_info);

			_ser_reset_status(cser);
			phl_ps_ser_notify(cser->phl_info, false);
			/* 3. SER recovery done: clr pending event */
			phl_disp_eng_clr_pending_msg(cser->phl_info, HW_BAND_0);
			break;

		case MSG_EVT_SER_M9_L2_RESET:

			PHL_WARN("SDIO: MSG_EVT_SER_M9_L2_RESET \n");
			_os_cancel_timer(drv, &cser->poll_timer);
			if(cser->state > CMD_SER_NOT_OCCUR) {
				if(TEST_STATUS_FLAG(cser->state, CMD_SER_M1) ||
				   TEST_STATUS_FLAG(cser->state, CMD_SER_M2)) {
					/* M1 ~ M2 */
					_ser_resume_trx_sdio(cser);
				} else {
					/* M3 ~ M5 */
					rtw_hal_config_interrupt(phl_info->hal, RTW_PHL_EN_HCI_INT);
				}
			}
			_ser_set_status(cser, CMD_SER_M9);
			_ser_l2_notify(cser);

			/* Reset SER state*/
			_ser_reset_status(cser);
			break;
		}

	return ret;
}

enum phl_mdl_ret_code _ser_hdl_internal_evt(
	void *dispr, void *priv, struct phl_msg *msg)
{
	struct cmd_ser *cser = (struct cmd_ser *)priv;
	struct phl_info_t *phl_info = cser->phl_info;
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;

	switch(MSG_EVT_ID_FIELD(msg->msg_id)) {
		case RTW_PHL_SER_L0_RESET: /* L0 notify only */
			PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "[CMD_SER]: Get L0 reset notify\n");
			ret = MDL_RET_SUCCESS;
			break;

		default:
			if (phl_info->phl_com->hci_type == RTW_HCI_PCIE)
				ret = _ser_hdl_internal_evt_pcie(dispr, priv, msg);
			else if (phl_info->phl_com->hci_type == RTW_HCI_USB)
				ret = _ser_hdl_internal_evt_usb(dispr, priv, msg);
			else if (phl_info->phl_com->hci_type == RTW_HCI_SDIO)
				ret = _ser_hdl_internal_evt_sdio(dispr, priv, msg);
			break;
	}

	return ret;
}

static enum phl_mdl_ret_code
_phl_ser_mdl_init(void *phl, void *dispr, void **priv)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv = phl_to_drvpriv(phl_info);
	struct cmd_ser *cser = NULL;
	u8	idx = 0;

	FUNCIN();
	if (priv == NULL)
		return MDL_RET_FAIL;

	(*priv) = NULL;
	cser = (struct cmd_ser *)_os_mem_alloc(drv,
					       sizeof(struct cmd_ser));
	if (cser == NULL) {
		PHL_ERR(" %s, alloc fail\n",__FUNCTION__);
		return MDL_RET_FAIL;
	}

	_os_mem_set(drv, cser, 0, sizeof(struct cmd_ser));
	_os_spinlock_init(drv, &cser->_lock);
	_os_init_timer(drv, &cser->poll_timer, _ser_poll_timer_cb,
		       cser, "cmd_ser_poll_timer");

	INIT_LIST_HEAD(&cser->stslist.queue);
	for(idx =0; idx < CMD_SER_LOG_SIZE; idx++)
	{
		INIT_LIST_HEAD(&cser->stsl2[idx].list);
		cser->stsl2[idx].idx = idx;
		pq_push(drv, &cser->stslist, &cser->stsl2[idx].list, _tail, _ps);
	}

	cser->phl_info = phl_info;
	cser->dispr = dispr;
	(*priv) = (void*)cser;

	if (phl_info->phl_com->hci_type == RTW_HCI_PCIE)
		cser->evtsrc = CMD_SER_SRC_INT;
	else if (phl_info->phl_com->hci_type == RTW_HCI_USB)
		cser->evtsrc = CMD_SER_SRC_POLL;
	else if (phl_info->phl_com->hci_type == RTW_HCI_SDIO)
		cser->evtsrc = CMD_SER_SRC_INT;

	PHL_INFO("%s:: cser->evtsrc = %d\n",
		__func__, cser->evtsrc);
	FUNCOUT();

	return MDL_RET_SUCCESS;
}

static void
_phl_ser_mdl_deinit(void *dispr, void *priv)
{
	struct cmd_ser *cser = (struct cmd_ser *)priv;
	void *drv = phl_to_drvpriv(cser->phl_info);

	FUNCIN();

	_os_cancel_timer(drv, &cser->poll_timer);
	_os_release_timer(drv, &cser->poll_timer);
	_os_spinlock_free(drv, &cser->_lock);
	_os_mem_free(drv, cser, sizeof(struct cmd_ser));
	PHL_INFO(" %s\n", __FUNCTION__);
}

static enum phl_mdl_ret_code
_phl_ser_mdl_start(void *dispr, void *priv)
{
	struct cmd_ser *cser = (struct cmd_ser *)priv;
	struct phl_info_t *phl_info = cser->phl_info;
	void *drv = phl_to_drvpriv(phl_info);

	_ser_reset_status(cser);
	cser->poll_cnt = 0;

	if (phl_info->phl_com->hci_type == RTW_HCI_USB) {
		/* Disable L0 Reset Notify from FW to driver */
		rtw_hal_ser_set_error_status(phl_info->hal,RTW_PHL_SER_L0_CFG_DIS_NOTIFY);
		_os_set_timer(drv, &cser->poll_timer, CMD_SER_USB_POLLING_INTERVAL_IDL);
	}

	return MDL_RET_SUCCESS;
}

static enum phl_mdl_ret_code
_phl_ser_mdl_stop(void *dispr, void *priv)
{
	struct cmd_ser *cser = (struct cmd_ser *)priv;
	void *drv = phl_to_drvpriv(cser->phl_info);

	_os_cancel_timer(drv, &cser->poll_timer);

	return MDL_RET_SUCCESS;
}

static enum phl_mdl_ret_code
_phl_ser_mdl_msg_hdlr(void *dispr, void *priv,
				struct phl_msg *msg)
{
	enum phl_mdl_ret_code ret = MDL_RET_IGNORE;

	if(IS_MSG_FAIL(msg->msg_id)) {

		PHL_INFO("%s :: MSG(%d)_FAIL - EVT_ID=%d \n", __func__,
		         MSG_MDL_ID_FIELD(msg->msg_id), MSG_EVT_ID_FIELD(msg->msg_id));

		return _ser_fail_ev_hdlr(dispr, priv, msg);
	}

	switch(MSG_MDL_ID_FIELD(msg->msg_id)) {
		case PHL_MDL_SER:
			if (IS_MSG_IN_PRE_PHASE(msg->msg_id))
				ret = _ser_hdl_internal_evt(dispr, priv, msg);
			break;

		default:
			ret = _ser_hdl_external_evt(dispr, priv, msg);
			break;
	}

	return ret;
}

static enum phl_mdl_ret_code
_phl_ser_mdl_set_info(void *dispr, void *priv,
		      struct phl_module_op_info *info)
{
	struct cmd_ser *cser = (struct cmd_ser *)priv;
	void *drv = phl_to_drvpriv(cser->phl_info);
	enum phl_mdl_ret_code ret = MDL_RET_IGNORE;
	/* PHL_INFO(" %s :: info->op_code=%d \n", __func__, info->op_code); */

	switch(info->op_code) {
		case BK_MODL_OP_INPUT_CMD:
			if(info->inbuf) {
				cser->ser_l2_hdlr=(u8 (*)(void *))info->inbuf;
			}
			ret = MDL_RET_SUCCESS;
			break;
	}

	return ret;
}

static enum phl_mdl_ret_code
_phl_ser_mdl_query_info(void *dispr, void *priv,
			struct phl_module_op_info *info)
{
	struct cmd_ser *cser = (struct cmd_ser *)priv;
	void *drv = phl_to_drvpriv(cser->phl_info);
	enum phl_mdl_ret_code ret = MDL_RET_IGNORE;
	/* PHL_INFO(" %s :: info->op_code=%d \n", __func__, info->op_code); */

	switch(info->op_code) {
		case BK_MODL_OP_STATE:
			_os_mem_cpy(drv, (void*)info->outbuf, &cser->state, 1);
			ret = MDL_RET_SUCCESS;
			break;
	}
	return ret;
}

static struct phl_bk_module_ops ser_ops= {
	.init = _phl_ser_mdl_init,
	.deinit = _phl_ser_mdl_deinit,
	.start = _phl_ser_mdl_start,
	.stop = _phl_ser_mdl_stop,
	.msg_hdlr = _phl_ser_mdl_msg_hdlr,
	.set_info = _phl_ser_mdl_set_info,
	.query_info = _phl_ser_mdl_query_info,
};


enum rtw_phl_status
phl_register_ser_module(struct phl_info_t *phl_info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);

	phl_status = phl_disp_eng_register_module(phl_info, HW_BAND_0,
						  PHL_MDL_SER,
						  &ser_ops);
	if (RTW_PHL_STATUS_SUCCESS != phl_status) {
		PHL_ERR("%s register SER module in cmd disp failed! \n", __func__);
	}

	return phl_status;
}

#ifdef CONFIG_PHL_CMD_SER
u8 phl_ser_inprogress(void *phl) {

	struct phl_module_op_info op_info = {0};
	u8 state = 0;

	op_info.op_code = BK_MODL_OP_STATE;
	op_info.inbuf = (u8*)&state;
	op_info.inlen = 1;

	if( rtw_phl_query_bk_module_info(phl, HW_BAND_0, PHL_MDL_SER,
					 &op_info) == RTW_PHL_STATUS_SUCCESS) {

		if(state) /* non-CMD_SER_NOT_OCCUR */
			return true;
	}
	return false;
}

enum rtw_phl_status phl_ser_send_msg(void *phl,
	enum RTW_PHL_SER_NOTIFY_EVENT notify)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	struct phl_msg nextmsg = {0};
	struct phl_msg_attribute attr = {0};
	u16 event = 0;
	void *dispr = NULL;

	switch (notify) {
	case RTW_PHL_SER_PAUSE_TRX: /* M1 */
		event = MSG_EVT_SER_M1_PAUSE_TRX;
		break;
	case RTW_PHL_SER_DO_RECOVERY: /* M3 */
		event = MSG_EVT_SER_M3_DO_RECOV;
		break;
	case RTW_PHL_SER_READY: /* M5 */
		event = MSG_EVT_SER_M5_READY;
		break;
	case RTW_PHL_SER_L2_RESET: /* M9 */
		event = MSG_EVT_SER_M9_L2_RESET;
		break;
	case RTW_PHL_SER_EVENT_CHK:
		event = MSG_EVT_SER_EVENT_CHK;
		break;
	case RTW_PHL_SER_L0_RESET:
	default:
		PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "phl_ser_send_msg(): Unsupported case:%d, please check it\n",
			  notify);
		return RTW_PHL_STATUS_FAILURE;
	}
	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "phl_ser_send_msg  event  %d\n", event);


	SET_MSG_MDL_ID_FIELD(nextmsg.msg_id, PHL_MDL_SER);
	SET_MSG_EVT_ID_FIELD(nextmsg.msg_id, event);
	nextmsg.band_idx = HW_BAND_0;

	phl_status = rtw_phl_send_msg_to_dispr(phl,
					       &nextmsg,
					       &attr,
					       NULL);

	if(phl_status != RTW_PHL_STATUS_SUCCESS){
		PHL_ERR("[CMD_SER] send_msg_to_dispr fail! (%d)\n", event);
	}

	return phl_status;
}
#endif


#endif // NEO mark off first

#ifndef CONFIG_FSM

#if 0 // NEO mark off first

/* The same as phl_fw_watchdog_timeout_notify of fsm-ser */
enum rtw_phl_status phl_fw_watchdog_timeout_notify(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	struct phl_msg msg = {0};

	PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_, "phl_fw_watchdog_timeout_notify !!!\n");

	SET_MSG_MDL_ID_FIELD(msg.msg_id, PHL_MDL_PHY_MGNT);
	SET_MSG_EVT_ID_FIELD(msg.msg_id, MSG_EVT_DUMP_PLE_BUFFER);
	phl_msg_hub_send(phl_info, NULL, &msg);

	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status phl_ser_event_notify(void *phl, u8 *p_ntfy)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum RTW_PHL_SER_NOTIFY_EVENT notify = RTW_PHL_SER_L2_RESET;
	struct phl_msg msg = {0};
	u32 err = 0;

	notify = rtw_hal_ser_get_error_status(phl_info->hal, &err);

	if (p_ntfy != NULL)
		*p_ntfy = notify;

	phl_info->phl_com->phl_stats.ser_event[notify]++;

	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "phl_ser_event_notify, error 0x%x, notify 0x%x\n", err, notify);

	if (notify == RTW_PHL_SER_L0_RESET) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "phl_ser_event_notify, hit L0 Reset\n");
		return RTW_PHL_STATUS_SUCCESS;
	}

	if (notify == RTW_PHL_SER_LOG_ONLY) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "phl_ser_event_notify, RTW_PHL_SER_LOG_ONLY\n");
		return RTW_PHL_STATUS_SUCCESS;
	}

	if (notify == RTW_PHL_SER_DUMP_FW_LOG) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "phl_ser_event_notify, RTW_PHL_SER_DUMP_FW_LOG\n");

		SET_MSG_MDL_ID_FIELD(msg.msg_id, PHL_MDL_PHY_MGNT);
		SET_MSG_EVT_ID_FIELD(msg.msg_id, MSG_EVT_DUMP_PLE_BUFFER);
		phl_msg_hub_send(phl_info, NULL, &msg);

		return RTW_PHL_STATUS_SUCCESS;
	}

	return phl_ser_send_msg(phl, notify);
}

#endif // NEO : mark off first

/* The same as rtw_phl_ser_dump_ple_buffer of fsm-ser */
enum rtw_phl_status rtw_phl_ser_dump_ple_buffer(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "rtw_phl_ser_dump_ple_buffer\n");

	// NEO
	//rtw_hal_dump_fw_rsvd_ple(phl_info->hal);

	return RTW_PHL_STATUS_SUCCESS;
}

#endif /*#ifndef CONFIG_FSM*/

