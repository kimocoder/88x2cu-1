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
#define _PHL_PHY_MGNT_C_
#include "phl_headers.h"
#ifdef CONFIG_CMD_DISP

enum rtw_phl_status phl_disp_eng_bk_module_deinit(struct phl_info_t *phl);
int share_thread_hdl(void *param)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)param;
	void *d = phl_to_drvpriv(phl_info);
	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);
	u8 i = 0;

	PHL_INFO("%s enter\n", __FUNCTION__);
	while (!_os_thread_check_stop(d, &(disp_eng->share_thread))) {

		_os_sema_down(d, &disp_eng->msg_q_sema);

		/* A simple for-loop would guarantee
		 * all dispatcher split entire bandwidth of shared thread evenly,
		 * if adopting FIFO rule here,
		 * would lead to disproportionate distribution of thread bandwidth.
		*/
		for (i = 0 ; i < disp_eng->phy_num; i++) {
			if(_os_thread_check_stop(d, &(disp_eng->share_thread)))
				break;
			dispr_share_thread_loop_hdl(disp_eng->dispatcher[i]);
		}
	}
	for (i = 0 ; i < disp_eng->phy_num; i++)
		dispr_share_thread_leave_hdl(disp_eng->dispatcher[i]);
	_os_thread_wait_stop(d, &(disp_eng->share_thread));
	PHL_INFO("%s down\n", __FUNCTION__);
	return 0;
}
enum rtw_phl_status phl_disp_eng_init(struct phl_info_t *phl, u8 phy_num)
{
	u8 i = 0;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl->disp_eng);
	void *d = phl_to_drvpriv(phl);
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;

	if (disp_eng->dispatcher != NULL) {
		PHL_ERR("[PHY]: %s, not empty\n",__FUNCTION__);
		return RTW_PHL_STATUS_FAILURE;
	}

	disp_eng->phl_info = phl;
	disp_eng->phy_num = phy_num;
#ifdef CONFIG_CMD_DISP_SOLO_MODE
	disp_eng->thread_mode = SOLO_THREAD_MODE;
#else
	disp_eng->thread_mode = SHARE_THREAD_MODE;
#endif
	disp_eng->dispatcher = _os_mem_alloc(d, sizeof(void*) * phy_num);

	if (disp_eng->dispatcher == NULL) {
		disp_eng->phy_num = 0;
		PHL_ERR("[PHY]: %s, alloc fail\n",__FUNCTION__);
		return RTW_PHL_STATUS_RESOURCE;
	}
	for (i = 0 ; i < phy_num; i++) {
		status = dispr_init(phl, &(disp_eng->dispatcher[i]), i);
		if(status != RTW_PHL_STATUS_SUCCESS)
			break;
	}

	if (status != RTW_PHL_STATUS_SUCCESS)
		phl_disp_eng_deinit(phl);

	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status phl_disp_eng_deinit(struct phl_info_t *phl)
{
	u8 i = 0;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl->disp_eng);
	void *d = phl_to_drvpriv(phl);

	if (disp_eng->dispatcher == NULL)
		return RTW_PHL_STATUS_FAILURE;

	phl_disp_eng_bk_module_deinit(phl);

	for (i = 0 ; i < disp_eng->phy_num; i++) {
		if(disp_eng->dispatcher[i] == NULL)
			continue;
		dispr_deinit(phl, disp_eng->dispatcher[i]);
		disp_eng->dispatcher[i] = NULL;
	}

	if (disp_eng->phy_num) {
		_os_mem_free(d, disp_eng->dispatcher,
				sizeof(void *) * (disp_eng->phy_num));
		disp_eng->dispatcher = NULL;
		disp_eng->phy_num = 0;
	}

	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status phl_disp_eng_bk_module_deinit(struct phl_info_t *phl)
{
	u8 i = 0;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl->disp_eng);

	for (i = 0 ; i < disp_eng->phy_num; i++) {
		if(disp_eng->dispatcher[i] == NULL)
			continue;
		dispr_module_deinit(disp_eng->dispatcher[i]);
	}

	return RTW_PHL_STATUS_SUCCESS;
}


enum rtw_phl_status phl_disp_eng_start(struct phl_info_t *phl)
{
	u8 i = 0;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl->disp_eng);
	void *d = phl_to_drvpriv(phl);

	_os_sema_init(d, &(disp_eng->msg_q_sema), 0);
	if (!disp_eng_is_solo_thread_mode(phl)) {
		_os_thread_init(d, &(disp_eng->share_thread), share_thread_hdl, phl,
				"disp_eng_share_thread");
		_os_thread_schedule(d, &(disp_eng->share_thread));
	}
	for (i = 0 ; i < disp_eng->phy_num; i++){
		if(disp_eng->dispatcher[i] == NULL)
			continue;
		dispr_start(disp_eng->dispatcher[i]);
		dispr_module_start(disp_eng->dispatcher[i]);
	}

	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status phl_disp_eng_stop(struct phl_info_t *phl)
{
	u8 i = 0;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl->disp_eng);
	void *d = phl_to_drvpriv(phl);
	u8 solo_mode = (disp_eng_is_solo_thread_mode(phl)) ? (true) : (false);

	if (disp_eng->dispatcher == NULL) {
		PHL_ERR("[PHY]: %s, abnomarl state\n",__FUNCTION__);
		return RTW_PHL_STATUS_SUCCESS;
	}

	for (i = 0 ; i < disp_eng->phy_num; i++) {
		if(disp_eng->dispatcher[i] == NULL)
			continue;
		dispr_module_stop(disp_eng->dispatcher[i]);
		if (solo_mode == true)
			dispr_stop(disp_eng->dispatcher[i]);
		else
			dispr_share_thread_stop_prior_hdl(disp_eng->dispatcher[i]);
	}

	if (solo_mode == false) {
		_os_thread_stop(d, &(disp_eng->share_thread));
		_os_sema_up(d, &(disp_eng->msg_q_sema));
		_os_thread_deinit(d, &(disp_eng->share_thread));

		for (i = 0 ; i < disp_eng->phy_num; i++)
			dispr_share_thread_stop_post_hdl(disp_eng->dispatcher[i]);
	}
	_os_sema_free(d, &(disp_eng->msg_q_sema));
	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status
rtw_phl_register_module(void *phl, u8 band_idx, enum phl_module_id id,
			struct phl_bk_module_ops *ops)
{
	return phl_disp_eng_register_module((struct phl_info_t *)phl,
					    band_idx, id, ops);
}

enum rtw_phl_status
rtw_phl_deregister_module(void *phl,u8 band_idx, enum phl_module_id id)
{
	return phl_disp_eng_deregister_module((struct phl_info_t *)phl,
					      band_idx, id);
}

enum rtw_phl_status
rtw_phl_send_msg_to_dispr(void *phl, u8 band_idx,
			  struct phl_msg *msg,
			  struct phl_msg_attribute *attr,
			  u32 *msg_hdl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	u8 idx = band_idx;

	if ((band_idx + 1) > disp_eng->phy_num) {
		PHL_WARN("%s invalid input :%d\n", __func__, band_idx);
		return RTW_PHL_STATUS_INVALID_PARAM;
	}

	return phl_dispr_send_msg(disp_eng->dispatcher[idx],
				  msg, attr, msg_hdl);
}

enum rtw_phl_status
rtw_phl_cancel_dispr_msg(void *phl, u8 band_idx, u32 *msg_hdl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	u8 idx = band_idx;

	if ((band_idx + 1) > disp_eng->phy_num) {
		PHL_WARN("%s invalid input :%d\n", __func__, band_idx);
		return RTW_PHL_STATUS_INVALID_PARAM;
	}

	return phl_dispr_cancel_msg(disp_eng->dispatcher[idx], msg_hdl);
}

enum rtw_phl_status
rtw_phl_add_cmd_token_req(void *phl, u8 band_idx,
			  struct phl_cmd_token_req *req,
			  u32 *req_hdl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	u8 idx = band_idx;

	if ((band_idx + 1) > disp_eng->phy_num) {
		PHL_WARN("%s invalid input :%d\n", __func__, band_idx);
		return RTW_PHL_STATUS_INVALID_PARAM;
	}

	return phl_dispr_add_token_req(disp_eng->dispatcher[idx],
				       req, req_hdl);
}

enum rtw_phl_status
rtw_phl_cancel_cmd_token(void *phl, u8 band_idx, u32 *req_hdl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	u8 idx = band_idx;

	if ((band_idx + 1) > disp_eng->phy_num) {
		PHL_WARN("%s invalid input :%d\n", __func__, band_idx);
		return RTW_PHL_STATUS_INVALID_PARAM;
	}

	return phl_dispr_cancel_token_req(disp_eng->dispatcher[idx],
					  req_hdl);
}

enum rtw_phl_status
rtw_phl_free_cmd_token(void *phl, u8 band_idx, u32 *req_hdl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	u8 idx = band_idx;

	if ((band_idx + 1) > disp_eng->phy_num) {
		PHL_WARN("%s invalid input :%d\n", __func__, band_idx);
		return RTW_PHL_STATUS_INVALID_PARAM;
	}

	return phl_dispr_free_token(disp_eng->dispatcher[idx], req_hdl);
}

enum rtw_phl_status rtw_phl_query_cur_cmd_info(void *phl, u8 band_idx,
					       struct phl_module_op_info* op_info)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);
	u8 idx = band_idx;

	if ((band_idx + 1) > disp_eng->phy_num) {
		PHL_WARN("%s invalid input :%d\n", __func__, band_idx);
		return RTW_PHL_STATUS_INVALID_PARAM;
	}

	return phl_dispr_query_cur_cmd_info(disp_eng->dispatcher[idx],
					    op_info);
}

enum rtw_phl_status rtw_phl_set_bk_module_info(void *phl, u8 band_idx,
		enum phl_module_id id, struct phl_module_op_info *op_info)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);
	void *handle = NULL;
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	u8 idx = band_idx;

	if ((band_idx + 1) > disp_eng->phy_num) {
		PHL_WARN("%s invalid input :%d\n", __func__, band_idx);
		return RTW_PHL_STATUS_INVALID_PARAM;
	}

	status = phl_dispr_get_bk_module_handle(disp_eng->dispatcher[idx],
						id, &handle);
	if(status != RTW_PHL_STATUS_SUCCESS)
		return RTW_PHL_STATUS_FAILURE;

	return phl_dispr_set_bk_module_info(disp_eng->dispatcher[idx],
						handle, op_info);
}
enum rtw_phl_status rtw_phl_query_bk_module_info(void *phl, u8 band_idx,
		enum phl_module_id id, struct phl_module_op_info *op_info)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_cmd_dispatch_engine *disp_eng = &(phl_info->disp_eng);
	void *handle = NULL;
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	u8 idx = band_idx;

	if ((band_idx + 1) > disp_eng->phy_num) {
		PHL_WARN("%s invalid input :%d\n", __func__, band_idx);
		return RTW_PHL_STATUS_INVALID_PARAM;
	}

	status = phl_dispr_get_bk_module_handle(disp_eng->dispatcher[idx],
						id, &handle);
	if (status != RTW_PHL_STATUS_SUCCESS)
		return RTW_PHL_STATUS_FAILURE;

	return phl_dispr_query_bk_module_info(disp_eng->dispatcher[idx],
					      handle, op_info);
}
enum rtw_phl_status
phl_disp_eng_get_dispr_by_idx(struct phl_info_t *phl, u8 band_idx, void **dispr)
{
	struct phl_cmd_dispatch_engine *disp_eng = &(phl->disp_eng);
	u8 idx = band_idx;

	if ((band_idx + 1) > disp_eng->phy_num) {
		PHL_WARN("%s invalid input :%d\n", __func__, band_idx);
		return RTW_PHL_STATUS_INVALID_PARAM;
	}

	if (dispr == NULL) {
		PHL_ERR("%s invalid dispr\n", __func__);
		return RTW_PHL_STATUS_FAILURE;
	}
	(*dispr) = disp_eng->dispatcher[idx];
	return RTW_PHL_STATUS_SUCCESS;
}
enum rtw_phl_status phl_disp_eng_register_module(struct phl_info_t *phl,
						 u8 band_idx,
						 enum phl_module_id id,
						 struct phl_bk_module_ops *ops)
{
	struct phl_cmd_dispatch_engine *disp_eng = &(phl->disp_eng);
	u8 idx = band_idx;

	if ((band_idx + 1) > disp_eng->phy_num) {
		PHL_WARN("%s invalid input :%d\n", __func__, band_idx);
		return RTW_PHL_STATUS_INVALID_PARAM;
	}

	return dispr_register_module(disp_eng->dispatcher[idx], id, ops);
}

enum rtw_phl_status phl_disp_eng_deregister_module(struct phl_info_t *phl,
						   u8 band_idx,
						   enum phl_module_id id)
{
	struct phl_cmd_dispatch_engine *disp_eng = &(phl->disp_eng);
	u8 idx = band_idx;

	if ((band_idx + 1) > disp_eng->phy_num) {
		PHL_WARN("%s invalid input :%d\n", __func__, band_idx);
		return RTW_PHL_STATUS_INVALID_PARAM;
	}

	return dispr_deregister_module(disp_eng->dispatcher[idx], id);
}
void disp_eng_notify_share_thread(struct phl_info_t *phl, void *dispr)
{
	void *d = phl_to_drvpriv(phl);
	struct phl_cmd_dispatch_engine *disp_eng = &(phl->disp_eng);

	_os_sema_up(d, &(disp_eng->msg_q_sema));
}
#else
enum rtw_phl_status phl_disp_eng_init(struct phl_info_t *phl, u8 phy_num)
{
	return RTW_PHL_STATUS_SUCCESS;
}
enum rtw_phl_status phl_disp_eng_deinit(struct phl_info_t *phl)
{
	return RTW_PHL_STATUS_SUCCESS;
}
enum rtw_phl_status phl_disp_eng_start(struct phl_info_t *phl)
{
	return RTW_PHL_STATUS_SUCCESS;
}
enum rtw_phl_status phl_disp_eng_stop(struct phl_info_t *phl)
{
	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status phl_disp_eng_register_module(struct phl_info_t *phl,
						 u8 band_idx,
						 enum phl_module_id id,
						 struct phl_bk_module_ops *ops)
{
	return RTW_PHL_STATUS_FAILURE;
}

enum rtw_phl_status phl_disp_eng_deregister_module(struct phl_info_t *phl,
						   u8 band_idx,
						   enum phl_module_id id)
{
	return RTW_PHL_STATUS_FAILURE;
}
enum rtw_phl_status phl_disp_eng_get_dispr_by_idx(struct phl_info_t *phl,
						  u8 band_idx, void **dispr)
{
	return RTW_PHL_STATUS_FAILURE;
}
enum rtw_phl_status phl_dispr_get_idx(void *dispr, u8 *idx)
{
	return RTW_PHL_STATUS_FAILURE;
}

/* use phl_disp_eng_get_dispr_by_idx to get valid dispr handle first */
enum rtw_phl_status phl_dispr_get_cur_cmd_req(void *dispr, void **handle)
{
	return RTW_PHL_STATUS_FAILURE;
}
enum rtw_phl_status phl_dispr_set_cur_cmd_info(void *dispr,
					       struct phl_module_op_info *op_info)
{
	return RTW_PHL_STATUS_FAILURE;
}
enum rtw_phl_status phl_dispr_query_cur_cmd_info(void *dispr,
						 struct phl_module_op_info *op_info)
{
	return RTW_PHL_STATUS_FAILURE;
}
enum rtw_phl_status phl_dispr_get_bk_module_handle(void *dispr,
						   enum phl_module_id id,
						   void **handle)
{
	return RTW_PHL_STATUS_FAILURE;
}
enum rtw_phl_status phl_dispr_set_bk_module_info(void *dispr, void *handle,
						 struct phl_module_op_info *op_info)
{
	return RTW_PHL_STATUS_FAILURE;
}
enum rtw_phl_status phl_dispr_query_bk_module_info(void *dispr, void *handle,
						   struct phl_module_op_info *op_info)
{
	return RTW_PHL_STATUS_FAILURE;
}
enum rtw_phl_status phl_dispr_set_src_info(void *dispr, struct phl_msg *msg,
					   struct phl_module_op_info *op_info)
{
	return RTW_PHL_STATUS_FAILURE;
}
enum rtw_phl_status phl_dispr_query_src_info(void *dispr, struct phl_msg *msg,
					     struct phl_module_op_info *op_info)
{
	return RTW_PHL_STATUS_FAILURE;
}
enum rtw_phl_status phl_dispr_send_msg(void *dispr, struct phl_msg *msg,
				       struct phl_msg_attribute *attr, u32 *msg_hdl)
{
	return RTW_PHL_STATUS_FAILURE;
}
enum rtw_phl_status phl_dispr_cancel_msg(void *dispr, u32 *msg_hdl)
{
	return RTW_PHL_STATUS_FAILURE;
}
enum rtw_phl_status phl_dispr_clr_pending_msg(void *dispr)
{
	return RTW_PHL_STATUS_FAILURE;
}

enum rtw_phl_status phl_dispr_add_token_req(void *dispr,
					    struct phl_cmd_token_req *req, u32 *req_hdl)
{
	return RTW_PHL_STATUS_FAILURE;
}
enum rtw_phl_status phl_dispr_cancel_token_req(void *dispr, u32 *req_hdl)
{
	return RTW_PHL_STATUS_FAILURE;
}
enum rtw_phl_status phl_dispr_free_token(void *dispr, u32 *req_hdl)
{
	return RTW_PHL_STATUS_FAILURE;
}
enum rtw_phl_status rtw_phl_query_cur_cmd_info(void *phl, u8 band_idx,
					       struct phl_module_op_info* op_info)
{
	return RTW_PHL_STATUS_FAILURE;
}

enum rtw_phl_status rtw_phl_set_bk_module_info(void *phl, u8 band_idx,
		enum phl_module_id id, struct phl_module_op_info *op_info)
{
	return RTW_PHL_STATUS_FAILURE;
}
enum rtw_phl_status rtw_phl_query_bk_module_info(void *phl, u8 band_idx,
		enum phl_module_id id, struct phl_module_op_info *op_info)
{
	return RTW_PHL_STATUS_FAILURE;
}
#endif
