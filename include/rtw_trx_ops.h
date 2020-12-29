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
#ifndef _RTW_TRX_OPS_H_
#define _RTW_TRX_OPS_H_
#include <drv_types.h>


/************************ recv *******************/
static inline s32 rtw_intf_init_recv_priv(struct dvobj_priv *dvobj)
{
	return dvobj->intf_ops->init_recv_priv(dvobj);
}
static inline void rtw_intf_free_recv_priv(struct dvobj_priv *dvobj)
{
	return dvobj->intf_ops->free_recv_priv(dvobj);
}

#ifdef CONFIG_RECV_THREAD_MODE
static inline s32 rtw_intf_recv_hdl(_adapter *adapter)
{
	return adapter_to_dvobj(adapter)->intf_ops->recv_hdl(adapter);
}
#endif

struct lite_data_buf *rtw_alloc_litedatabuf(struct trx_data_buf_q *data_buf_q);
s32 rtw_free_litedatabuf(struct trx_data_buf_q *data_buf_q,
		struct lite_data_buf *lite_data_buf);

#endif /* _RTW_TRX_OPS_H_ */
