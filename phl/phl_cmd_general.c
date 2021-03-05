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
#define _PHL_CMD_GENERAL_C_
#include "phl_headers.h"

#ifdef CONFIG_CMD_DISP

enum phl_cmd_sts {
	PHL_CMD_SUBMITTED = -1,
	PHL_CMD_DONE_SUCCESS = 0,
	PHL_CMD_DONE_TIMEOUT,
	PHL_CMD_DONE_CMD_ERROR,
	PHL_CMD_DONE_CMD_DROP,
	PHL_CMD_DONE_CANNOT_IO,
	PHL_CMD_DONE_UNKNOWN,
};

struct phl_sync {
	u32 submit_time;
	u32 timeout_ms; /* 0: wait forever, >0: up to ms waiting */
	enum phl_cmd_sts status; /* status for operation */
	_os_event done;
};

struct phl_cmd_sync {
	enum phl_msg_evt_id evt_id;
	struct phl_sync sync;
	_os_lock lock;
};

struct phl_cmd_obj {
	enum phl_msg_evt_id evt_id; /* u8 id */
	u8 *buf;
	u32 buf_len;
	bool no_io;
	void (*cmd_complete)(void* priv, u8 *buf, u32 buf_len, enum rtw_phl_status status);
	/*cmd sync*/
	bool is_cmd_wait;
	_os_atomic ref_cnt;
	struct phl_cmd_sync cmd_sync;
};

#define DBG_CMD_SYNC

#ifdef DBG_CMD_SYNC
static void _phl_cmd_sync_dump(struct phl_cmd_sync *cmd_sync, const char *caller)
{
	PHL_INFO("[CMD_SYNC] %s\n", caller);
	PHL_INFO("[CMD_SYNC] evt_id:%d status:%d\n", cmd_sync->evt_id, cmd_sync->sync.status);
	PHL_INFO("[CMD_SYNC] take:%d ms\n", phl_get_passing_time_ms(cmd_sync->sync.submit_time));
}
#endif

static void _phl_cmd_sync_init(struct phl_info_t *phl_info,
		enum phl_msg_evt_id evt_id,
		struct phl_cmd_sync *cmd_sync, u32 timeout_ms)
{
	void *drv = phl_to_drvpriv(phl_info);

	_os_spinlock_init(drv, &cmd_sync->lock);
	cmd_sync->evt_id = evt_id;
	cmd_sync->sync.timeout_ms = timeout_ms;
	cmd_sync->sync.submit_time = _os_get_cur_time_ms();
	_os_event_init(drv, &(cmd_sync->sync.done));
	cmd_sync->sync.status = PHL_CMD_SUBMITTED;
}

static void _phl_cmd_sync_deinit(struct phl_info_t *phl_info,
			struct phl_cmd_sync *cmd_sync)
{
	#ifdef DBG_CMD_SYNC
	_phl_cmd_sync_dump(cmd_sync, __func__);
	#endif
	_os_spinlock_free(phl_to_drvpriv(phl_info), &cmd_sync->lock);
}

inline static enum rtw_phl_status _cmd_stat_2_phl_stat(enum phl_cmd_sts status)
{
	if (status == PHL_CMD_DONE_TIMEOUT)
		return RTW_PHL_STATUS_CMD_TIMEOUT;
	else if(status == PHL_CMD_DONE_CANNOT_IO)
		return RTW_PHL_STATUS_CMD_CANNOT_IO;
	else if (status == PHL_CMD_DONE_CMD_ERROR)
		return RTW_PHL_STATUS_CMD_ERROR;
	else if (status == PHL_CMD_DONE_CMD_DROP)
		return RTW_PHL_STATUS_CMD_DROP;
	else
		return RTW_PHL_STATUS_CMD_SUCCESS;
}

static enum rtw_phl_status
_phl_cmd_wait(struct phl_info_t *phl_info, struct phl_cmd_sync *cmd_sync)
{
	void *drv = phl_to_drvpriv(phl_info);
	u32 cmd_wait_ms = cmd_sync->sync.timeout_ms;/*0: wait forever, >0: up to ms waiting*/

	#ifdef DBG_CMD_SYNC
	PHL_INFO("evt_id:%d %s in...............\n", cmd_sync->evt_id, __func__);
	#endif

	if (_os_event_wait(drv, &cmd_sync->sync.done, cmd_wait_ms) == 0) {
		_os_spinlock(drv, &cmd_sync->lock, _bh, NULL);
		cmd_sync->sync.status = PHL_CMD_DONE_TIMEOUT;
		_os_spinunlock(drv, &cmd_sync->lock, _bh, NULL);
		PHL_ERR("%s evt_id:%d timeout\n", __func__, cmd_sync->evt_id);
	}
	#ifdef DBG_CMD_SYNC
	PHL_INFO("evt_id:%d %s out...............\n", cmd_sync->evt_id, __func__);
	_phl_cmd_sync_dump(cmd_sync, __func__);
	#endif
	return _cmd_stat_2_phl_stat(cmd_sync->sync.status);
}

static bool _phl_cmd_chk_wating_status(enum phl_cmd_sts status)
{
	switch (status) {
	case PHL_CMD_SUBMITTED:
		/* fall through */
	case PHL_CMD_DONE_UNKNOWN:
		return true;
	default:
		return false;
	}
}

static void _phl_cmd_done(struct phl_info_t *phl_info,
		struct phl_cmd_sync *cmd_sync, enum phl_cmd_sts status)
{
	void *drv = phl_to_drvpriv(phl_info);

	if (!cmd_sync) {
		PHL_ERR("%s cmd_sync is NULL\n", __func__);
		_os_warn_on(1);
		return;
	}
	#ifdef DBG_CMD_SYNC
	PHL_INFO("evt_id:%d %s in...............\n", cmd_sync->evt_id, __func__);
	#endif
	_os_spinlock(drv, &cmd_sync->lock, _bh, NULL);
	if (_phl_cmd_chk_wating_status(cmd_sync->sync.status)) {
		PHL_INFO("%s status:%d\n", __func__, status);
		cmd_sync->sync.status = status;
	}
	_os_spinunlock(drv, &cmd_sync->lock, _bh, NULL);

	_os_event_set(drv, &cmd_sync->sync.done);
	#ifdef DBG_CMD_SYNC
	PHL_INFO("evt_id:%d %s out...............\n", cmd_sync->evt_id, __func__);
	_phl_cmd_sync_dump(cmd_sync, __func__);
	#endif
}
/********************************************************/
static enum rtw_phl_status
_phl_cmd_general_pre_phase_msg_hdlr(struct phl_info_t *phl_info, void *dispr,
				    struct phl_msg *msg)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	enum phl_msg_evt_id evt_id = MSG_EVT_ID_FIELD(msg->msg_id);

	switch (evt_id) {

#if 0 //NEO
	case MSG_EVT_PCIE_TRX_MIT:
		phl_evt_pcie_trx_mit_hdlr(phl_info, msg);
		break;
#endif

	default:
		break;
	}

	if (RTW_PHL_STATUS_SUCCESS != status)
		return RTW_PHL_STATUS_FAILURE;

	return RTW_PHL_STATUS_SUCCESS;
}

static enum rtw_phl_status
_phl_cmd_general_post_phase_msg_hdlr(struct phl_info_t *phl_info, void *dispr,
				     struct phl_msg *msg)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	enum phl_msg_evt_id evt_id = MSG_EVT_ID_FIELD(msg->msg_id);

	switch (evt_id) {

	case MSG_EVT_NONE:
		break;

	default:
		break;
	}

	if (RTW_PHL_STATUS_SUCCESS != status)
		return RTW_PHL_STATUS_FAILURE;

	return RTW_PHL_STATUS_SUCCESS;
}

static enum phl_mdl_ret_code _phl_cmd_general_init(void *phl, void *dispr,
						   void **priv)
{
	*priv = phl;
	return MDL_RET_SUCCESS;
}

static void _phl_cmd_general_deinit(void *dispr, void *priv) {}

static enum phl_mdl_ret_code _phl_cmd_general_start(void *dispr, void *priv)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)priv;
	u8 dispr_idx = 0;

	if (RTW_PHL_STATUS_SUCCESS != phl_dispr_get_idx(dispr, &dispr_idx))
		return MDL_RET_FAIL;

	//NEO
	#if 0
	if (RTW_PHL_STATUS_SUCCESS !=
	    phl_pcie_trx_mit_start(phl_info, dispr_idx))
		return MDL_RET_FAIL;
	#endif

	return MDL_RET_SUCCESS;
}

static enum phl_mdl_ret_code _phl_cmd_general_stop(void *dispr, void *priv)
{
	return MDL_RET_SUCCESS;
}

static enum phl_mdl_ret_code _phl_cmd_general_msg_hdlr(void *dispr, void *priv,
						       struct phl_msg *msg)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)priv;
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;

	if (IS_MSG_FAIL(msg->msg_id)) {

		PHL_INFO("%s :: MSG(%d)_FAIL - EVT_ID=%d \n", __func__,
			 MSG_MDL_ID_FIELD(msg->msg_id),
			 MSG_EVT_ID_FIELD(msg->msg_id));

		return MDL_RET_FAIL;
	}

	if (MSG_MDL_ID_FIELD(msg->msg_id) != PHL_MDL_GENERAL)
		return MDL_RET_IGNORE;

	if (IS_MSG_IN_PRE_PHASE(msg->msg_id))
		status =
		    _phl_cmd_general_pre_phase_msg_hdlr(phl_info, dispr, msg);
	else
		status =
		    _phl_cmd_general_post_phase_msg_hdlr(phl_info, dispr, msg);

	if (status != RTW_PHL_STATUS_SUCCESS)
		return MDL_RET_FAIL;

	return MDL_RET_SUCCESS;
}

static enum phl_mdl_ret_code
_phl_cmd_general_set_info(void *dispr, void *priv,
			  struct phl_module_op_info *info)
{
	return MDL_RET_SUCCESS;
}

static enum phl_mdl_ret_code
_phl_cmd_general_query_info(void *dispr, void *priv,
			    struct phl_module_op_info *info)
{
	return MDL_RET_SUCCESS;
}

enum rtw_phl_status phl_register_cmd_general(struct phl_info_t *phl_info)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);
	u8 band_idx = 0;

	struct phl_bk_module_ops bk_ops;
	bk_ops.init = _phl_cmd_general_init;
	bk_ops.deinit = _phl_cmd_general_deinit;
	bk_ops.start = _phl_cmd_general_start;
	bk_ops.stop = _phl_cmd_general_stop;
	bk_ops.msg_hdlr = _phl_cmd_general_msg_hdlr;
	bk_ops.set_info = _phl_cmd_general_set_info;
	bk_ops.query_info = _phl_cmd_general_query_info;

	status = RTW_PHL_STATUS_SUCCESS;
	for (band_idx = 0; band_idx < disp_eng->phy_num; band_idx++) {

		if (RTW_PHL_STATUS_SUCCESS !=
		    phl_disp_eng_register_module(phl_info, band_idx,
						 PHL_MDL_GENERAL, &bk_ops)) {

			PHL_ERR(
			    "%s register MDL_GENRAL in cmd disp failed :%d\n",
			    __func__, band_idx + 1);

			status = RTW_PHL_STATUS_FAILURE;
		}
	}

	return status;
}

static enum rtw_phl_status
_phl_cmd_obj_free(struct phl_info_t *phl_info, struct phl_cmd_obj *phl_cmd)
{
	void *drv = phl_to_drvpriv(phl_info);

	if(phl_cmd == NULL) {
		PHL_ERR("%s phl_cmd is NULL\n", __func__);
		_os_warn_on(1);
		return RTW_PHL_STATUS_FAILURE;
	}

	if (phl_cmd->is_cmd_wait == true)
		_phl_cmd_sync_deinit(phl_info, &phl_cmd->cmd_sync);
	_os_kmem_free(drv, phl_cmd, sizeof(struct phl_cmd_obj));
	phl_cmd = NULL;
	return RTW_PHL_STATUS_SUCCESS;
}

static void _phl_cmd_complete(void *priv, struct phl_msg *msg)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)priv;
	void *drv = phl_to_drvpriv(phl_info);
	struct phl_cmd_obj *phl_cmd = (struct phl_cmd_obj *)msg->inbuf;
	enum phl_cmd_sts csts = PHL_CMD_DONE_UNKNOWN;
	enum rtw_phl_status pstst = RTW_PHL_STATUS_SUCCESS;

	PHL_DBG("%s evt_id:%d\n", __func__, phl_cmd->evt_id);

	if (IS_MSG_CANNOT_IO(msg->msg_id))
		csts = PHL_CMD_DONE_CANNOT_IO;
	else if (IS_MSG_FAIL(msg->msg_id))
		csts = PHL_CMD_DONE_CMD_ERROR;
	else if (IS_MSG_CANCEL(msg->msg_id))
		csts = PHL_CMD_DONE_CMD_DROP;
	else
		csts = PHL_CMD_DONE_SUCCESS;

	if (phl_cmd->is_cmd_wait)
		_phl_cmd_done(phl_info, &phl_cmd->cmd_sync, csts);

	pstst = _cmd_stat_2_phl_stat(csts);
	if (phl_cmd->cmd_complete)
		phl_cmd->cmd_complete(drv, phl_cmd->buf, phl_cmd->buf_len, pstst);

	if (phl_cmd->is_cmd_wait) {
		#define PHL_MAX_SCHEDULE_TIMEOUT 100000
		u32 try_cnt = 0;
		u32 start = _os_get_cur_time_ms();

		do {
			if (_os_atomic_read(drv, &(phl_cmd->ref_cnt)) == 1)
				break;
			_os_sleep_ms(drv, 10);
			try_cnt++;
			if (try_cnt == 50)
				PHL_ERR("F-%s, L-%d polling is_cmd_wait to false\n",
							__FUNCTION__, __LINE__);
		} while (phl_get_passing_time_ms(start) < PHL_MAX_SCHEDULE_TIMEOUT);
	}

	_phl_cmd_obj_free(phl_info, phl_cmd);
}

enum rtw_phl_status
phl_cmd_enqueue(struct phl_info_t *phl_info,
                enum phl_band_idx band_idx,
                enum phl_msg_evt_id evt_id,
                u8 *cmd_buf,
                u32 cmd_len,
                void (*cmd_complete)(void* priv, u8 *buf, u32 buf_len, enum rtw_phl_status status),
                enum phl_cmd_type cmd_type,
                u32 cmd_timeout)
{
	void *drv = phl_to_drvpriv(phl_info);
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;
	struct phl_cmd_obj *phl_cmd = NULL;
	struct phl_msg msg = {0};
	struct phl_msg_attribute attr = {0};

	phl_cmd = _os_kmem_alloc(drv, sizeof(struct phl_cmd_obj));
	if (phl_cmd == NULL) {
		PHL_ERR("%s: alloc phl_cmd failed!\n", __func__);
		psts = RTW_PHL_STATUS_RESOURCE;
		goto _exit;
	}
	phl_cmd->evt_id = evt_id;
	phl_cmd->buf = cmd_buf;
	phl_cmd->buf_len = cmd_len;
	phl_cmd->cmd_complete = cmd_complete;

	SET_MSG_MDL_ID_FIELD(msg.msg_id, PHL_MDL_GENERAL);
	SET_MSG_EVT_ID_FIELD(msg.msg_id, evt_id);
	msg.inbuf = (u8 *)phl_cmd;
	msg.inlen = sizeof(struct phl_cmd_obj);
	msg.band_idx = band_idx;
	attr.completion.completion = _phl_cmd_complete;
	attr.completion.priv = phl_info;

	if (cmd_type == PHL_CMD_WAIT) {
		phl_cmd->is_cmd_wait = true;
		_os_atomic_set(drv, &phl_cmd->ref_cnt, 0);
		_phl_cmd_sync_init(phl_info, evt_id, &phl_cmd->cmd_sync, cmd_timeout);
	}

	psts = phl_disp_eng_send_msg(phl_info, &msg, &attr, NULL);
	if (psts == RTW_PHL_STATUS_SUCCESS) {
		if (phl_cmd->is_cmd_wait == true) {
			psts = _phl_cmd_wait(phl_info, &phl_cmd->cmd_sync);
			/*ref_cnt++ for cmd wait done*/
			_os_atomic_inc(drv, &phl_cmd->ref_cnt);
		} else {
			psts = RTW_PHL_STATUS_SUCCESS;
		}
	} else {
		PHL_ERR("%s send msg failed\n", __func__);
		_phl_cmd_obj_free(phl_info, phl_cmd);
	}
_exit:
	return psts;
}

#endif /*CONFIG_CMD_DISP*/

