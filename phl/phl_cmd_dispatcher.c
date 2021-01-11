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
#define _PHL_CMD_DISPR_C_
#include "phl_headers.h"
#ifdef CONFIG_CMD_DISP

#if 0 // NEO TODO

#define MAX_PHL_MSG_NUM (24)
#define MAX_CMD_REQ_NUM (8)
#define MODL_MASK_LEN (PHL_BK_MDL_END/8)

#define IS_GENRAL_MODULE(_mdl_id) ((_mdl_id) <= PHL_MDL_RX)
#define GEN_VALID_HDL(_idx) ((u32)(BIT31 | (u32)(_idx)))
#define IS_HDL_VALID(_hdl) ((_hdl) & BIT31)
#define GET_IDX_FROM_HDL(_hdl) ((u8)((_hdl) & 0xFF))

#define GET_CUR_PENDING_EVT( _obj, _mdl_id) \
	((u16)((_obj)->mdl_info[(_mdl_id)].pending_evt_id))
#define SET_CUR_PENDING_EVT( _obj, _mdl_id, _evt_id) \
	((_obj)->mdl_info[(_mdl_id)].pending_evt_id = (_evt_id))

enum phl_msg_status {
	MSG_STATUS_ENQ = BIT0,
	MSG_STATUS_RUN = BIT1,
	MSG_STATUS_NOTIFY_COMPLETE = BIT2,
	MSG_STATUS_CANCEL = BIT3,
	MSG_STATUS_PRE_PHASE = BIT4,
	MSG_STATUS_FAIL = BIT5,
	MSG_STATUS_OWNER_BK_MDL = BIT6,
	MSG_STATUS_OWNER_REQ = BIT7,
	MSG_STATUS_CLR_SNDR_MSG_IF_PENDING = BIT8,
	MSG_STATUS_PENDING = BIT9,
};

enum cmd_req_status {
	REQ_STATUS_ENQ = BIT0,
	REQ_STATUS_RUN = BIT1,
	REQ_STATUS_CANCEL = BIT2,
	REQ_STATUS_LAST_PERMIT = BIT3,
	REQ_STATUS_PREPARE = BIT4,
};

enum phl_mdl_status {
	MDL_INIT = BIT0,
	MDL_STARTED = BIT1,
};

enum dispatcher_status {
	DISPR_INIT = BIT0,
	DISPR_STARTED = BIT1,
	DISPR_MSGQ_INIT = BIT2,
	DISPR_REQ_INIT = BIT3,
	DISPR_NOTIFY_IDLE = BIT4,
	DISPR_CLR_PEND_MSG = BIT5,
};

enum token_op_type {
	TOKEN_OP_ADD_CMD_REQ = 1,
	TOKEN_OP_FREE_CMD_REQ = 2,
	TOKEN_OP_CANCEL_CMD_REQ = 3,
};

/**
 * phl_bk_module - instance of phl background module,
 * @status: contain mgnt status flags, refer to enum phl_mdl_status
 * @id: refer to enum phl_module_id
 * @priv: private context
 * @ops: interface to interacting with phl_module
 */
struct phl_bk_module {
	_os_list list;
	u8 status;
	u8 id;
	void *priv;
	struct phl_bk_module_ops ops;
};

/**
 * phl_dispr_msg_ex - phl msg extension,
 * @status: contain mgnt status flags, refer to enum phl_msg_status
 * @idx: idx in original msg_ex pool
 * @msg: msg content from external module
 * @premap: notifty map in pre-role phase, refer to enum phl_module_id
 * @postmap: notifty map in post-role phase, refer to enum phl_module_id
 * @completion: msg completion routine.
 * @priv: private context to completion routine.
 * @module: module handle of msg source, only used when msg fails
 */
struct phl_dispr_msg_ex {
	_os_list list;
	u16 status;
	u8 idx;
	struct phl_msg msg;
	u8 premap[MODL_MASK_LEN];
	u8 postmap[MODL_MASK_LEN];
	struct msg_completion_routine completion;
	struct phl_bk_module *module; /* module handle which assign in msg_id*/
};

/**
 * phl_token_op_info - for internal mgnt purpose,
 * @info: mgnt data
 */
struct phl_token_op_info {
	_os_list list;
	u8 used;
	enum token_op_type type;
	u8 data;
};
/**
 * phl_cmd_token_req_ex - cmd token request extension,
 * @status: contain mgnt status flags, refer to enum cmd_req_status
 * @idx: idx in original req_ex pool
 * @req: req content from external module.
 */
struct phl_cmd_token_req_ex {
	_os_list list;
	u8 idx;
	u8 status;
	struct phl_cmd_token_req req;
	struct phl_token_op_info add_req_info;
	struct phl_token_op_info free_req_info;
};

struct mdl_mgnt_info {
	u16 pending_evt_id;
	//void* handle;
};

/**
 * cmd_dispatcher,
 * @idx: idx in dispatch engine, corresponding to band idx
 * @status: contain mgnt status flags, refer to enum dispatcher_status
 * @phl_info: for general reference usage.
 * @module_q: module queue that link each modules based on priority
 * @msg_ex_pool: msg extension pool
 * @bk_thread: background thread
 * @token_req_ex_pool: req extension pool
 * @token_cnt: current token count,
 * 	       cmd req can be executed when dispatcher's token count is 0
 * @bitmap: cosist of existing background modules loaded in current dispatcher,
 *	    refer to enum phl_module_id
 * @basemap: BK modules that must be notified when handling msg
 */
struct cmd_dispatcher {
	u8 idx;
	u8 status;
	struct phl_info_t *phl_info;
	struct phl_queue module_q[PHL_MDL_PRI_MAX];
	struct phl_dispr_msg_ex msg_ex_pool[MAX_PHL_MSG_NUM];
	_os_sema msg_q_sema; /* wake up background thread in SOLO_THREAD_MODE*/
	struct phl_queue msg_wait_q;
	struct phl_queue msg_idle_q;
	struct phl_queue msg_pend_q;
	_os_thread bk_thread;
	struct phl_cmd_token_req_ex token_req_ex_pool[MAX_CMD_REQ_NUM];
	struct phl_queue token_req_wait_q;
	struct phl_queue token_req_idle_q;
	struct phl_queue token_op_q;
	_os_lock token_op_q_lock;
	_os_atomic token_cnt; // atomic
	struct phl_cmd_token_req_ex *cur_cmd_req;
	u8 bitmap[MODL_MASK_LEN];
	u8 basemap[MODL_MASK_LEN];
	struct mdl_mgnt_info mdl_info[PHL_MDL_ID_MAX];
};

enum rtw_phl_status dispr_process_token_req(struct cmd_dispatcher *obj);
void send_bk_msg_phy_on(struct cmd_dispatcher *obj);
void send_bk_msg_phy_idle(struct cmd_dispatcher *obj);

inline static
enum phl_bk_module_priority _get_mdl_priority(enum phl_module_id id)
{
	if (id < PHL_BK_MDL_ROLE_START)
		return PHL_MDL_PRI_MAX;
	else if (id <= PHL_BK_MDL_ROLE_END)
		return PHL_MDL_PRI_ROLE;
	else if ( id <= PHL_BK_MDL_MDRY_END)
		return PHL_MDL_PRI_MANDATORY;
	else if (id <= PHL_BK_MDL_OPT_END)
		return PHL_MDL_PRI_OPTIONAL;
	else
		return PHL_MDL_PRI_MAX;
}

inline static u8 _is_bitmap_empty(void *d, u8 *bitmap)
{
	u8 empty[MODL_MASK_LEN] = {0};

	return (!_os_mem_cmp(d, bitmap, empty, MODL_MASK_LEN))?(true):(false);
}

inline static void _print_bitmap(u8 *bitmap)
{
	u8 k = 0;

	PHL_INFO("print bitmap: \n");
	for (k = 0; k < MODL_MASK_LEN; k++) {
		PHL_DBG("[%d]:0x%x\n", k, bitmap[k]);
	}
}

static void notify_bk_thread(struct cmd_dispatcher *obj)
{
	void *d = phl_to_drvpriv(obj->phl_info);

	if (disp_eng_is_solo_thread_mode(obj->phl_info))
		_os_sema_up(d, &(obj->msg_q_sema));
	else
		disp_eng_notify_share_thread(obj->phl_info, (void*)obj);
}

static u8 pop_front_idle_msg(struct cmd_dispatcher *obj,
			     struct phl_dispr_msg_ex **msg)
{
	void *d = phl_to_drvpriv(obj->phl_info);
	_os_list *new_msg = NULL;

	(*msg) = NULL;
	if (pq_pop(d, &(obj->msg_idle_q), &new_msg, _first, _bh)) {
		(*msg) = (struct phl_dispr_msg_ex *)new_msg;
		(*msg)->status = 0;
		(*msg)->module = NULL;
		(*msg)->completion.priv = NULL;
		(*msg)->completion.completion = NULL;
		_os_mem_set(d, (*msg)->premap, 0, MODL_MASK_LEN);
		_os_mem_set(d, (*msg)->postmap, 0, MODL_MASK_LEN);
		_os_mem_set(d, &((*msg)->msg), 0, sizeof(struct phl_msg));
		PHL_INFO("%s: remain cnt(%d)\n", __FUNCTION__, obj->msg_idle_q.cnt);
		return true;
	} else {
		return false;
	}
}

static void push_back_idle_msg(struct cmd_dispatcher *obj,
			       struct phl_dispr_msg_ex *ex)
{
	void *d = phl_to_drvpriv(obj->phl_info);

	if (TEST_STATUS_FLAG(ex->status, MSG_STATUS_NOTIFY_COMPLETE) &&
	    ex->completion.completion) {
		if (TEST_STATUS_FLAG(ex->status, MSG_STATUS_CANCEL))
			SET_MSG_INDC_FIELD(ex->msg.msg_id, MSG_INDC_CANCEL);
		ex->completion.completion(ex->completion.priv, &(ex->msg));
		CLEAR_STATUS_FLAG(ex->status, MSG_STATUS_NOTIFY_COMPLETE);
	}
	ex->status = 0;
	if(GET_CUR_PENDING_EVT(obj, MSG_MDL_ID_FIELD(ex->msg.msg_id)) == MSG_EVT_ID_FIELD(ex->msg.msg_id))
		SET_CUR_PENDING_EVT(obj, MSG_MDL_ID_FIELD(ex->msg.msg_id), MSG_EVT_MAX);
	ex->msg.msg_id = 0;
	pq_push(d, &(obj->msg_idle_q), &(ex->list), _tail, _bh);
}

static u8 pop_front_wait_msg(struct cmd_dispatcher *obj,
			     struct phl_dispr_msg_ex **msg)
{
	void *d = phl_to_drvpriv(obj->phl_info);
	_os_list *new_msg = NULL;

	(*msg) = NULL;
	if (pq_pop(d, &(obj->msg_wait_q), &new_msg, _first, _bh)) {
		(*msg) = (struct phl_dispr_msg_ex *)new_msg;
		SET_STATUS_FLAG((*msg)->status, MSG_STATUS_RUN);
		CLEAR_STATUS_FLAG((*msg)->status, MSG_STATUS_ENQ);
		CLEAR_STATUS_FLAG((*msg)->status, MSG_STATUS_PENDING);
		return true;
	} else {
		return false;
	}
}

static void push_back_wait_msg(struct cmd_dispatcher *obj,
			       struct phl_dispr_msg_ex *ex)
{
	void *d = phl_to_drvpriv(obj->phl_info);

	SET_STATUS_FLAG(ex->status, MSG_STATUS_ENQ);
	CLEAR_STATUS_FLAG(ex->status, MSG_STATUS_RUN);
	pq_push(d, &(obj->msg_wait_q), &(ex->list), _tail, _bh);
	notify_bk_thread(obj);
}

static u8 pop_front_pending_msg(struct cmd_dispatcher *obj,
			     struct phl_dispr_msg_ex **msg)
{
	void *d = phl_to_drvpriv(obj->phl_info);
	_os_list *new_msg = NULL;

	(*msg) = NULL;
	if (pq_pop(d, &(obj->msg_pend_q), &new_msg, _first, _bh)) {
		(*msg) = (struct phl_dispr_msg_ex *)new_msg;
		return true;
	} else {
		return false;
	}
}

static void push_back_pending_msg(struct cmd_dispatcher *obj,
			       struct phl_dispr_msg_ex *ex)
{
	void *d = phl_to_drvpriv(obj->phl_info);

	SET_STATUS_FLAG(ex->status, MSG_STATUS_ENQ);
	CLEAR_STATUS_FLAG(ex->status, MSG_STATUS_RUN);

	if(TEST_STATUS_FLAG(ex->status, MSG_STATUS_CLR_SNDR_MSG_IF_PENDING))
		SET_CUR_PENDING_EVT(obj, MSG_MDL_ID_FIELD(ex->msg.msg_id), MSG_EVT_ID_FIELD(ex->msg.msg_id));
	pq_push(d, &(obj->msg_pend_q), &(ex->list), _tail, _bh);
	PHL_INFO("%s: remain cnt(%d)\n", __FUNCTION__, obj->msg_pend_q.cnt);
}

static void clear_pending_msg(struct cmd_dispatcher *obj)
{
	struct phl_dispr_msg_ex *ex = NULL;

	if(!TEST_STATUS_FLAG(obj->status, DISPR_CLR_PEND_MSG))
		return;
	CLEAR_STATUS_FLAG(obj->status, DISPR_CLR_PEND_MSG);
	while (pop_front_pending_msg(obj, &ex))
		push_back_wait_msg(obj, ex);
}

static void clear_waiting_msg(struct cmd_dispatcher *obj)
{
	struct phl_dispr_msg_ex *ex = NULL;

	PHL_INFO("%s: remain cnt(%d)\n", __FUNCTION__, obj->msg_idle_q.cnt);
	while(obj->msg_idle_q.cnt != MAX_PHL_MSG_NUM) {
		while (pop_front_pending_msg(obj, &ex))
			push_back_wait_msg(obj, ex);
		while (pop_front_wait_msg(obj, &ex))
			push_back_idle_msg(obj, ex);
	}
}

static bool is_msg_canceled(struct cmd_dispatcher *obj, struct phl_dispr_msg_ex *ex)
{
	u16 pending_evt = GET_CUR_PENDING_EVT(obj, MSG_MDL_ID_FIELD(ex->msg.msg_id));

	if (!TEST_STATUS_FLAG(obj->status, DISPR_STARTED) ||
	    TEST_STATUS_FLAG(ex->status, MSG_STATUS_CANCEL))
		return true;

	if (pending_evt != MSG_EVT_MAX && pending_evt != MSG_EVT_ID_FIELD(ex->msg.msg_id)) {
		SET_STATUS_FLAG(ex->status, MSG_STATUS_CANCEL);
		PHL_INFO("msg canceled, cur pending evt(%d)\n", pending_evt);
		return true;
	}

	return false;
}

void init_dispr_msg_pool(struct cmd_dispatcher *obj)
{
	u8 i = 0;
	void *d = phl_to_drvpriv(obj->phl_info);

	if (TEST_STATUS_FLAG(obj->status, DISPR_MSGQ_INIT))
		return;
	pq_init(d, &(obj->msg_idle_q));
	pq_init(d, &(obj->msg_wait_q));
	pq_init(d, &(obj->msg_pend_q));
	_os_mem_set(d, obj->msg_ex_pool, 0,
		    sizeof(struct phl_dispr_msg_ex) * MAX_PHL_MSG_NUM);
	for (i = 0; i < MAX_PHL_MSG_NUM; i++) {
		obj->msg_ex_pool[i].idx = i;
		push_back_idle_msg(obj, &(obj->msg_ex_pool[i]));
	}

	SET_STATUS_FLAG(obj->status, DISPR_MSGQ_INIT);
}

void deinit_dispr_msg_pool(struct cmd_dispatcher *obj)
{
	void *d = phl_to_drvpriv(obj->phl_info);

	if (!TEST_STATUS_FLAG(obj->status, DISPR_MSGQ_INIT))
		return;
	CLEAR_STATUS_FLAG(obj->status, DISPR_MSGQ_INIT);

	pq_deinit(d, &(obj->msg_idle_q));
	pq_deinit(d, &(obj->msg_wait_q));
	pq_deinit(d, &(obj->msg_pend_q));
}

void cancel_msg(struct cmd_dispatcher *obj, struct phl_dispr_msg_ex *ex)
{
	void *d = phl_to_drvpriv(obj->phl_info);

	/* zero bitmap to ensure msg would not be forward to
	 * any modules after cancel.
	 * */
	_reset_bitmap(d, ex->premap, MODL_MASK_LEN);
	_reset_bitmap(d, ex->postmap, MODL_MASK_LEN);

	SET_STATUS_FLAG(ex->status, MSG_STATUS_CANCEL);
}

void cancel_running_msg(struct cmd_dispatcher *obj)
{
	u8 i = 0;

	for (i = 0; i < MAX_PHL_MSG_NUM;i++) {
		if(TEST_STATUS_FLAG(obj->msg_ex_pool[i].status, MSG_STATUS_RUN))
			cancel_msg(obj, &(obj->msg_ex_pool[i]));
	}
}
void set_msg_bitmap(struct cmd_dispatcher *obj, struct phl_dispr_msg_ex *ex,
		    enum phl_msg_opt opt, u8 mdl_id, u8 *id_arr, u32 len)
{
	void *d = phl_to_drvpriv(obj->phl_info);

	if (!(opt & MSG_OPT_SKIP_NOTIFY_OPT_MDL)) {
		_os_mem_cpy(d, ex->premap, obj->bitmap, MODL_MASK_LEN);
		_os_mem_cpy(d, ex->postmap, obj->bitmap, MODL_MASK_LEN);
	} else {
		/* ensure mandatory & wifi role module recv all msg*/
		_os_mem_cpy(d, ex->premap, obj->basemap, MODL_MASK_LEN);
		_os_mem_cpy(d, ex->postmap, obj->basemap, MODL_MASK_LEN);
	}
	if (opt & MSG_OPT_BLIST_PRESENT) {
		_clr_bitmap_bit(ex->premap, id_arr, len);
		_clr_bitmap_bit(ex->postmap, id_arr, len);
	} else {
		_add_bitmap_bit(ex->premap, id_arr, len);
		_add_bitmap_bit(ex->postmap, id_arr, len);
	}

	if(_chk_bitmap_bit(obj->bitmap, mdl_id)) {
		_add_bitmap_bit(ex->premap, &mdl_id, 1);
		_add_bitmap_bit(ex->postmap, &mdl_id, 1);
	}
//_print_bitmap(ex->premap);
}

u8 *get_msg_bitmap(struct phl_dispr_msg_ex *ex)
{
	if (TEST_STATUS_FLAG(ex->status, MSG_STATUS_PRE_PHASE)) {
		SET_MSG_INDC_FIELD(ex->msg.msg_id, MSG_INDC_PRE_PHASE);
		return ex->premap;
	} else {
		CLEAR_MSG_INDC_FIELD(ex->msg.msg_id, MSG_INDC_PRE_PHASE);
		return ex->postmap;
	}
}


void init_dispr_mdl_mgnt_info(struct cmd_dispatcher *obj)
{
	u8 i = 0;

	for (i = 0; i < PHL_MDL_ID_MAX; i++)
		SET_CUR_PENDING_EVT(obj, i, MSG_EVT_MAX);

}

static u8 pop_front_idle_req(struct cmd_dispatcher *obj,
			     struct phl_cmd_token_req_ex **req)
{
	void *d = phl_to_drvpriv(obj->phl_info);
	_os_list *new_req = NULL;

	(*req) = NULL;
	if (pq_pop(d, &(obj->token_req_idle_q), &new_req, _first, _bh)) {
		(*req) = (struct phl_cmd_token_req_ex*)new_req;
		(*req)->status = 0;
		_os_mem_set(d, &((*req)->req), 0,
			    sizeof(struct phl_cmd_token_req));
		_os_mem_set(d, &((*req)->add_req_info), 0,
			    sizeof(struct phl_token_op_info));
		_os_mem_set(d, &((*req)->free_req_info), 0,
			    sizeof(struct phl_token_op_info));
		return true;
	} else {
		return false;
	}
}

static void push_back_idle_req(struct cmd_dispatcher *obj,
			       struct phl_cmd_token_req_ex *req)
{
	void *d = phl_to_drvpriv(obj->phl_info);

	req->status = 0;
	SET_CUR_PENDING_EVT(obj, req->req.module_id, MSG_EVT_MAX);
	pq_push(d, &(obj->token_req_idle_q), &(req->list), _tail, _bh);
}

static u8 pop_front_wait_req(struct cmd_dispatcher *obj,
			     struct phl_cmd_token_req_ex **req)
{
	void *d = phl_to_drvpriv(obj->phl_info);
	_os_list *new_req = NULL;

	(*req) = NULL;
	if (pq_pop(d, &(obj->token_req_wait_q), &new_req, _first, _bh)) {
		(*req) = (struct phl_cmd_token_req_ex*)new_req;
		SET_STATUS_FLAG((*req)->status, REQ_STATUS_PREPARE);
		CLEAR_STATUS_FLAG((*req)->status, REQ_STATUS_ENQ);
		return true;
	} else {
		return false;
	}
}

static void push_back_wait_req(struct cmd_dispatcher *obj,
			       struct phl_cmd_token_req_ex *req)
{
	void *d = phl_to_drvpriv(obj->phl_info);

	pq_push(d, &(obj->token_req_wait_q), &(req->list), _tail, _bh);
	SET_STATUS_FLAG(req->status, REQ_STATUS_ENQ);
}

static void clear_wating_req(struct cmd_dispatcher *obj)
{
	 struct phl_cmd_token_req_ex *ex = NULL;

	PHL_INFO("%s: remain cnt(%d)\n", __FUNCTION__, obj->token_req_idle_q.cnt);
	while(obj->token_req_idle_q.cnt != MAX_CMD_REQ_NUM) {
		while (pop_front_wait_req(obj, &ex)) {
			ex->req.abort(obj, ex->req.priv);
			push_back_idle_req(obj, ex);
		}
	}
}

void deregister_cur_cmd_req(struct cmd_dispatcher *obj, u8 notify)
{
	struct phl_cmd_token_req *req = NULL;
	void *d = phl_to_drvpriv(obj->phl_info);
	u8 i = 0;
	struct phl_dispr_msg_ex *ex = NULL;

	if (obj->cur_cmd_req) {
		req = &(obj->cur_cmd_req->req);
		PHL_INFO("%s, id(%d), status(%d)\n", __FUNCTION__, req->module_id, obj->cur_cmd_req->status);
		CLEAR_STATUS_FLAG(obj->cur_cmd_req->status, REQ_STATUS_RUN);
		for (i = 0; i < MAX_PHL_MSG_NUM; i++) {
			ex = &(obj->msg_ex_pool[i]);
			if (req->module_id != MSG_MDL_ID_FIELD(ex->msg.msg_id))
				continue;
			CLEAR_STATUS_FLAG(ex->status, MSG_STATUS_OWNER_REQ);
			cancel_msg(obj, ex);
			if(TEST_STATUS_FLAG(ex->status, MSG_STATUS_PENDING)) {
				phl_dispr_clr_pending_msg((void*)obj);
				/* inserted pending msg of this sepecific sender
				 * back to wait Q before abort notify
				 * would guarantee msg sent in abort notify is exactly last msg from this sender
				 * */
				clear_pending_msg(obj);
			}
		}
		if (notify == true) {
			SET_STATUS_FLAG(obj->cur_cmd_req->status, REQ_STATUS_LAST_PERMIT);
			req->abort(obj, req->priv);
			CLEAR_STATUS_FLAG(obj->cur_cmd_req->status, REQ_STATUS_LAST_PERMIT);
		}
		push_back_idle_req(obj, obj->cur_cmd_req);
		_os_atomic_set(d, &(obj->token_cnt),
			       _os_atomic_read(d, &(obj->token_cnt))-1);
	}
	obj->cur_cmd_req = NULL;
	PHL_INFO("%s\n", __FUNCTION__);
}

u8 register_cur_cmd_req(struct cmd_dispatcher *obj,
			  struct phl_cmd_token_req_ex *req)
{
	void *d = phl_to_drvpriv(obj->phl_info);
	enum phl_mdl_ret_code ret = MDL_RET_SUCCESS;

	SET_STATUS_FLAG(req->status, REQ_STATUS_RUN);
	CLEAR_STATUS_FLAG(req->status, REQ_STATUS_PREPARE);
	obj->cur_cmd_req = req;
	_os_atomic_set(d, &(obj->token_cnt),
		       _os_atomic_read(d, &(obj->token_cnt))+1);
	PHL_INFO("%s, id(%d)\n", __FUNCTION__, obj->cur_cmd_req->req.module_id);
	ret = obj->cur_cmd_req->req.acquired((void*)obj, obj->cur_cmd_req->req.priv);
	PHL_INFO("%s, ret(%d)\n", __FUNCTION__, ret);

	if (ret == MDL_RET_FAIL) {
		deregister_cur_cmd_req(obj, false);
		return false;
	}
	else
		return true;
}

void cancel_all_cmd_req(struct cmd_dispatcher *obj)
{
	u8 i = 0;
	void *d = phl_to_drvpriv(obj->phl_info);
	struct phl_cmd_token_req_ex* req_ex = NULL;

	for (i = 0; i < MAX_CMD_REQ_NUM;i++) {
		req_ex = &(obj->token_req_ex_pool[i]);
		if (req_ex->status)
			SET_STATUS_FLAG(req_ex->status, REQ_STATUS_CANCEL);
	}
}

void init_cmd_req_pool(struct cmd_dispatcher *obj)
{
	u8 i = 0;
	void *d = phl_to_drvpriv(obj->phl_info);

	if (TEST_STATUS_FLAG(obj->status, DISPR_REQ_INIT))
		return;
	pq_init(d, &(obj->token_req_wait_q));
	pq_init(d, &(obj->token_req_idle_q));
	pq_init(d, &(obj->token_op_q));
	_os_mem_set(d, obj->token_req_ex_pool, 0,
		    sizeof(struct phl_cmd_token_req_ex) * MAX_CMD_REQ_NUM);
	for (i = 0; i < MAX_CMD_REQ_NUM;i++) {
		obj->token_req_ex_pool[i].idx = i;
		pq_push(d, &(obj->token_req_idle_q),
			&(obj->token_req_ex_pool[i].list), _tail, _bh);
	}
	SET_STATUS_FLAG(obj->status, DISPR_REQ_INIT);
}

void deinit_cmd_req_pool(struct cmd_dispatcher *obj)
{
	void *d = phl_to_drvpriv(obj->phl_info);

	CLEAR_STATUS_FLAG(obj->status, DISPR_REQ_INIT);

	pq_deinit(d, &(obj->token_req_wait_q));
	pq_deinit(d, &(obj->token_req_idle_q));
	pq_deinit(d, &(obj->token_op_q));
}

u8 chk_module_ops(struct phl_bk_module_ops *ops)
{
	if (ops == NULL ||
	    ops->init == NULL ||
	    ops->deinit == NULL ||
	    ops->msg_hdlr == NULL ||
	    ops->set_info == NULL ||
	    ops->query_info == NULL ||
	    ops->start == NULL ||
	    ops->stop == NULL)
		return false;
	return true;
}

u8 chk_cmd_req_ops(struct phl_cmd_token_req *req)
{
	if (req == NULL ||
	    req->module_id < PHL_FG_MDL_START ||
	    req->abort == NULL ||
	    req->acquired == NULL ||
	    req->msg_hdlr == NULL ||
	    req->set_info == NULL ||
	    req->query_info == NULL)
		return false;
	return true;
}
static u8 pop_front_token_op_info(struct cmd_dispatcher *obj,
				  struct phl_token_op_info **op_info)
{
	void *d = phl_to_drvpriv(obj->phl_info);
	_os_list *new_info = NULL;

	(*op_info) = NULL;
	if (pq_pop(d, &(obj->token_op_q), &new_info, _first, _bh)) {
		(*op_info) = (struct phl_token_op_info *)new_info;
		return true;
	} else {
		return false;
	}
}

static u8 push_back_token_op_info(struct cmd_dispatcher *obj,
				  struct phl_token_op_info *op_info,
				  enum token_op_type type,
			    	  u8 data)
{
	void *d = phl_to_drvpriv(obj->phl_info);
	_os_spinlockfg sp_flags;

	_os_spinlock(d, &obj->token_op_q_lock, _bh, &sp_flags);
	if (op_info->used == true) {
		_os_spinunlock(d, &obj->token_op_q_lock, _bh, &sp_flags);
		return false;
	}
	op_info->used = true;
	op_info->type = type;
	op_info->data = data;
	_os_spinunlock(d, &obj->token_op_q_lock, _bh, &sp_flags);
	pq_push(d, &(obj->token_op_q), &(op_info->list), _tail, _bh);
	notify_bk_thread(obj);
	return true;
}

void _handle_token_op_info(struct cmd_dispatcher *obj, struct phl_token_op_info *op_info)
{
	struct phl_cmd_token_req_ex *req_ex = NULL;
	void *d = phl_to_drvpriv(obj->phl_info);

	switch (op_info->type) {
		case TOKEN_OP_ADD_CMD_REQ:
			dispr_process_token_req(obj);
			break;
		case TOKEN_OP_FREE_CMD_REQ:
			req_ex = &(obj->token_req_ex_pool[op_info->data]);
			if (!TEST_STATUS_FLAG(req_ex->status, REQ_STATUS_RUN))
				break;
			deregister_cur_cmd_req(obj, false);
			dispr_process_token_req(obj);
			break;
		case TOKEN_OP_CANCEL_CMD_REQ:
			req_ex = &(obj->token_req_ex_pool[op_info->data]);
			if (TEST_STATUS_FLAG(req_ex->status, REQ_STATUS_ENQ)) {
				pq_del_node(d, &(obj->token_req_wait_q), &(req_ex->list), _bh);
				/*
				 * Call command abort handle, abort handle 
				 * should decide it has been acquired or not.
				 */
				req_ex->req.abort(obj, req_ex->req.priv);
				push_back_idle_req(obj, req_ex);
			} else if (TEST_STATUS_FLAG(req_ex->status, REQ_STATUS_RUN)){
				deregister_cur_cmd_req(obj, true);
				dispr_process_token_req(obj);
			}
			break;
		default:
			break;
	}
}

void token_op_hanler(struct cmd_dispatcher *obj)
{
	struct phl_token_op_info *info = NULL;

	while (pop_front_token_op_info(obj, &info)) {
		_handle_token_op_info(obj, info);
		info->used = false;
	}
}
static u8
dispr_enqueue_token_op_info(struct cmd_dispatcher *obj,
			    struct phl_token_op_info *op_info,
			    enum token_op_type type,
			    u8 data)
{
	return push_back_token_op_info(obj, op_info, type, data);
}

u8 bk_module_init(struct cmd_dispatcher *obj, struct phl_bk_module *module)
{
	if (TEST_STATUS_FLAG(module->status, MDL_INIT)) {
		PHL_ERR("%s module_id:%d already init\n",
			__FUNCTION__, module->id);
		return false;
	}

	if (module->ops.init((void*)obj->phl_info, (void*)obj,
			     &(module->priv)) == MDL_RET_SUCCESS) {
		SET_STATUS_FLAG(module->status, MDL_INIT);
		return true;
	} else {
		PHL_ERR("%s fail module_id: %d \n", __FUNCTION__, module->id);
		return false;
	}
}

void bk_module_deinit(struct cmd_dispatcher *obj, struct phl_bk_module *module)
{
	if (TEST_STATUS_FLAG(module->status, MDL_INIT))
		module->ops.deinit((void*)obj, module->priv);
	CLEAR_STATUS_FLAG(module->status, MDL_INIT);
}

u8 bk_module_start(struct cmd_dispatcher *obj, struct phl_bk_module *module)
{
	if (!TEST_STATUS_FLAG(module->status, MDL_INIT) ||
	    TEST_STATUS_FLAG(module->status, MDL_STARTED)) {
		PHL_ERR("%s module_id:%d already start\n", __FUNCTION__,
			module->id);
		return false;
	}

	if (module->ops.start((void*)obj, module->priv) == MDL_RET_SUCCESS) {
		SET_STATUS_FLAG(module->status, MDL_STARTED);
		return true;
	} else {
		PHL_ERR("%s fail module_id: %d \n", __FUNCTION__,
			module->id);
		return false;
	}
}

u8 bk_module_stop(struct cmd_dispatcher *obj, struct phl_bk_module *module)
{
	if (!TEST_STATUS_FLAG(module->status, MDL_STARTED))
		return false;
	CLEAR_STATUS_FLAG(module->status, MDL_STARTED);
	if (module->ops.stop((void*)obj, module->priv) == MDL_RET_SUCCESS) {
		return true;
	} else {
		PHL_ERR("%s fail module_id: %d \n", __FUNCTION__,
			module->id);
		return false;
	}
}

void cur_req_hdl(struct cmd_dispatcher *obj, struct phl_dispr_msg_ex *ex)
{
	struct phl_cmd_token_req_ex *cur_req = obj->cur_cmd_req;

	if (cur_req == NULL)
		return;
	if (!TEST_STATUS_FLAG(cur_req->status, REQ_STATUS_RUN) ||
	    TEST_STATUS_FLAG(cur_req->status, REQ_STATUS_CANCEL))
		return;
	cur_req->req.msg_hdlr((void*)obj, cur_req->req.priv, &(ex->msg));
}

void notify_msg_fail(struct cmd_dispatcher *obj, struct phl_dispr_msg_ex *ex)
{
	SET_STATUS_FLAG(ex->status, MSG_STATUS_FAIL);
	SET_MSG_INDC_FIELD(ex->msg.msg_id, MSG_INDC_FAIL);
	PHL_INFO("%s\n", __FUNCTION__);
	if (TEST_STATUS_FLAG(ex->status, MSG_STATUS_OWNER_BK_MDL) &&
	    _chk_bitmap_bit(obj->bitmap, ex->module->id)) {
		ex->module->ops.msg_hdlr(obj, ex->module->priv, &(ex->msg));
	}

	if (TEST_STATUS_FLAG(ex->status, MSG_STATUS_OWNER_REQ)) {
		cur_req_hdl(obj, ex);
	}
}

enum phl_mdl_ret_code feed_mdl_msg(struct cmd_dispatcher *obj,
				   struct phl_bk_module *mdl,
				   struct phl_dispr_msg_ex *ex)
{
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	u8 *bitmap = NULL;

	PHL_DBG("%s, id:%d \n", __FUNCTION__, mdl->id);
	ret = mdl->ops.msg_hdlr(obj, mdl->priv, &(ex->msg));
	if (ret == MDL_RET_FAIL) {
		PHL_INFO("id:%d evt:0x%x fail\n",
			 mdl->id, ex->msg.msg_id);
		notify_msg_fail(obj, ex);
	} else if (ret == MDL_RET_PENDING) {
		PHL_INFO("id:%d evt:0x%x pending\n",
			 mdl->id, ex->msg.msg_id);
		SET_STATUS_FLAG(ex->status, MSG_STATUS_PENDING);
	} else {
		if (MSG_INDC_FIELD(ex->msg.msg_id) & MSG_INDC_PRE_PHASE)
			bitmap = ex->premap;
		else
			bitmap = ex->postmap;
		_clr_bitmap_bit(bitmap, &(mdl->id), 1);
	}
	return ret;
}

void msg_pre_phase_hdl(struct cmd_dispatcher *obj, struct phl_dispr_msg_ex *ex)
{
	s8 i = 0;
	void *d = phl_to_drvpriv(obj->phl_info);
	struct phl_bk_module *mdl = NULL;
	_os_list *node = NULL;
	struct phl_queue *q = NULL;
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	u8 owner_id = (ex->module)?(ex->module->id):(PHL_MDL_ID_MAX);
	enum phl_bk_module_priority priority = PHL_MDL_PRI_MAX;

	if (owner_id <= PHL_BK_MDL_END)
		priority = _get_mdl_priority(owner_id);

	for (i = PHL_MDL_PRI_MAX - 1 ; i >= PHL_MDL_PRI_ROLE ; i--) {
		if (priority == i && _chk_bitmap_bit(obj->bitmap, owner_id)) {
			ret = feed_mdl_msg(obj, ex->module, ex);
			if (ret == MDL_RET_FAIL || ret == MDL_RET_PENDING)
				return;
		}
		q = &(obj->module_q[(u8)i]);

		if (pq_get_front(d, q, &node, _bh) == false)
			continue;

		do {
			mdl = (struct phl_bk_module*)node;
			if (!_chk_bitmap_bit(ex->premap, mdl->id) ||
			    !TEST_STATUS_FLAG(mdl->status, MDL_STARTED))
				continue;
			ret = feed_mdl_msg(obj, mdl, ex);
			if (ret == MDL_RET_FAIL || ret == MDL_RET_PENDING)
				return;
		} while(pq_get_next(d, q, node, &node, _bh));
	}
}

void msg_post_phase_hdl(struct cmd_dispatcher *obj, struct phl_dispr_msg_ex *ex)
{
	s8 i = 0;
	void *d = phl_to_drvpriv(obj->phl_info);
	struct phl_bk_module *mdl = NULL;
	_os_list *node = NULL;
	struct phl_queue *q = NULL;
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	u8 owner_id = (ex->module)?(ex->module->id):(PHL_MDL_ID_MAX);
	enum phl_bk_module_priority priority = PHL_MDL_PRI_MAX;

	if (owner_id <= PHL_BK_MDL_END)
		priority = _get_mdl_priority(owner_id);

	for (i = PHL_MDL_PRI_ROLE ; i < PHL_MDL_PRI_MAX ; i++) {
		if (priority == i && _chk_bitmap_bit(obj->bitmap, owner_id)) {
			ret = feed_mdl_msg(obj, ex->module, ex);
			if (ret == MDL_RET_FAIL || ret == MDL_RET_PENDING)
				return;
		}
		q = &(obj->module_q[(u8)i]);
		if (pq_get_front(d, q, &node, _bh) == false)
			continue;
		do {
			mdl = (struct phl_bk_module*)node;
			if (!_chk_bitmap_bit(ex->postmap, mdl->id)||
			    !TEST_STATUS_FLAG(mdl->status, MDL_STARTED))
				continue;
			ret = feed_mdl_msg(obj, mdl, ex);
			if (ret == MDL_RET_FAIL || ret == MDL_RET_PENDING)
				return;
		} while(pq_get_next(d, q, node, &node, _bh));
	}
}

u8 get_cur_cmd_req_id(struct cmd_dispatcher *obj, u32 *req_status)
{
	struct phl_cmd_token_req_ex *cur_req = obj->cur_cmd_req;

	if(req_status)
		*req_status = 0;

	if (cur_req == NULL )
		return (u8)PHL_MDL_ID_MAX;

	if(req_status)
		*req_status = cur_req->status;

	if(!TEST_STATUS_FLAG(cur_req->status, REQ_STATUS_RUN) ||
		TEST_STATUS_FLAG(cur_req->status, REQ_STATUS_CANCEL))
		return (u8)PHL_MDL_ID_MAX;
	else
		return cur_req->req.module_id;
}


void msg_dispatch(struct cmd_dispatcher *obj, struct phl_dispr_msg_ex *ex)
{
	u8 *bitmap = get_msg_bitmap(ex);
	void *d = phl_to_drvpriv(obj->phl_info);

	PHL_DBG("%s, msg_id:0x%x \n", __FUNCTION__, ex->msg.msg_id);
	if ((MSG_INDC_FIELD(ex->msg.msg_id) & MSG_INDC_PRE_PHASE) &&
	    _is_bitmap_empty(d, bitmap) == false)
		msg_pre_phase_hdl(obj, ex);

	if (TEST_STATUS_FLAG(ex->status, MSG_STATUS_FAIL)||
	    TEST_STATUS_FLAG(ex->status, MSG_STATUS_CANCEL))
		goto recycle;

	if (_is_bitmap_empty(d, bitmap)) {
		/* pre protocol phase done, switch to post protocol phase*/
		CLEAR_STATUS_FLAG(ex->status, MSG_STATUS_PRE_PHASE);
		bitmap = get_msg_bitmap(ex);
	} else {
		goto reschedule;
	}

	if (_is_bitmap_empty(d, bitmap) == false)
		msg_post_phase_hdl(obj, ex);

	if (TEST_STATUS_FLAG(ex->status, MSG_STATUS_FAIL)||
	    TEST_STATUS_FLAG(ex->status, MSG_STATUS_CANCEL))
		goto recycle;

	if (_is_bitmap_empty(d, bitmap)) {
		/* post protocol phase done */
		cur_req_hdl(obj, ex);
		goto recycle;
	}
reschedule:
	PHL_INFO("%s, msg:0x%x reschedule \n", __FUNCTION__,
		 ex->msg.msg_id);
	if(TEST_STATUS_FLAG(ex->status, MSG_STATUS_PENDING))
		push_back_pending_msg(obj, ex);
	else
		push_back_wait_msg(obj, ex);
	return;
recycle:
	PHL_DBG("%s, msg:0x%x recycle \n", __FUNCTION__,
		 ex->msg.msg_id);
	push_back_idle_msg(obj, ex);
}

void dispr_thread_loop_hdl(struct cmd_dispatcher *obj)
{
	struct phl_dispr_msg_ex *ex = NULL;

	/* check pending msg need in advance.
	* if pending msg is not empty before while loop breaks,
	* these msg would be cleared in deinit_dispr_msg_pool.
	*/
	clear_pending_msg(obj);
	/* token op Q in advance.
	* if req wait Q is not empty before while loop breaks,
	* these msg would be cleared in deinit_cmd_req_pool.
	*/
	token_op_hanler(obj);

	if (pop_front_wait_msg(obj, &ex)) {
		if (is_msg_canceled(obj, ex)) {
			push_back_idle_msg(obj, ex);
			return;
		}
		/* ensure all modules set in msg bitmap
			exists in cur dispatcher*/
		_and_bitmaps(obj->bitmap, ex->premap, MODL_MASK_LEN);
		_and_bitmaps(obj->bitmap, ex->postmap, MODL_MASK_LEN);
		msg_dispatch(obj, ex);
	}
}

void dispr_thread_leave_hdl(struct cmd_dispatcher *obj)
{
	deregister_cur_cmd_req(obj, true);
	/* clear remaining pending & waiting msg */
	clear_waiting_msg(obj);
	/* pop out all waiting cmd req and notify abort. */
	clear_wating_req(obj);
}

int background_thread_hdl(void *param)
{
	struct cmd_dispatcher *obj = (struct cmd_dispatcher *)param;
	void *d = phl_to_drvpriv(obj->phl_info);

	PHL_INFO("%s enter\n", __FUNCTION__);
	while (!_os_thread_check_stop(d, &(obj->bk_thread))) {

		_os_sema_down(d, &obj->msg_q_sema);

		if(_os_thread_check_stop(d, &(obj->bk_thread)))
			break;
		dispr_thread_loop_hdl(obj);
	}
	dispr_thread_leave_hdl(obj);
	_os_thread_wait_stop(d, &(obj->bk_thread));
	PHL_INFO("%s down\n", __FUNCTION__);
	return 0;
}

u8 search_mdl(void *d, void *mdl, void *priv)
{
	enum phl_module_id id = *(enum phl_module_id *)priv;
	struct phl_bk_module *module = NULL;

	module = (struct phl_bk_module *)mdl;
	if (module->id == id) {
		PHL_INFO("%s :: id %d\n", __FUNCTION__, id);
		return true;
	}
	else
		return false;
}

u8 get_module_by_id(struct cmd_dispatcher *obj, u8 id,
		    struct phl_bk_module **mdl)
{
	void *d = phl_to_drvpriv(obj->phl_info);
	u8 i = 0;
	_os_list *node = NULL;

	if (IS_GENRAL_MODULE(id) ||
	    !_chk_bitmap_bit(obj->bitmap, id) ||
	    mdl == NULL)
		return false;

	for (i = 0; i < PHL_MDL_PRI_MAX; i++) {

		if(pq_search_node(d, &(obj->module_q[i]), &node, _bh, false, &id, search_mdl)) {
			*mdl = (struct phl_bk_module*)node;
			return true;
		}
	}
	*mdl = NULL;
	return false;
}

enum rtw_phl_status phl_dispr_get_idx(void *dispr, u8 *idx)
{
	struct cmd_dispatcher *obj = (struct cmd_dispatcher *)dispr;

	if (dispr == NULL)
		return RTW_PHL_STATUS_FAILURE;
	if (!TEST_STATUS_FLAG(obj->status, DISPR_INIT) || idx == NULL)
		return RTW_PHL_STATUS_FAILURE;
	*idx = obj->idx;
	return RTW_PHL_STATUS_SUCCESS;
}

void dispr_thread_stop_prior_hdl(struct cmd_dispatcher * obj)
{
	CLEAR_STATUS_FLAG(obj->status, DISPR_STARTED);
	cancel_all_cmd_req(obj);
	cancel_running_msg(obj);
}

void dispr_thread_stop_post_hdl(struct cmd_dispatcher * obj)
{
	void *d = phl_to_drvpriv(obj->phl_info);

	/* have to wait for bk thread ends before deinit msg & req*/
	deinit_dispr_msg_pool(obj);
	deinit_cmd_req_pool(obj);
	_os_atomic_set(d, &(obj->token_cnt), 0);
	_os_sema_free(d, &(obj->msg_q_sema));
}

enum rtw_phl_status dispr_init(struct phl_info_t *phl_info, void **dispr, u8 idx)
{
	struct cmd_dispatcher *obj = NULL;
	void *d = phl_to_drvpriv(phl_info);
	u8 i = 0;

	(*dispr) = NULL;

	obj = (struct cmd_dispatcher *)_os_mem_alloc(d, sizeof(struct cmd_dispatcher));
	if (obj == NULL) {
		PHL_ERR("%s, alloc fail\n", __FUNCTION__);
		return RTW_PHL_STATUS_RESOURCE;
	}

	obj->phl_info = phl_info;
	obj->idx = idx;
	_os_atomic_set(d, &(obj->token_cnt), 0);
	for (i = 0 ; i < PHL_MDL_PRI_MAX; i++)
		pq_init(d, &(obj->module_q[i]));

	(*dispr) = (void*)obj;
	_os_spinlock_init(d, &(obj->token_op_q_lock));
	SET_STATUS_FLAG(obj->status, DISPR_INIT);
	SET_STATUS_FLAG(obj->status, DISPR_NOTIFY_IDLE);
	PHL_INFO("%s, size dispr(%d), msg_ex(%d), req_ex(%d) \n",
		 __FUNCTION__, (int)sizeof(struct cmd_dispatcher),
		 (int)sizeof(struct phl_dispr_msg_ex),
		 (int)sizeof(struct phl_cmd_token_req_ex));
	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status dispr_deinit(struct phl_info_t *phl, void *dispr)
{
	struct cmd_dispatcher *obj = (struct cmd_dispatcher *)dispr;
	void *d = phl_to_drvpriv(obj->phl_info);
	u8 i = 0;

	if (!TEST_STATUS_FLAG(obj->status, DISPR_INIT))
		return RTW_PHL_STATUS_SUCCESS;
	dispr_stop(dispr);
	for (i = 0 ; i < PHL_MDL_PRI_MAX; i++)
		pq_deinit(d, &(obj->module_q[i]));
	_os_spinlock_free(d, &(obj->token_op_q_lock));
	_os_mem_free(d, obj, sizeof(struct cmd_dispatcher));
	PHL_INFO("%s\n", __FUNCTION__);
	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status dispr_start(void *dispr)
{
	struct cmd_dispatcher *obj = (struct cmd_dispatcher *)dispr;
	void *d = phl_to_drvpriv(obj->phl_info);

	if (TEST_STATUS_FLAG(obj->status, DISPR_STARTED))
		return RTW_PHL_STATUS_FAILURE;
	init_dispr_msg_pool(obj);
	init_cmd_req_pool(obj);
	init_dispr_mdl_mgnt_info(obj);
	_os_sema_init(d, &(obj->msg_q_sema), 0);
	if (disp_eng_is_solo_thread_mode(obj->phl_info)) {
		_os_thread_init(d, &(obj->bk_thread), background_thread_hdl, obj,
				"dispr_solo_thread");
		_os_thread_schedule(d, &(obj->bk_thread));
	}
	SET_STATUS_FLAG(obj->status, DISPR_STARTED);
	PHL_INFO("%s\n", __FUNCTION__);
	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status dispr_stop(void *dispr)
{
	struct cmd_dispatcher *obj = (struct cmd_dispatcher *)dispr;
	void *d = phl_to_drvpriv(obj->phl_info);

	if (!TEST_STATUS_FLAG(obj->status, DISPR_STARTED))
		return RTW_PHL_STATUS_FAILURE;

	dispr_thread_stop_prior_hdl(obj);
	if (disp_eng_is_solo_thread_mode(obj->phl_info)) {
		_os_thread_stop(d, &(obj->bk_thread));
		_os_sema_up(d, &(obj->msg_q_sema));
		_os_thread_deinit(d, &(obj->bk_thread));
	}
	dispr_thread_stop_post_hdl(obj);
	PHL_INFO("%s\n", __FUNCTION__);
	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status dispr_register_module(void *dispr,
					  enum phl_module_id id,
					  struct phl_bk_module_ops *ops)
{
	struct cmd_dispatcher *obj = (struct cmd_dispatcher *)dispr;
	void *d = phl_to_drvpriv(obj->phl_info);
	struct phl_bk_module *module = NULL;
	u8 ret = true;
	enum phl_bk_module_priority priority = _get_mdl_priority(id);

	FUNCIN();

	if (!TEST_STATUS_FLAG(obj->status, DISPR_INIT)  ||
	    priority == PHL_MDL_PRI_MAX ||
	    chk_module_ops(ops) == false ||
	    _chk_bitmap_bit(obj->bitmap, id) == true) {
		PHL_ERR("%s, register fail\n", __FUNCTION__);
		return RTW_PHL_STATUS_FAILURE;
	}

	module = (struct phl_bk_module *)_os_mem_alloc(d, sizeof(struct phl_bk_module));
	if (module == NULL) {
		PHL_ERR("%s, allocte fail\n", __FUNCTION__);
		return RTW_PHL_STATUS_FAILURE;
	}

	module->id = id;
	_os_mem_cpy(d, &(module->ops), ops, sizeof(struct phl_bk_module_ops));
	pq_push(d, &(obj->module_q[priority]), &(module->list), _tail, _bh);

	ret = bk_module_init(obj, module);
	if (ret == true && TEST_STATUS_FLAG(obj->status, DISPR_STARTED)) {
		ret = bk_module_start(obj, module);
		if (ret == true)
			_add_bitmap_bit(obj->bitmap, &(module->id), 1);
		if (ret == true && priority != PHL_MDL_PRI_OPTIONAL)
			_add_bitmap_bit(obj->basemap, &(module->id), 1);
	}
	PHL_INFO("%s id:%d, ret:%d\n",__FUNCTION__, id, ret);
	if (ret == true) {
		return RTW_PHL_STATUS_SUCCESS;
	} else {
		bk_module_deinit(obj, module);
		_os_mem_free(d, module, sizeof(struct phl_bk_module));
		return RTW_PHL_STATUS_FAILURE;
	}
}

enum rtw_phl_status dispr_deregister_module(void *dispr,
					    enum phl_module_id id)
{
	struct cmd_dispatcher *obj = (struct cmd_dispatcher *)dispr;
	void *d = phl_to_drvpriv(obj->phl_info);
	struct phl_bk_module *module = NULL;
	_os_list *mdl = NULL;
	enum rtw_phl_status phl_stat = RTW_PHL_STATUS_FAILURE;
	enum phl_bk_module_priority priority = _get_mdl_priority(id);

	FUNCIN();

	if (!TEST_STATUS_FLAG(obj->status, DISPR_INIT) ||
	    priority == PHL_MDL_PRI_MAX)
		return phl_stat;

	if(pq_search_node(d, &(obj->module_q[priority]), &mdl, _bh, true, &id, search_mdl)) {
		module = (struct phl_bk_module *)mdl;
		_clr_bitmap_bit(obj->bitmap, &(module->id), 1);
		_clr_bitmap_bit(obj->basemap, &(module->id), 1);
		bk_module_stop(obj, module);
		bk_module_deinit(obj, module);
		_os_mem_free(d, module, sizeof(struct phl_bk_module));
		phl_stat = RTW_PHL_STATUS_SUCCESS;
	}

	PHL_INFO("%s, id: %d stat:%d\n", __FUNCTION__, id, phl_stat);
	return phl_stat;
}

enum rtw_phl_status dispr_module_init(void *dispr)
{
	struct cmd_dispatcher *obj = (struct cmd_dispatcher *)dispr;
	void *d = phl_to_drvpriv(obj->phl_info);
	_os_list *mdl = NULL;
	u8 i = 0;

	if (!TEST_STATUS_FLAG(obj->status, DISPR_INIT))
		return RTW_PHL_STATUS_FAILURE;

	for (i = 0; i < PHL_MDL_PRI_MAX; i++) {
		if (pq_get_front(d, &(obj->module_q[i]), &mdl, _bh) == false)
			continue;
		do {
			bk_module_init(obj, (struct phl_bk_module *)mdl);
		} while(pq_get_next(d, &(obj->module_q[i]), mdl, &mdl, _bh));
	}
	PHL_INFO("%s\n", __FUNCTION__);
	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status dispr_module_deinit(void *dispr)
{
	struct cmd_dispatcher *obj = (struct cmd_dispatcher *)dispr;
	void *d = phl_to_drvpriv(obj->phl_info);
	_os_list *mdl = NULL;
	u8 i = 0;

	if (!TEST_STATUS_FLAG(obj->status, DISPR_INIT))
		return RTW_PHL_STATUS_FAILURE;

	for (i = 0; i < PHL_MDL_PRI_MAX; i++) {
		while (pq_pop(d, &(obj->module_q[i]), &mdl, _first, _bh)) {
			bk_module_deinit(obj, (struct phl_bk_module *)mdl);
			_os_mem_free(d, mdl, sizeof(struct phl_bk_module));
		}
	}
	PHL_INFO("%s\n", __FUNCTION__);
	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status dispr_module_start(void *dispr)
{
	struct cmd_dispatcher *obj = (struct cmd_dispatcher *)dispr;
	void *d = phl_to_drvpriv(obj->phl_info);
	_os_list *mdl = NULL;
	struct phl_bk_module *module = NULL;
	u8 i = 0;
	u8 ret = false;

	if (!TEST_STATUS_FLAG(obj->status, DISPR_STARTED))
		return RTW_PHL_STATUS_FAILURE;

	for (i = 0; i < PHL_MDL_PRI_MAX; i++) {
		if (pq_get_front(d, &(obj->module_q[i]), &mdl, _bh) == false)
			continue;
		do {
			module = (struct phl_bk_module*)mdl;
			ret = bk_module_start(obj, module);
			if (ret == true)
				_add_bitmap_bit(obj->bitmap, &(module->id), 1);
			if (ret == true && i != PHL_MDL_PRI_OPTIONAL)
				_add_bitmap_bit(obj->basemap, &(module->id), 1);
		} while(pq_get_next(d, &(obj->module_q[i]), mdl, &mdl, _bh));
	}
	PHL_INFO("%s\n", __FUNCTION__);
//_print_bitmap(obj->bitmap);
	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status dispr_module_stop(void *dispr)
{
	struct cmd_dispatcher *obj = (struct cmd_dispatcher *)dispr;
	void *d = phl_to_drvpriv(obj->phl_info);
	_os_list *mdl = NULL;
	struct phl_bk_module *module = NULL;
	u8 i = 0;

	if (!TEST_STATUS_FLAG(obj->status, DISPR_STARTED))
		return RTW_PHL_STATUS_FAILURE;

	for (i = 0; i < PHL_MDL_PRI_MAX; i++) {
		if (pq_get_front(d, &(obj->module_q[i]), &mdl, _bh) == false)
			continue;
		do {
			module = (struct phl_bk_module *)mdl;
			_clr_bitmap_bit(obj->bitmap, &(module->id), 1);
			_clr_bitmap_bit(obj->basemap, &(module->id), 1);
			bk_module_stop(obj, module);
		} while(pq_get_next(d, &(obj->module_q[i]), mdl, &mdl, _bh));
	}
	PHL_INFO("%s\n", __FUNCTION__);
//_print_bitmap(obj->bitmap);
	return RTW_PHL_STATUS_SUCCESS;
}

/**
 * phl_dispr_get_cur_cmd_req -- background module can call this function to
 * check cmd dispatcher is idle to know the risk or conflict for the I/O.
 * @dispr: dispatcher handler, get from phl_disp_eng_get_dispr_by_idx
 * @handle: get current cmd request, NULL means cmd dispatcher is idle

 * return RTW_PHL_STATUS_SUCCESS means cmd dispatcher is busy and can get
 * current cmd request from handle parameter
 * return RTW_PHL_STATUS_FAILURE means cmd dispatcher is idle
 */
enum rtw_phl_status
phl_dispr_get_cur_cmd_req(void *dispr, void **handle)
{
	struct cmd_dispatcher *obj = (struct cmd_dispatcher *)dispr;
	struct phl_cmd_token_req_ex *cur_req = NULL;
	enum rtw_phl_status phl_stat = RTW_PHL_STATUS_FAILURE;

	if (!TEST_STATUS_FLAG(obj->status, DISPR_INIT|DISPR_STARTED) || handle == NULL)
		return phl_stat;

	(*handle) = NULL;
	cur_req = obj->cur_cmd_req;

	if (cur_req == NULL ||
	    !TEST_STATUS_FLAG(cur_req->status, REQ_STATUS_RUN) ||
	    TEST_STATUS_FLAG(cur_req->status, REQ_STATUS_CANCEL))
		return phl_stat;

	*handle = (void *)cur_req;
	phl_stat = RTW_PHL_STATUS_SUCCESS;

	PHL_DBG("%s, req module id:%d phl_stat:%d\n", __FUNCTION__,
		 cur_req->req.module_id, phl_stat);
	return phl_stat;
}

enum rtw_phl_status
phl_dispr_set_cur_cmd_info(void *dispr,
			   struct phl_module_op_info *op_info)
{
	void *handle = NULL;
	struct phl_cmd_token_req_ex *cmd_req = NULL;
	struct phl_cmd_token_req *req = NULL;

	if (RTW_PHL_STATUS_SUCCESS != phl_dispr_get_cur_cmd_req(dispr, &handle))
		return RTW_PHL_STATUS_FAILURE;

	cmd_req = (struct phl_cmd_token_req_ex *)handle;
	req = &(cmd_req->req);

	PHL_INFO("%s, id:%d\n", __FUNCTION__, req->module_id);
	if (req->set_info(dispr, req->priv, op_info) == MDL_RET_SUCCESS)
		return RTW_PHL_STATUS_SUCCESS;
	else
		return RTW_PHL_STATUS_FAILURE;
}

#endif // if 0 NEO

enum rtw_phl_status
phl_dispr_query_cur_cmd_info(void *dispr,
			     struct phl_module_op_info *op_info)
{
	RTW_ERR("%s TODO NEO\n", __func__);
	return RTW_PHL_STATUS_FAILURE;
#if 0 // NEO TODO
	void *handle = NULL;
	struct phl_cmd_token_req_ex *cmd_req = NULL;
	struct phl_cmd_token_req *req = NULL;

	if (RTW_PHL_STATUS_SUCCESS != phl_dispr_get_cur_cmd_req(dispr, &handle))
		return RTW_PHL_STATUS_FAILURE;

	cmd_req = (struct phl_cmd_token_req_ex *)handle;
	req = &(cmd_req->req);

	PHL_DBG("%s, id:%d\n", __FUNCTION__, req->module_id);
	if (req->query_info(dispr, req->priv, op_info) == MDL_RET_SUCCESS)
		return RTW_PHL_STATUS_SUCCESS;
	else
		return RTW_PHL_STATUS_FAILURE;
#endif
}

#if 0 // NEO TODO

enum rtw_phl_status
phl_dispr_get_bk_module_handle(void *dispr,
			       enum phl_module_id id,
			       void **handle)
{
	struct cmd_dispatcher *obj = (struct cmd_dispatcher *)dispr;
	void *d = phl_to_drvpriv(obj->phl_info);
	_os_list *mdl = NULL;
	struct phl_bk_module *module = NULL;
	enum rtw_phl_status phl_stat = RTW_PHL_STATUS_FAILURE;
	enum phl_bk_module_priority priority = _get_mdl_priority(id);

	if (!TEST_STATUS_FLAG(obj->status, DISPR_INIT) ||
	    handle == NULL ||
	    priority == PHL_MDL_PRI_MAX ||
	    !_chk_bitmap_bit(obj->bitmap, id))
		return phl_stat;

	(*handle) = NULL;


	if(pq_search_node(d, &(obj->module_q[priority]), &mdl, _bh, false, &id, search_mdl)) {
		(*handle) = mdl;
		phl_stat = RTW_PHL_STATUS_SUCCESS;
	}
	PHL_INFO("%s, id:%d phl_stat:%d\n", __FUNCTION__, id, phl_stat);
	return phl_stat;
}

enum rtw_phl_status
phl_dispr_set_bk_module_info(void *dispr,
			     void *handle,
			     struct phl_module_op_info *op_info)
{
	struct phl_bk_module *module = (struct phl_bk_module *)handle;
	struct phl_bk_module_ops *ops = &(module->ops);

	if (!TEST_STATUS_FLAG(module->status, MDL_INIT))
		return RTW_PHL_STATUS_FAILURE;
	PHL_INFO("%s, id:%d\n", __FUNCTION__, module->id);
	if (ops->set_info(dispr, module->priv, op_info) == MDL_RET_SUCCESS)
		return RTW_PHL_STATUS_SUCCESS;
	else
		return RTW_PHL_STATUS_FAILURE;
}

enum rtw_phl_status
phl_dispr_query_bk_module_info(void *dispr,
			       void *handle,
			       struct phl_module_op_info *op_info)
{
	struct phl_bk_module *module = (struct phl_bk_module *)handle;
	struct phl_bk_module_ops *ops = &(module->ops);

	if (!TEST_STATUS_FLAG(module->status, MDL_INIT))
		return RTW_PHL_STATUS_FAILURE;
	PHL_INFO("%s, id:%d\n", __FUNCTION__, module->id);
	if (ops->query_info(dispr, module->priv, op_info) == MDL_RET_SUCCESS)
		return RTW_PHL_STATUS_SUCCESS;
	else
		return RTW_PHL_STATUS_FAILURE;
}

enum rtw_phl_status
phl_dispr_set_src_info(void *dispr,
		       struct phl_msg *msg,
		       struct phl_module_op_info *op_info)
{
	struct cmd_dispatcher *obj = (struct cmd_dispatcher *)dispr;
	enum rtw_phl_status phl_stat = RTW_PHL_STATUS_FAILURE;
	u8 id = MSG_MDL_ID_FIELD(msg->msg_id);
	struct phl_cmd_token_req_ex *cur_req = obj->cur_cmd_req;
	u8 i = 0;
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	struct phl_dispr_msg_ex *ex = (struct phl_dispr_msg_ex *)msg;
	u8 cur_req_id = get_cur_cmd_req_id(obj, NULL);

	if (!TEST_STATUS_FLAG(obj->status, DISPR_INIT) ||
	    (!_chk_bitmap_bit(obj->bitmap, id) &&
	    cur_req_id != id))
		return phl_stat;

	if (cur_req_id == id) {
		ret = cur_req->req.set_info(dispr, cur_req->req.priv, op_info);
	} else if (TEST_STATUS_FLAG(ex->status, MSG_STATUS_OWNER_BK_MDL)) {
		ret = ex->module->ops.set_info(dispr, ex->module->priv,
					       op_info);
	}
	PHL_INFO("%s, id:%d phl_stat:%d\n", __FUNCTION__, id, phl_stat);
	if (ret == MDL_RET_FAIL)
		return RTW_PHL_STATUS_FAILURE;
	else
		return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status
phl_dispr_query_src_info(void *dispr,
			 struct phl_msg *msg,
			 struct phl_module_op_info *op_info)
{
	struct cmd_dispatcher *obj = (struct cmd_dispatcher *)dispr;
	enum rtw_phl_status phl_stat = RTW_PHL_STATUS_FAILURE;
	u8 id = MSG_MDL_ID_FIELD(msg->msg_id);
	struct phl_cmd_token_req_ex *cur_req = obj->cur_cmd_req;
	u8 i = 0;
	struct phl_dispr_msg_ex *ex = (struct phl_dispr_msg_ex *)msg;
	enum phl_mdl_ret_code ret = MDL_RET_FAIL;
	u8 cur_req_id = get_cur_cmd_req_id(obj, NULL);

	if (!TEST_STATUS_FLAG(obj->status, DISPR_INIT) ||
	    (!_chk_bitmap_bit(obj->bitmap, id) &&
	    cur_req_id != id))
		return phl_stat;

	if (cur_req_id == id) {
		ret = cur_req->req.query_info(dispr, cur_req->req.priv, op_info);
	} else if (TEST_STATUS_FLAG(ex->status, MSG_STATUS_OWNER_BK_MDL)) {
		ret = ex->module->ops.query_info(dispr, ex->module->priv,
						 op_info);
	}
	PHL_INFO("%s, id:%d phl_stat:%d\n", __FUNCTION__, id, phl_stat);
	if (ret == MDL_RET_FAIL)
		return RTW_PHL_STATUS_FAILURE;
	else
		return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status
phl_dispr_send_msg(void *dispr,
		   struct phl_msg *msg,
		   struct phl_msg_attribute *attr,
		   u32 *msg_hdl)
{
	struct cmd_dispatcher *obj = (struct cmd_dispatcher *)dispr;
	void *d = phl_to_drvpriv(obj->phl_info);
	struct phl_dispr_msg_ex *msg_ex = NULL;
	u8 module_id = MSG_MDL_ID_FIELD(msg->msg_id); /* msg src */
	u32 req_status = 0;
	u8 cur_req_id = get_cur_cmd_req_id(obj, &req_status);

	if (!TEST_STATUS_FLAG(obj->status, DISPR_STARTED)) {
		return RTW_PHL_STATUS_FAILURE;
	}

	if(attr && attr->notify.id_arr == NULL && attr->notify.len) {
		PHL_ERR("%s attribute err\n",__FUNCTION__);
		return RTW_PHL_STATUS_INVALID_PARAM;
	}

	if (!IS_GENRAL_MODULE(module_id) &&
	    !_chk_bitmap_bit(obj->bitmap, module_id) &&
	    ((cur_req_id != PHL_MDL_ID_MAX  && cur_req_id != module_id) ||
	     (cur_req_id == PHL_MDL_ID_MAX && req_status == 0)||
	     (cur_req_id == PHL_MDL_ID_MAX && !TEST_STATUS_FLAG(req_status,REQ_STATUS_LAST_PERMIT)))) {
		PHL_ERR("%s module not allow to send\n", __FUNCTION__);
		return RTW_PHL_STATUS_INVALID_PARAM;
	}

	if (!pop_front_idle_msg(obj, &msg_ex)) {
		PHL_ERR("%s idle msg empty\n", __FUNCTION__);
		return RTW_PHL_STATUS_RESOURCE;
	}

	if (msg_hdl)
		*msg_hdl = 0;

	_os_mem_cpy(d, &msg_ex->msg, msg, sizeof(struct phl_msg));

	if (attr) {
		set_msg_bitmap(obj, msg_ex, attr->opt, module_id,
			       attr->notify.id_arr, attr->notify.len);
		if (attr->completion.completion) {
			SET_STATUS_FLAG(msg_ex->status, MSG_STATUS_NOTIFY_COMPLETE);
			msg_ex->completion.completion = attr->completion.completion;
			msg_ex->completion.priv = attr->completion.priv;
		}
		if (TEST_STATUS_FLAG(attr->opt, MSG_OPT_CLR_SNDR_MSG_IF_PENDING))
			SET_STATUS_FLAG(msg_ex->status, MSG_STATUS_CLR_SNDR_MSG_IF_PENDING);

		PHL_INFO("%s, opt:0x%x\n",__FUNCTION__, attr->opt);
	}

	if(TEST_STATUS_FLAG(req_status,REQ_STATUS_LAST_PERMIT) &&
	   (attr == NULL || !TEST_STATUS_FLAG(attr->opt, MSG_OPT_SEND_IN_ABORT))) {
		PHL_ERR("%s msg not allow since cur req is going to unload\n", __FUNCTION__);
		SET_MSG_INDC_FIELD(msg_ex->msg.msg_id, MSG_INDC_FAIL);
		push_back_idle_msg(obj, msg_ex);
		return RTW_PHL_STATUS_FAILURE;
	}

	if (cur_req_id == module_id) {
		SET_STATUS_FLAG(msg_ex->status, MSG_STATUS_OWNER_REQ);
	} else if (get_module_by_id(obj, module_id, &(msg_ex->module)) == true) {
		SET_STATUS_FLAG(msg_ex->status, MSG_STATUS_OWNER_BK_MDL);
		PHL_INFO("%s module(%d) found\n", __FUNCTION__, module_id);
	}

	SET_STATUS_FLAG(msg_ex->status, MSG_STATUS_PRE_PHASE);

	push_back_wait_msg(obj, msg_ex);

	PHL_INFO("%s, status:0x%x\n",__FUNCTION__, msg_ex->status);
	if(msg_hdl)
		*msg_hdl = GEN_VALID_HDL(msg_ex->idx);
	PHL_INFO("%s, msg_id:0x%x\n", __FUNCTION__, msg->msg_id);
	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status phl_dispr_cancel_msg(void *dispr, u32 *msg_hdl)
{
	struct cmd_dispatcher *obj = (struct cmd_dispatcher *)dispr;
	void *d = phl_to_drvpriv(obj->phl_info);
	struct phl_dispr_msg_ex *msg_ex = NULL;

	if (!TEST_STATUS_FLAG(obj->status, DISPR_STARTED) || msg_hdl == NULL)
		return RTW_PHL_STATUS_FAILURE;
	if (!IS_HDL_VALID(*msg_hdl) ||
	    GET_IDX_FROM_HDL(*msg_hdl) >= MAX_PHL_MSG_NUM) {
		PHL_ERR("%s, HDL invalid\n", __FUNCTION__);
		return RTW_PHL_STATUS_FAILURE;
	}

	msg_ex = &(obj->msg_ex_pool[GET_IDX_FROM_HDL(*msg_hdl)]);
	*msg_hdl = 0;
	if (!TEST_STATUS_FLAG(msg_ex->status, MSG_STATUS_ENQ) &&
	    !TEST_STATUS_FLAG(msg_ex->status, MSG_STATUS_RUN)) {
		PHL_ERR("%s, HDL status err\n", __FUNCTION__);
		return RTW_PHL_STATUS_FAILURE;
	}

	cancel_msg(obj, msg_ex);
	PHL_INFO("%s\n", __FUNCTION__);
	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status phl_dispr_clr_pending_msg(void *dispr)
{
	struct cmd_dispatcher *obj = (struct cmd_dispatcher *)dispr;
	void *d = phl_to_drvpriv(obj->phl_info);

	SET_STATUS_FLAG(obj->status, DISPR_CLR_PEND_MSG);
	notify_bk_thread(obj);
	PHL_INFO("%s\n", __FUNCTION__);
	return RTW_PHL_STATUS_SUCCESS;
}
enum rtw_phl_status
phl_dispr_add_token_req(void *dispr,
			struct phl_cmd_token_req *req,
			u32 *req_hdl)
{
	struct cmd_dispatcher *obj = (struct cmd_dispatcher *)dispr;
	void *d = phl_to_drvpriv(obj->phl_info);
	struct phl_cmd_token_req_ex *req_ex = NULL;
	enum rtw_phl_status stat = RTW_PHL_STATUS_SUCCESS;
	_os_list *node = NULL;

	if (!TEST_STATUS_FLAG(obj->status, DISPR_STARTED) ||
	    req_hdl == NULL ||
	    chk_cmd_req_ops(req) == false)
		return RTW_PHL_STATUS_FAILURE;

	if (!pop_front_idle_req(obj, &req_ex)) {
		PHL_ERR("%s idle req empty\n", __FUNCTION__);
		return RTW_PHL_STATUS_RESOURCE;
	}
	_os_mem_cpy(d, &(req_ex->req), req, sizeof(struct phl_cmd_token_req));

	push_back_wait_req(obj, req_ex);
	*req_hdl = GEN_VALID_HDL(req_ex->idx);
	PHL_INFO("%s, id:%d, hdl:0x%x token_cnt:%d\n", __FUNCTION__,
		 req->module_id,
		 *req_hdl,
		 _os_atomic_read(d, &(obj->token_cnt)));

	if (pq_get_front(d, &(obj->token_op_q), &node, _bh) == false &&
	    _os_atomic_read(d, &(obj->token_cnt)) == 0)
		stat = RTW_PHL_STATUS_SUCCESS;
	else
		stat = RTW_PHL_STATUS_PENDING;
	dispr_enqueue_token_op_info(obj, &req_ex->add_req_info, TOKEN_OP_ADD_CMD_REQ, req_ex->idx);
	return stat;
}

enum rtw_phl_status phl_dispr_cancel_token_req(void *dispr, u32 *req_hdl)
{
	struct cmd_dispatcher *obj = (struct cmd_dispatcher *)dispr;
	void *d = phl_to_drvpriv(obj->phl_info);
	struct phl_cmd_token_req_ex *req_ex = NULL;

	if (!TEST_STATUS_FLAG(obj->status, DISPR_STARTED) || req_hdl == NULL)
		return RTW_PHL_STATUS_FAILURE;

	if (!IS_HDL_VALID(*req_hdl) ||
	    GET_IDX_FROM_HDL(*req_hdl) >= MAX_CMD_REQ_NUM) {
		PHL_ERR("%s, HDL(0x%x) invalid\n", __FUNCTION__, *req_hdl);
		return RTW_PHL_STATUS_FAILURE;
	}
	req_ex = &(obj->token_req_ex_pool[GET_IDX_FROM_HDL(*req_hdl)]);
	if (!TEST_STATUS_FLAG(req_ex->status, REQ_STATUS_ENQ) &&
	    !TEST_STATUS_FLAG(req_ex->status, REQ_STATUS_RUN) &&
	    !TEST_STATUS_FLAG(req_ex->status, REQ_STATUS_PREPARE)) {
		PHL_ERR("%s, HDL(0x%x) status err\n", __FUNCTION__, *req_hdl);
		return RTW_PHL_STATUS_FAILURE;
	}
	SET_STATUS_FLAG(req_ex->status, REQ_STATUS_CANCEL);
	if (dispr_enqueue_token_op_info(obj, &req_ex->free_req_info, TOKEN_OP_CANCEL_CMD_REQ, req_ex->idx))
		return RTW_PHL_STATUS_SUCCESS;
	else
		return RTW_PHL_STATUS_FAILURE;
}

enum rtw_phl_status phl_dispr_free_token(void *dispr, u32 *req_hdl)
{
	struct cmd_dispatcher *obj = (struct cmd_dispatcher *)dispr;
	void *d = phl_to_drvpriv(obj->phl_info);
	struct phl_cmd_token_req_ex *req_ex = NULL;

	if (!TEST_STATUS_FLAG(obj->status, DISPR_STARTED) || req_hdl == NULL)
		return RTW_PHL_STATUS_FAILURE;

	if (obj->cur_cmd_req == NULL ||
	    _os_atomic_read(d, &(obj->token_cnt)) == 0  ||
	    !IS_HDL_VALID(*req_hdl) ||
	    GET_IDX_FROM_HDL(*req_hdl) >= MAX_CMD_REQ_NUM) {
		PHL_ERR("%s, HDL(0x%x) invalid\n", __FUNCTION__, *req_hdl);
		return RTW_PHL_STATUS_FAILURE;
	}
	req_ex = &(obj->token_req_ex_pool[GET_IDX_FROM_HDL(*req_hdl)]);
	if (!TEST_STATUS_FLAG(req_ex->status, REQ_STATUS_RUN) &&
	    !TEST_STATUS_FLAG(req_ex->status, REQ_STATUS_PREPARE)) {
		PHL_ERR("%s, HDL(0x%x) mismatch\n", __FUNCTION__, *req_hdl);
		return RTW_PHL_STATUS_FAILURE;
	}
	SET_STATUS_FLAG(req_ex->status, REQ_STATUS_CANCEL);
	if (dispr_enqueue_token_op_info(obj, &req_ex->free_req_info, TOKEN_OP_FREE_CMD_REQ, req_ex->idx))
		return RTW_PHL_STATUS_SUCCESS;
	else
		return RTW_PHL_STATUS_FAILURE;
}

enum rtw_phl_status dispr_process_token_req(struct cmd_dispatcher *obj)
{
	void *d = phl_to_drvpriv(obj->phl_info);
	struct phl_cmd_token_req_ex *ex = NULL;

	if (!TEST_STATUS_FLAG(obj->status, DISPR_STARTED) ||
	    _os_atomic_read(d, &(obj->token_cnt)) > 0)
		return RTW_PHL_STATUS_FAILURE;

	do {
		if (pop_front_wait_req(obj, &ex) == false) {
			if (!TEST_STATUS_FLAG(obj->status, DISPR_NOTIFY_IDLE)) {
				SET_STATUS_FLAG(obj->status, DISPR_NOTIFY_IDLE);
				send_bk_msg_phy_idle(obj);
			}
			return RTW_PHL_STATUS_SUCCESS;
		}

		if (TEST_STATUS_FLAG(obj->status, DISPR_NOTIFY_IDLE)) {
			CLEAR_STATUS_FLAG(obj->status, DISPR_NOTIFY_IDLE);
			send_bk_msg_phy_on(obj);
		}

	}while(!register_cur_cmd_req(obj, ex));

	return RTW_PHL_STATUS_SUCCESS;
}

void dispr_share_thread_loop_hdl(void *dispr)
{
	dispr_thread_loop_hdl( (struct cmd_dispatcher *)dispr);
}

void dispr_share_thread_leave_hdl(void *dispr)
{
	dispr_thread_leave_hdl((struct cmd_dispatcher *)dispr);
}

void dispr_share_thread_stop_prior_hdl(void *dispr)
{
	dispr_thread_stop_prior_hdl((struct cmd_dispatcher *)dispr);
}

void dispr_share_thread_stop_post_hdl(void *dispr)
{
	dispr_thread_stop_post_hdl((struct cmd_dispatcher *)dispr);
}
void send_bk_msg_phy_on(struct cmd_dispatcher *obj)
{
	struct phl_msg msg = {0};
	struct phl_msg_attribute attr = {0};

	SET_MSG_MDL_ID_FIELD(msg.msg_id, PHL_MDL_PHY_MGNT);
	SET_MSG_EVT_ID_FIELD(msg.msg_id, MSG_EVT_PHY_ON);
	phl_dispr_send_msg((void*)obj, &msg, &attr, NULL);
}

void send_bk_msg_phy_idle(struct cmd_dispatcher *obj)
{
	struct phl_msg msg = {0};
	struct phl_msg_attribute attr = {0};

	SET_MSG_MDL_ID_FIELD(msg.msg_id, PHL_MDL_PHY_MGNT);
	SET_MSG_EVT_ID_FIELD(msg.msg_id, MSG_EVT_PHY_IDLE);
	phl_dispr_send_msg((void*)obj, &msg, &attr, NULL);
}

#endif // if 0 NEO

#endif
