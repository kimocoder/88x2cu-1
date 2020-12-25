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
#define _RTW_TRX_C_
#include <drv_types.h>		/* struct dvobj_priv and etc. */


struct lite_data_buf *rtw_alloc_litedatabuf(struct trx_data_buf_q *data_buf_q)
{
	struct lite_data_buf *litedatabuf =  NULL;
	_list *list, *head;
	_queue *free_litedatabuf_q = &data_buf_q->free_data_buf_queue;
	unsigned long sp_flags;

	/* RTW_INFO("+rtw_alloc_litexmitbuf\n"); */

	_rtw_spinlock_irq(&free_litedatabuf_q->lock, &sp_flags);

	if (_rtw_queue_empty(free_litedatabuf_q) == _TRUE)
		litedatabuf = NULL;
	else {

		head = get_list_head(free_litedatabuf_q);

		list = get_next(head);

		litedatabuf = LIST_CONTAINOR(list,
			struct lite_data_buf, list);

		rtw_list_delete(&(litedatabuf->list));
	}

	if (litedatabuf !=  NULL) {
		data_buf_q->free_data_buf_cnt--;


		if (litedatabuf->sctx) {
			RTW_INFO("%s plitexmitbuf->sctx is not NULL\n",
				__func__);
			rtw_sctx_done_err(&litedatabuf->sctx,
				RTW_SCTX_DONE_BUF_ALLOC);
		}
	}

	_rtw_spinunlock_irq(&free_litedatabuf_q->lock, &sp_flags);


	return litedatabuf;
}

s32 rtw_free_litedatabuf(struct trx_data_buf_q *data_buf_q,
		struct lite_data_buf *lite_data_buf)
{
	_queue *free_litedatabuf_q = &data_buf_q->free_data_buf_queue;
	unsigned long sp_flags;

	/* RTW_INFO("+rtw_free_litexmitbuf\n"); */

	if (data_buf_q == NULL)
		return _FAIL;

	if (lite_data_buf == NULL)
		return _FAIL;

	lite_data_buf->pbuf = NULL;
	lite_data_buf->phl_buf_ptr = NULL;
#ifdef CONFIG_USB_HCI
	lite_data_buf->dataurb = NULL;
#endif

	if (lite_data_buf->sctx) {
		RTW_INFO("%s lite_data_buf->sctx is not NULL\n", __func__);
		rtw_sctx_done_err(&lite_data_buf->sctx, RTW_SCTX_DONE_BUF_FREE);
		return _FAIL;
	}

	_rtw_spinlock_irq(&free_litedatabuf_q->lock, &sp_flags);

	rtw_list_delete(&lite_data_buf->list);

	rtw_list_insert_tail(&(lite_data_buf->list),
		get_list_head(free_litedatabuf_q));

	data_buf_q->free_data_buf_cnt++;

	_rtw_spinunlock_irq(&free_litedatabuf_q->lock, &sp_flags);


	return _SUCCESS;
}

