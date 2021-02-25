/******************************************************************************
 *
 * Copyright(c) 2007 - 2017 Realtek Corporation.
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
#define _HAL_USB_C_

#include <drv_types.h>
#include <hal_data.h>

int	usb_init_recv_priv(_adapter *padapter, u16 ini_in_buf_sz)
{
	struct registry_priv *regsty = adapter_to_regsty(padapter);
	struct recv_priv	*precvpriv = &adapter_to_dvobj(padapter)->recvpriv;
	int	i, res = _SUCCESS;
	struct recv_buf *precvbuf;

#ifdef PLATFORM_FREEBSD
#ifdef CONFIG_RX_INDICATE_QUEUE
	TASK_INIT(&precvpriv->rx_indicate_tasklet, 0, rtw_rx_indicate_tasklet, padapter);
#endif /* CONFIG_RX_INDICATE_QUEUE */
#endif /* PLATFORM_FREEBSD */

#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
#ifdef PLATFORM_LINUX
	precvpriv->int_in_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (precvpriv->int_in_urb == NULL) {
		res = _FAIL;
		RTW_INFO("alloc_urb for interrupt in endpoint fail !!!!\n");
		goto exit;
	}
#endif /* PLATFORM_LINUX */
	precvpriv->int_in_buf = rtw_zmalloc(ini_in_buf_sz);
	if (precvpriv->int_in_buf == NULL) {
		res = _FAIL;
		RTW_INFO("alloc_mem for interrupt in endpoint fail !!!!\n");
		goto exit;
	}
#endif /* CONFIG_USB_INTERRUPT_IN_PIPE */

	/* init recv_buf */
	_rtw_init_queue(&precvpriv->free_recv_buf_queue);
	_rtw_init_queue(&precvpriv->recv_buf_pending_queue);
#ifndef CONFIG_USE_USB_BUFFER_ALLOC_RX
	/* this is used only when RX_IOBUF is sk_buff */
	skb_queue_head_init(&precvpriv->free_recv_skb_queue);
#endif

	RTW_INFO("NR_RECVBUFF: %d, recvbuf_nr: %d\n", NR_RECVBUFF, regsty->recvbuf_nr);
	RTW_INFO("MAX_RECVBUF_SZ: %d\n", MAX_RECVBUF_SZ);
	precvpriv->pallocated_recv_buf = rtw_zmalloc(regsty->recvbuf_nr * sizeof(struct recv_buf) + 4);
	if (precvpriv->pallocated_recv_buf == NULL) {
		res = _FAIL;
		goto exit;
	}

	precvpriv->precv_buf = (u8 *)N_BYTE_ALIGMENT((SIZE_PTR)(precvpriv->pallocated_recv_buf), 4);

	precvbuf = (struct recv_buf *)precvpriv->precv_buf;

	for (i = 0; i < regsty->recvbuf_nr ; i++) {
		_rtw_init_listhead(&precvbuf->list);

#ifdef PLATFORM_WINDOWS
		_rtw_spinlock_init(&precvbuf->recvbuf_lock);
#endif

		precvbuf->alloc_sz = MAX_RECVBUF_SZ;

		res = rtw_os_recvbuf_resource_alloc(padapter, precvbuf, precvbuf->alloc_sz);
		if (res == _FAIL)
			break;

		precvbuf->ref_cnt = 0;
		precvbuf->adapter = padapter;

		/* rtw_list_insert_tail(&precvbuf->list, &(precvpriv->free_recv_buf_queue.queue)); */

		precvbuf++;
	}

	precvpriv->free_recv_buf_queue_cnt = regsty->recvbuf_nr;

#if defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD)

	skb_queue_head_init(&precvpriv->rx_skb_queue);

#ifdef CONFIG_RX_INDICATE_QUEUE
	memset(&precvpriv->rx_indicate_queue, 0, sizeof(struct ifqueue));
	mtx_init(&precvpriv->rx_indicate_queue.ifq_mtx, "rx_indicate_queue", NULL, MTX_DEF);
#endif /* CONFIG_RX_INDICATE_QUEUE */

#ifdef CONFIG_PREALLOC_RECV_SKB
	{
		int i;
		SIZE_PTR tmpaddr = 0;
		SIZE_PTR alignment = 0;
		struct sk_buff *pskb = NULL;

		RTW_INFO("NR_PREALLOC_RECV_SKB: %d\n", NR_PREALLOC_RECV_SKB);
#ifdef CONFIG_FIX_NR_BULKIN_BUFFER
		RTW_INFO("Enable CONFIG_FIX_NR_BULKIN_BUFFER\n");
#endif

		for (i = 0; i < NR_PREALLOC_RECV_SKB; i++) {
#ifdef CONFIG_PREALLOC_RX_SKB_BUFFER
			pskb = rtw_alloc_skb_premem(MAX_RECVBUF_SZ);
#else
			pskb = rtw_skb_alloc(MAX_RECVBUF_SZ + RECVBUFF_ALIGN_SZ);
#endif /* CONFIG_PREALLOC_RX_SKB_BUFFER */

			if (pskb) {
#ifdef PLATFORM_FREEBSD
				pskb->dev = padapter->pifp;
#else
				pskb->dev = padapter->pnetdev;
#endif /* PLATFORM_FREEBSD */

#ifndef CONFIG_PREALLOC_RX_SKB_BUFFER
				tmpaddr = (SIZE_PTR)pskb->data;
				alignment = tmpaddr & (RECVBUFF_ALIGN_SZ - 1);
				skb_reserve(pskb, (RECVBUFF_ALIGN_SZ - alignment));
#endif
				skb_queue_tail(&precvpriv->free_recv_skb_queue, pskb);
			}
		}
	}
#endif /* CONFIG_PREALLOC_RECV_SKB */

#endif /* defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) */

exit:

	return res;
}

void usb_free_recv_priv(_adapter *padapter, u16 ini_in_buf_sz)
{
	int i;
	struct registry_priv *regsty = &padapter->registrypriv;
	struct recv_buf *precvbuf;
	struct recv_priv	*precvpriv = &adapter_to_dvobj(padapter)->recvpriv;

	precvbuf = (struct recv_buf *)precvpriv->precv_buf;

	for (i = 0; i < regsty->recvbuf_nr ; i++) {
		rtw_os_recvbuf_resource_free(padapter, precvbuf);
		precvbuf++;
	}

	if (precvpriv->pallocated_recv_buf)
		rtw_mfree(precvpriv->pallocated_recv_buf, regsty->recvbuf_nr * sizeof(struct recv_buf) + 4);

#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
#ifdef PLATFORM_LINUX
	if (precvpriv->int_in_urb)
		usb_free_urb(precvpriv->int_in_urb);
#endif
	if (precvpriv->int_in_buf)
		rtw_mfree(precvpriv->int_in_buf, ini_in_buf_sz);
#endif /* CONFIG_USB_INTERRUPT_IN_PIPE */

#ifdef PLATFORM_LINUX

	if (skb_queue_len(&precvpriv->rx_skb_queue))
		RTW_WARN("rx_skb_queue not empty\n");

	rtw_skb_queue_purge(&precvpriv->rx_skb_queue);

	if (skb_queue_len(&precvpriv->free_recv_skb_queue))
		RTW_WARN("free_recv_skb_queue not empty, %d\n", skb_queue_len(&precvpriv->free_recv_skb_queue));

#if !defined(CONFIG_USE_USB_BUFFER_ALLOC_RX)
#if defined(CONFIG_PREALLOC_RECV_SKB) && defined(CONFIG_PREALLOC_RX_SKB_BUFFER)
	{
		struct sk_buff *skb;

		while ((skb = skb_dequeue(&precvpriv->free_recv_skb_queue)) != NULL) {
			if (rtw_free_skb_premem(skb) != 0)
				rtw_skb_free(skb);
		}
	}
#else
	rtw_skb_queue_purge(&precvpriv->free_recv_skb_queue);
#endif /* defined(CONFIG_PREALLOC_RX_SKB_BUFFER) && defined(CONFIG_PREALLOC_RECV_SKB) */
#endif /* !defined(CONFIG_USE_USB_BUFFER_ALLOC_RX) */

#endif /* PLATFORM_LINUX */

#ifdef PLATFORM_FREEBSD
	struct sk_buff  *pskb;
	while (NULL != (pskb = skb_dequeue(&precvpriv->rx_skb_queue)))
		rtw_skb_free(pskb);

#if !defined(CONFIG_USE_USB_BUFFER_ALLOC_RX)
	rtw_skb_queue_purge(&precvpriv->free_recv_skb_queue);
#endif

#ifdef CONFIG_RX_INDICATE_QUEUE
	struct mbuf *m;
	for (;;) {
		IF_DEQUEUE(&precvpriv->rx_indicate_queue, m);
		if (m == NULL)
			break;
		rtw_skb_free(m);
	}
	mtx_destroy(&precvpriv->rx_indicate_queue.ifq_mtx);
#endif /* CONFIG_RX_INDICATE_QUEUE */

#endif /* PLATFORM_FREEBSD */
}

#ifdef CONFIG_FW_C2H_REG
void usb_c2h_hisr_hdl(_adapter *adapter, u8 *buf)
{
	u8 *c2h_evt = buf;
	u8 id, seq, plen;
	u8 *payload;

	if (rtw_hal_c2h_reg_hdr_parse(adapter, buf, &id, &seq, &plen, &payload) != _SUCCESS)
		return;

	if (0)
		RTW_PRINT("%s C2H == %d\n", __func__, id);

	if (rtw_hal_c2h_id_handle_directly(adapter, id, seq, plen, payload)) {
		/* Handle directly */
		rtw_hal_c2h_handler(adapter, id, seq, plen, payload);

		/* Replace with special pointer to trigger c2h_evt_clear only */
		if (rtw_cbuf_push(adapter->evtpriv.c2h_queue, (void*)&adapter->evtpriv) != _SUCCESS)
			RTW_ERR("%s rtw_cbuf_push fail\n", __func__);
	} else {
		c2h_evt = rtw_malloc(C2H_REG_LEN);
		if (c2h_evt != NULL) {
			_rtw_memcpy(c2h_evt, buf, C2H_REG_LEN);
			if (rtw_cbuf_push(adapter->evtpriv.c2h_queue, (void*)c2h_evt) != _SUCCESS)
				RTW_ERR("%s rtw_cbuf_push fail\n", __func__);
		} else {
			/* Error handling for malloc fail */
			if (rtw_cbuf_push(adapter->evtpriv.c2h_queue, (void*)NULL) != _SUCCESS)
				RTW_ERR("%s rtw_cbuf_push fail\n", __func__);
		}
	}
	_set_workitem(&adapter->evtpriv.c2h_wk);
}
#endif

void usb_set_intf_ops(_adapter *padapter, struct _io_ops *pops)
{
	_rtw_memset((u8 *)pops, 0, sizeof(struct _io_ops));

	pops->_write_port_cancel = &rtw_usb_write_port_cancel;

#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
	pops->_read_interrupt = &usb_read_interrupt;
#endif

}
