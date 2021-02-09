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
#define _USB_OPS_LINUX_C_

#include <drv_types.h>
#include <hal_data.h>
#include <rtw_sreset.h>


int usbctrl_vendorreq(struct dvobj_priv *dvobj, u8 request, u16 value, u16 index, void *pdata, u16 len, u8 requesttype)
{
	struct usb_device *udev = dvobj_to_usb(dvobj)->pusbdev;

	unsigned int pipe;
	int status = 0;
#ifdef CONFIG_USB_VENDOR_REQ_BUFFER_DYNAMIC_ALLOCATE
	u32 tmp_buflen = 0;
#endif
	u8 reqtype;
	u8 *pIo_buf;
	int vendorreq_times = 0;


#if (defined(CONFIG_RTL8822B) || defined(CONFIG_RTL8821C)) || defined(CONFIG_RTL8822C)
#define REG_ON_SEC 0x00
#define REG_OFF_SEC 0x01
#define REG_LOCAL_SEC 0x02
	u8 current_reg_sec = REG_LOCAL_SEC;
#endif

#ifdef CONFIG_USB_VENDOR_REQ_BUFFER_DYNAMIC_ALLOCATE
	u8 *tmp_buf;
#else /* use stack memory */
	#ifndef CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC
	u8 tmp_buf[MAX_USB_IO_CTL_SIZE];
	#endif
#endif

	/* RTW_INFO("%s %s:%d\n",__FUNCTION__, current->comm, current->pid); */
	if (RTW_CANNOT_IO(dvobj)) {
		status = -EPERM;
		goto exit;
	}

	if (len > MAX_VENDOR_REQ_CMD_SIZE) {
		RTW_INFO("[%s] Buffer len error ,vendor request failed\n", __FUNCTION__);
		status = -EINVAL;
		goto exit;
	}

#ifdef CONFIG_USB_VENDOR_REQ_MUTEX
	_rtw_mutex_lock(&dvobj_to_usb(dvobj)->usb_vendor_req_mutex);
#endif


	/* Acquire IO memory for vendorreq */
#ifdef CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC
	pIo_buf = dvobj_to_usb(dvobj)->usb_vendor_req_buf;
#else
	#ifdef CONFIG_USB_VENDOR_REQ_BUFFER_DYNAMIC_ALLOCATE
	tmp_buf = rtw_malloc((u32) len + ALIGNMENT_UNIT);
	tmp_buflen = (u32)len + ALIGNMENT_UNIT;
	#else /* use stack memory */
	tmp_buflen = MAX_USB_IO_CTL_SIZE;
	#endif

	/* Added by Albert 2010/02/09 */
	/* For mstar platform, mstar suggests the address for USB IO should be 16 bytes alignment. */
	/* Trying to fix it here. */
	pIo_buf = (tmp_buf == NULL) ? NULL : tmp_buf + ALIGNMENT_UNIT - ((SIZE_PTR)(tmp_buf) & 0x0f);
#endif

	if (pIo_buf == NULL) {
		RTW_INFO("[%s] pIo_buf == NULL\n", __FUNCTION__);
		status = -ENOMEM;
		goto release_mutex;
	}

	while (++vendorreq_times <= MAX_USBCTRL_VENDORREQ_TIMES) {
		_rtw_memset(pIo_buf, 0, len);

		if (requesttype == 0x01) {
			pipe = usb_rcvctrlpipe(udev, 0);/* read_in */
			reqtype =  REALTEK_USB_VENQT_READ;
		} else {
			pipe = usb_sndctrlpipe(udev, 0);/* write_out */
			reqtype =  REALTEK_USB_VENQT_WRITE;
			_rtw_memcpy(pIo_buf, pdata, len);
		}

		status = rtw_usb_control_msg(udev, pipe, request, reqtype, value, index, pIo_buf, len, RTW_USB_CONTROL_MSG_TIMEOUT);

		if (status == len) {  /* Success this control transfer. */
			rtw_reset_continual_io_error(dvobj);
			if (requesttype == 0x01) {
				/* For Control read transfer, we have to copy the read data from pIo_buf to pdata. */
				_rtw_memcpy(pdata, pIo_buf,  len);
			}
		} else { /* error cases */
			switch (len) {
				case 1:
					RTW_INFO("reg 0x%x, usb %s %u fail, status:%d value=0x%x, vendorreq_times:%d\n"
						, (index << 16 | value), (requesttype == 0x01) ? "read" : "write" , len, status, *(u8 *)pdata, vendorreq_times);
				break;
				case 2:
					RTW_INFO("reg 0x%x, usb %s %u fail, status:%d value=0x%x, vendorreq_times:%d\n"
						, (index << 16 | value), (requesttype == 0x01) ? "read" : "write" , len, status, *(u16 *)pdata, vendorreq_times);
				break;
				case 4:
					RTW_INFO("reg 0x%x, usb %s %u fail, status:%d value=0x%x, vendorreq_times:%d\n"
						, (index << 16 | value), (requesttype == 0x01) ? "read" : "write" , len, status, *(u32 *)pdata, vendorreq_times);
				break;
				default:
					RTW_INFO("reg 0x%x, usb %s %u fail, status:%d, vendorreq_times:%d\n"
						, (index << 16 | value), (requesttype == 0x01) ? "read" : "write" , len, status, vendorreq_times);
				break;
			}

			if (status < 0) {
				if (status == (-ESHUTDOWN)	|| status == -ENODEV)
					dev_set_surprise_removed(dvobj);
				else {

				}
			} else { /* status != len && status >= 0 */
				if (status > 0) {
					if (requesttype == 0x01) {
						/* For Control read transfer, we have to copy the read data from pIo_buf to pdata. */
						_rtw_memcpy(pdata, pIo_buf,  len);
					}
				}
			}

			if (rtw_inc_and_chk_continual_io_error(dvobj) == _TRUE) {
				dev_set_surprise_removed(dvobj);
				break;
			}

		}

		if (status == len)
			break;

	}

#if (defined(CONFIG_RTL8822B) || defined(CONFIG_RTL8821C)) || defined(CONFIG_RTL8822C)
	if (value < 0xFE00) {
		if (value <= 0xff)
			current_reg_sec = REG_ON_SEC;
		else if (0x1000 <= value && value <= 0x10ff)
			current_reg_sec = REG_ON_SEC;
		else
			current_reg_sec = REG_OFF_SEC;
	} else {
		current_reg_sec = REG_LOCAL_SEC;
	}

	if (current_reg_sec == REG_ON_SEC) {
		unsigned int t_pipe = usb_sndctrlpipe(udev, 0);/* write_out */
		u8 t_reqtype =  REALTEK_USB_VENQT_WRITE;
		u8 t_len = 1;
		u8 t_req = 0x05;
		u16 t_reg = 0;
		u16 t_index = 0;

		t_reg = 0x4e0;

		status = rtw_usb_control_msg(udev, t_pipe, t_req, t_reqtype, t_reg, t_index, pIo_buf, t_len, RTW_USB_CONTROL_MSG_TIMEOUT);

		if (status == t_len)
			rtw_reset_continual_io_error(dvobj);
		else
			RTW_INFO("reg 0x%x, usb %s %u fail, status:%d\n", t_reg, "write" , t_len, status);

	}
#endif

	/* release IO memory used by vendorreq */
#ifdef CONFIG_USB_VENDOR_REQ_BUFFER_DYNAMIC_ALLOCATE
	rtw_mfree(tmp_buf, tmp_buflen);
#endif

release_mutex:
#ifdef CONFIG_USB_VENDOR_REQ_MUTEX
	_rtw_mutex_unlock(&dvobj_to_usb(dvobj)->usb_vendor_req_mutex);
#endif
exit:
	return status;

}

#if (KERNEL_VERSION(2, 5, 0) > LINUX_VERSION_CODE) ||\
	(KERNEL_VERSION(2, 6, 18) < LINUX_VERSION_CODE)
/*#define _usbctrl_vendorreq_async_callback(urb, regs)\*/
	/*_usbctrl_vendorreq_async_callback(urb)*/
/*#define usb_bulkout_zero_complete(purb, regs)\*/
	/*usb_bulkout_zero_complete(purb)*/
#define rtw_usb_g6_write_port_complete(purb, regs)\
	rtw_usb_g6_write_port_complete(purb)
#define rtw_usb_write_port_complete(purb, regs)\
	rtw_usb_write_port_complete(purb)
#define rtw_usb_read_port_complete(purb, regs)\
	rtw_usb_read_port_complete(purb)
#define rtw_usb_read_interrupt_complete(purb, regs)\
	rtw_usb_read_interrupt_complete(purb)
#endif

unsigned int bulkid2pipe(struct dvobj_priv *pdvobj, u32 addr, u8 bulk_out)
{
	unsigned int pipe = 0, ep_num = 0;
	PUSB_DATA pusb_data = dvobj_to_usb(pdvobj);
	struct usb_device *pusbd = pusb_data->pusbdev;

	if (!bulk_out) {
		if (addr == RECV_BULK_IN_ADDR)
			pipe = usb_rcvbulkpipe(pusbd, pusb_data->RtInPipe[0]);
		else
			pipe = usb_rcvintpipe(pusbd, pusb_data->RtInPipe[1]);
	} else {
                ep_num = pusb_data->RtOutPipe[addr];
                pipe = usb_sndbulkpipe(pusbd, ep_num);
	}

	return pipe;
}

int rtw_os_urb_resource_alloc(struct data_urb *dataurb)
{
	dataurb->urb= usb_alloc_urb(0, GFP_KERNEL);

	if (dataurb->urb == NULL) {
		rtw_msleep_os(10);
		dataurb->urb = usb_alloc_urb(0, GFP_KERNEL);
		if (dataurb->urb == NULL) {
			RTW_ERR("(dataurb->urb == NULL");
			rtw_warn_on(1);
			return _FAIL;
		}
	}
	return _SUCCESS;
}

void rtw_os_urb_resource_free(struct data_urb *dataurb)
{
	if (dataurb->urb) {
		usb_free_urb(dataurb->urb);
		dataurb->urb = NULL;
	}
}

struct data_urb *rtw_alloc_dataurb(struct trx_urb_buf_q *urb_q)
{
	struct data_urb *urb =  NULL;
	_list *urb_list, *urb_head;
	_queue *free_urb_q = &urb_q->free_urb_buf_queue;
	unsigned long sp_flags;

	/* RTW_INFO("+rtw_alloc_litexmitbuf\n"); */

	_rtw_spinlock_irq(&free_urb_q->lock, &sp_flags);

	if (_rtw_queue_empty(free_urb_q) == _TRUE)
		urb = NULL;
	else {

		urb_head = get_list_head(free_urb_q);

		urb_list = get_next(urb_head);

		urb = LIST_CONTAINOR(urb_list,
			struct data_urb, list);

		rtw_list_delete(&(urb->list));
	}

	if (urb !=  NULL)
		urb_q->free_urb_buf_cnt--;


	_rtw_spinunlock_irq(&free_urb_q->lock, &sp_flags);


	return urb;
}

s32 rtw_free_dataurb(struct trx_urb_buf_q *urb_q,
	struct data_urb *urb)
{
	_queue *free_urb_q = &urb_q->free_urb_buf_queue;
	unsigned long sp_flags;

	/* RTW_INFO("+rtw_free_xmiturb\n"); */

	if (urb_q == NULL)
		return _FAIL;

	if (urb == NULL)
		return _FAIL;

	_rtw_spinlock_irq(&free_urb_q->lock, &sp_flags);

	rtw_list_delete(&urb->list);

	rtw_list_insert_tail(&(urb->list),
		get_list_head(free_urb_q));

	urb_q->free_urb_buf_cnt++;

	_rtw_spinunlock_irq(&free_urb_q->lock, &sp_flags);

	return _SUCCESS;
}

static void rtw_usb_write_port_complete(struct urb *purb, struct pt_regs *regs)
{
	struct lite_data_buf *litexmitbuf =
		(struct lite_data_buf *)purb->context;
	struct xmit_buf *pxmitbuf = litexmitbuf->pxmitbuf;
	struct data_urb *xmiturb = litexmitbuf->dataurb;
	struct dvobj_priv *pdvobj = litexmitbuf->dvobj;
	_adapter	*padapter = pxmitbuf->padapter;
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;

	switch (pxmitbuf->flags) {
	case VO_QUEUE_INX:
		pxmitpriv->voq_cnt--;
		break;
	case VI_QUEUE_INX:
		pxmitpriv->viq_cnt--;
		break;
	case BE_QUEUE_INX:
		pxmitpriv->beq_cnt--;
		break;
	case BK_QUEUE_INX:
		pxmitpriv->bkq_cnt--;
		break;
	default:
		break;
	}


	if (RTW_CANNOT_TX(pdvobj)) {
		RTW_INFO(
			"%s(): TX Warning! bDriverStopped(%s) OR bSurpriseRemoved(%s) pxmitbuf->buf_tag(%x)\n"
			 , __func__
			 , dev_is_drv_stopped(pdvobj) ? "True" : "False"
			 , dev_is_surprise_removed(pdvobj) ? "True" : "False"
			 , pxmitbuf->buf_tag);

		goto check_completion;
	}


	if (purb->status == 0) {

	} else {
		RTW_INFO("###=> urb_write_port_complete status(%d)\n",
			 purb->status);
		if ((purb->status == -EPIPE) || (purb->status == -EPROTO)) {
			/* usb_clear_halt(pusbdev, purb->pipe);	 */
			/* msleep(10); */
			/*sreset_set_wifi_error_status(padapter, USB_WRITE_PORT_FAIL);*/
		} else if (purb->status == -EINPROGRESS) {
			goto check_completion;

		} else if (purb->status == -ENOENT) {
			RTW_INFO("%s: -ENOENT\n", __func__);
			goto check_completion;

		} else if (purb->status == -ECONNRESET) {
			RTW_INFO("%s: -ECONNRESET\n", __func__);
			goto check_completion;

		} else if (purb->status == -ESHUTDOWN) {
			dev_set_drv_stopped(pdvobj);

			goto check_completion;
		} else {
			dev_set_surprise_removed(pdvobj);
			RTW_INFO("bSurpriseRemoved=TRUE\n");

			goto check_completion;
		}
	}

check_completion:
	rtw_sctx_done_err(&pxmitbuf->sctx,
		purb->status ? RTW_SCTX_DONE_WRITE_PORT_ERR : RTW_SCTX_DONE_SUCCESS);

	rtw_free_litedatabuf(&pdvobj->litexmitbuf_q, litexmitbuf);
	rtw_free_dataurb(&pdvobj->xmit_urb_q, xmiturb);
	rtw_free_xmitbuf(pxmitpriv, pxmitbuf);

	{
		tasklet_hi_schedule(&pxmitpriv->xmit_tasklet);
	}
}

u32 rtw_usb_write_port(struct intf_hdl *pintfhdl, u32 addr, u32 len, u8 *wmem)
{
	_irqL irqL;
	unsigned int pipe;
	int status;
	u32 ret = _FAIL;
	PURB	purb = NULL;
	_adapter *padapter = (_adapter *)pintfhdl->padapter;
	struct dvobj_priv	*pdvobj = adapter_to_dvobj(padapter);
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;
	struct xmit_buf *pxmitbuf = (struct xmit_buf *)wmem;
	struct xmit_frame *pxmitframe = (struct xmit_frame *)pxmitbuf->priv_data;
	PUSB_DATA pusb_data = dvobj_to_usb(pdvobj);
	struct usb_device *pusbd = pusb_data->pusbdev;

	struct lite_data_buf *litexmitbuf = NULL;
	struct data_urb *xmiturb = NULL;


	//RTW_INFO("%s : NEO bulkout_id:%d\n", __func__, pxmitbuf->bulkout_id);

	litexmitbuf = rtw_alloc_litedatabuf(&pdvobj->litexmitbuf_q);
	if (litexmitbuf == NULL) {
		RTW_INFO("%s,%d Can't alloc lite xmit buf\n",
			__func__, __LINE__);
		goto exit;
	}

	xmiturb = rtw_alloc_dataurb(&pdvobj->xmit_urb_q);
	if (xmiturb == NULL) {
		RTW_INFO("%s,%d Can't alloc lite xmit urb\n",
			__func__, __LINE__);
		goto exit;
	}

	if (RTW_CANNOT_TX(pdvobj)) {
#ifdef DBG_TX
		RTW_INFO(" DBG_TX %s:%d bDriverStopped%s, bSurpriseRemoved:%s\n"
			 , __func__, __LINE__,
			 dev_is_drv_stopped(pdvobj) ? "True" : "False"
			 dev_is_surprise_removed(pdvobj) ? "True" : "False");
#endif
		rtw_sctx_done_err(&pxmitbuf->sctx, RTW_SCTX_DONE_TX_DENY);
		goto exit;
	}

	_enter_critical(&pxmitpriv->lock, &irqL);

	switch (addr) {
	case VO_QUEUE_INX:
		pxmitpriv->voq_cnt++;
		pxmitbuf->flags = VO_QUEUE_INX;
		break;
	case VI_QUEUE_INX:
		pxmitpriv->viq_cnt++;
		pxmitbuf->flags = VI_QUEUE_INX;
		break;
	case BE_QUEUE_INX:
		pxmitpriv->beq_cnt++;
		pxmitbuf->flags = BE_QUEUE_INX;
		break;
	case BK_QUEUE_INX:
		pxmitpriv->bkq_cnt++;
		pxmitbuf->flags = BK_QUEUE_INX;
		break;
	case HIGH_QUEUE_INX:
		pxmitbuf->flags = HIGH_QUEUE_INX;
		break;
	default:
		pxmitbuf->flags = MGT_QUEUE_INX;
		break;
	}

	_exit_critical(&pxmitpriv->lock, &irqL);

	purb = xmiturb->urb;

	/* translate DMA FIFO addr to pipehandle */
	pipe = bulkid2pipe(pdvobj, pxmitbuf->bulkout_id, _TRUE);

#ifdef CONFIG_REDUCE_USB_TX_INT
	if ((pxmitpriv->free_xmitbuf_cnt % NR_XMITBUFF == 0)
	    || (pxmitbuf->buf_tag > XMITBUF_DATA))
		purb->transfer_flags  &= (~URB_NO_INTERRUPT);
	else {
		purb->transfer_flags  |=  URB_NO_INTERRUPT;
		/* RTW_INFO("URB_NO_INTERRUPT "); */
	}
#endif

	litexmitbuf->dvobj = pdvobj;
	litexmitbuf->pbuf = pxmitframe->buf_addr;
	litexmitbuf->dataurb = xmiturb;
	litexmitbuf->pxmitbuf = pxmitbuf;

	usb_fill_bulk_urb(purb, pusbd, pipe,
			  litexmitbuf->pbuf,
			  len,
			  rtw_usb_write_port_complete,
			  litexmitbuf);

#ifdef CONFIG_USE_USB_BUFFER_ALLOC_TX
	purb->transfer_dma = pxmitbuf->dma_transfer_addr;
	purb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
	purb->transfer_flags |= URB_ZERO_PACKET;
#endif /* CONFIG_USE_USB_BUFFER_ALLOC_TX */

#ifdef USB_PACKET_OFFSET_SZ
#if (USB_PACKET_OFFSET_SZ == 0)
	purb->transfer_flags |= URB_ZERO_PACKET;
#endif
#endif

#if 0
	if (bwritezero)
		purb->transfer_flags |= URB_ZERO_PACKET;
#endif

	status = usb_submit_urb(purb, GFP_ATOMIC);
	if (!status) {
	} else {
		rtw_sctx_done_err(&pxmitbuf->sctx, RTW_SCTX_DONE_WRITE_PORT_ERR);
		RTW_INFO("%s, status=%d\n", __func__, status);

		switch (status) {
		case -ENODEV:
			dev_set_drv_stopped(pdvobj);
			break;
		default:
			break;
		}
		goto exit;
	}

	ret = _SUCCESS;

	/* Commented by Albert 2009/10/13
	 * We add the URB_ZERO_PACKET flag to urb so that the host will send the zero packet automatically. */
	/*
		if(bwritezero == _TRUE)
		{
			usb_bulkout_zero(pintfhdl, addr);
		}
	*/


exit:
	if (ret != _SUCCESS) {
		rtw_free_litedatabuf(&pdvobj->litexmitbuf_q, litexmitbuf);
		rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
	}
	return ret;

}


static void rtw_usb_g6_write_port_complete(struct urb *purb, struct pt_regs *regs)
{

	struct lite_data_buf *litexmitbuf =
		(struct lite_data_buf *)purb->context;
	struct data_urb *xmiturb =  litexmitbuf->dataurb;
	struct dvobj_priv *pdvobj = litexmitbuf->dvobj;
	unsigned long sp_flags;

	if (RTW_CANNOT_TX(pdvobj)) {
		RTW_INFO(
			"%s(): TX Warning! bDriverStopped(%s) OR bSurpriseRemoved(%s)\n"
			 , __func__
			 , dev_is_drv_stopped(pdvobj) ? "True" : "False"
			 , dev_is_surprise_removed(pdvobj) ? "True" : "False");

		goto check_completion;
	}


	if (purb->status == 0) {

	} else {
		RTW_INFO("###=> urb_write_port_complete status(%d)\n",
			purb->status);
		if ((purb->status == -EPIPE) || (purb->status == -EPROTO)) {
			/* usb_clear_halt(pusbdev, purb->pipe);	 */
			/* msleep(10); */
			/*sreset_set_wifi_error_status(padapter,*/
				/*USB_WRITE_PORT_FAIL);*/
		} else if (purb->status == -EINPROGRESS) {
			goto check_completion;

		} else if (purb->status == -ENOENT) {
			RTW_INFO("%s: -ENOENT\n", __func__);
			goto check_completion;

		} else if (purb->status == -ECONNRESET) {
			RTW_INFO("%s: -ECONNRESET\n", __func__);
			goto check_completion;

		} else if (purb->status == -ESHUTDOWN) {
			dev_set_drv_stopped(pdvobj);

			goto check_completion;
		} else {
			dev_set_surprise_removed(pdvobj);
			RTW_INFO("bSurpriseRemoved=TRUE\n");

			goto check_completion;
		}
	}


check_completion:


	rtw_sctx_done_err(&litexmitbuf->sctx,
	purb->status ? RTW_SCTX_DONE_WRITE_PORT_ERR : RTW_SCTX_DONE_SUCCESS);

	rtw_phl_recycle_tx_buf(pdvobj->phl, litexmitbuf->phl_buf_ptr);
	rtw_free_litedatabuf(&pdvobj->litexmitbuf_q, litexmitbuf);
	rtw_free_dataurb(&pdvobj->xmit_urb_q, xmiturb);
	rtw_phl_tx_req_notify(pdvobj->phl);

}


u32 rtw_usb_g6_write_port(void *d, u8 *phl_tx_buf_ptr,
	u8  bulk_id, u32 len, u8 *pkt_data_buf)
{

	int status;
	unsigned int pipe;
	u32 ret = _FAIL;
	struct dvobj_priv *pdvobj = (struct dvobj_priv *)d;
	struct usb_device *pusbd = dvobj_to_usb(pdvobj)->pusbdev;
	struct lite_data_buf *litexmitbuf = NULL;
	struct data_urb *xmiturb = NULL;


	litexmitbuf = rtw_alloc_litedatabuf(&pdvobj->litexmitbuf_q);
	if (litexmitbuf == NULL) {
		RTW_INFO("%s,%d Can't alloc lite xmit buf\n",
			__func__, __LINE__);
		goto exit;
	}
	xmiturb = rtw_alloc_dataurb(&pdvobj->xmit_urb_q);
	if (xmiturb == NULL) {
		RTW_INFO("%s,%d Can't alloc lite xmit urb\n",
			__func__, __LINE__);
		goto exit;
	}

	if (RTW_CANNOT_TX(pdvobj)) {
#ifdef DBG_TX
		RTW_INFO(" DBG_TX %s:%d bDriverStopped:%s, bSurpriseRemoved:%s\n"
			,__func__, __LINE__,
			dev_is_drv_stopped(pdvobj) ? "True" : "False",
			dev_is_surprise_removed(pdvobj) ? "True" : "False");
#endif
		rtw_sctx_done_err(&litexmitbuf->sctx, RTW_SCTX_DONE_TX_DENY);
		goto exit;
	}

	litexmitbuf->dvobj = pdvobj;
	litexmitbuf->pbuf = pkt_data_buf;
	litexmitbuf->dataurb = xmiturb;
	litexmitbuf->phl_buf_ptr = phl_tx_buf_ptr;

	pipe = bulkid2pipe(pdvobj, bulk_id, _TRUE);

	usb_fill_bulk_urb(xmiturb->urb, pusbd, pipe,
			  litexmitbuf->pbuf,
			  len,
			  rtw_usb_g6_write_port_complete,
			  litexmitbuf);
	xmiturb->urb->transfer_flags |= URB_ZERO_PACKET;
	status = usb_submit_urb(xmiturb->urb, GFP_ATOMIC);
	if (!status) {

	} else {
		rtw_sctx_done_err(&litexmitbuf->sctx,
			RTW_SCTX_DONE_WRITE_PORT_ERR);
		RTW_INFO("%s, status=%d\n", __func__, status);

		switch (status) {
		case -ENODEV:
			dev_set_drv_stopped(pdvobj);
			break;
		default:
			break;
		}
		goto exit;
	}

	ret = _SUCCESS;

exit:
	if (ret != _SUCCESS) {
		rtw_free_litedatabuf(&pdvobj->litexmitbuf_q, litexmitbuf);
		rtw_free_dataurb(&pdvobj->xmit_urb_q, xmiturb);
	}

	if (ret == _SUCCESS)
		ret = RTW_PHL_STATUS_SUCCESS;
	else
		ret = RTW_PHL_STATUS_FAILURE;

	return ret;

}

void rtw_usb_write_port_cancel(struct intf_hdl *pintfhdl)
{
	int i, j;
	_adapter	*padapter = pintfhdl->padapter;
	struct xmit_buf *pxmitbuf = (struct xmit_buf *)padapter->xmitpriv.pxmitbuf;

	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct data_urb *xmiturb = (struct data_urb *)dvobj->xmit_urb_q.urb_buf;
	u32 xmiturb_nr = RTW_XMITURB_NR;

	if (dvobj == NULL) {
		RTW_ERR("%s dvobj is NULL\n", __func__);
		rtw_warn_on(1);
		return;
	}

	RTW_INFO("%s\n", __func__);

	for (i = 0; i < xmiturb_nr; i++) {
		usb_kill_urb(xmiturb->urb);
		xmiturb++;
	}
}

static void rtw_usb_read_port_complete(struct urb *urb, struct pt_regs *regs)
{
	struct lite_data_buf *literecvbuf =
		(struct lite_data_buf *)urb->context;
	struct data_urb *recvurb =  literecvbuf->dataurb;
	struct dvobj_priv *dvobj = literecvbuf->dvobj;
	struct trx_data_buf_q *rx_data_buf_q = NULL;
	struct trx_urb_buf_q *rx_urb_q = NULL;
	u32 actual_length = urb->actual_length;
	u32 transfer_buffer_length = urb->transfer_buffer_length;
	u8 bulk_id = recvurb->bulk_id;
	u8 minlen = recvurb->minlen;
	unsigned long sp_flags;
	u8 status = _SUCCESS;


	if (bulk_id == REALTEK_USB_BULK_IN_EP_IDX) {
		rx_data_buf_q = &dvobj->literecvbuf_q;
		rx_urb_q = &dvobj->recv_urb_q;
		ATOMIC_DEC(&(dvobj->rx_pending_cnt));
	} else {
		#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
		rx_data_buf_q = &dvobj->intin_buf_q;
		rx_urb_q = &dvobj->intin_urb_q;
		#endif
	}

	if (RTW_CANNOT_RX(dvobj)) {
		RTW_INFO("%s() RX Warning! bDriverStopped(%s) OR bSurpriseRemoved(%s)\n"
			, __func__
			, dev_is_drv_stopped(dvobj) ? "True" : "False"
			, dev_is_surprise_removed(dvobj) ? "True" : "False");

		status = _FAIL;
		goto exit;
	}

	if (urb->status == 0) {
		if ((actual_length > transfer_buffer_length) || (actual_length < minlen)) {
			RTW_INFO("%s()-%d: actual_length:%u, transfer_buffer_length:%u, minlen:%u\n"
				, __FUNCTION__, __LINE__, actual_length, transfer_buffer_length, minlen);

			status = _FAIL;
			goto exit;
		} else {
			rtw_reset_continual_io_error(dvobj);
			status = _SUCCESS;
			goto exit;
		}
	} if (urb->status == -ENOENT) {
		/*use usb_kill_urb urb status code = -ENOENT*/
		status = _FAIL;
		goto exit;
	} else {

		RTW_INFO("###=> %s => urb.status(%d)\n", __func__, urb->status);
		status = _FAIL;

		if (rtw_inc_and_chk_continual_io_error(dvobj) == _TRUE)
			dev_set_surprise_removed(dvobj);

		switch (urb->status) {
		case -EINVAL:
		case -EPIPE:
		case -ENODEV:
		case -ESHUTDOWN:
			dev_set_drv_stopped(dvobj);
			break;
		case -EPROTO:
		case -EILSEQ:
		case -ETIME:
		case -ECOMM:
		case -EOVERFLOW:
			break;
		case -EINPROGRESS:
			RTW_INFO("ERROR: URB IS IN PROGRESS!/n");
			break;
		default:
			break;
		}
		goto exit;
	}

exit:

	if (status == _SUCCESS)
		status = RTW_PHL_STATUS_SUCCESS;
	else
		status = RTW_PHL_STATUS_FAILURE;

	//print_hex_dump(KERN_INFO, "USB read: ", DUMP_PREFIX_OFFSET, 16, 1, literecvbuf->pbuf, actual_length, 1);
	rtw_phl_post_in_complete(dvobj->phl, literecvbuf->phl_buf_ptr, actual_length, status);
	rtw_free_litedatabuf(rx_data_buf_q, literecvbuf);
	rtw_free_dataurb(rx_urb_q, recvurb);
}

u32 rtw_usb_read_port(void *d, void *rxobj,
	u8 *inbuf, u32 inbuf_len, u8 bulk_id, u8 minlen)
{
	int err;
	unsigned int pipe;
	u32 ret = _FAIL;
	struct dvobj_priv *dvobj = (struct dvobj_priv *)d;
	struct usb_device *usbd = dvobj_to_usb(dvobj)->pusbdev;
	struct lite_data_buf *literecvbuf = NULL;
	struct data_urb *recvurb = NULL;
	struct trx_data_buf_q *rx_data_buf_q = NULL;
	struct trx_urb_buf_q *rx_urb_q = NULL;
	struct usb_data *usb_data = dvobj_to_usb(dvobj);

	if (RTW_CANNOT_RX(dvobj) || (inbuf == NULL)) {
		goto exit;
	}

	if (bulk_id == REALTEK_USB_BULK_IN_EP_IDX) {
		rx_data_buf_q = &dvobj->literecvbuf_q;
		rx_urb_q = &dvobj->recv_urb_q;
	} else if (bulk_id == REALTEK_USB_IN_INT_EP_IDX) {
		#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
		rx_data_buf_q = &dvobj->intin_buf_q;
		rx_urb_q = &dvobj->intin_urb_q;
		#else
		goto exit;
		#endif
	} else {
		RTW_INFO("%s,%d Unkown bulk id:%d\n",
			__func__, __LINE__, bulk_id);
		ret = _FAIL;
		goto exit;
	}

	literecvbuf = rtw_alloc_litedatabuf(rx_data_buf_q);
	if (literecvbuf == NULL) {
		RTW_INFO("%s,%d Can't alloc lite recv buf\n",
			__func__, __LINE__);
		goto exit;
	}
	recvurb = rtw_alloc_dataurb(rx_urb_q);
	if (recvurb == NULL) {
		RTW_INFO("%s,%d Can't alloc lite recv urb\n",
			__func__, __LINE__);
		goto exit;
	}

	recvurb->bulk_id = bulk_id;
	recvurb->minlen = minlen;
	literecvbuf->dvobj = dvobj;
	literecvbuf->pbuf = inbuf;
	literecvbuf->dataurb = recvurb;
	literecvbuf->phl_buf_ptr = rxobj;

	pipe = bulkid2pipe(dvobj, bulk_id | 0x80, _FALSE);

	if (bulk_id == REALTEK_USB_BULK_IN_EP_IDX) {
		usb_fill_bulk_urb(recvurb->urb, usbd, pipe,
			literecvbuf->pbuf,
			inbuf_len,
			rtw_usb_read_port_complete,
			literecvbuf);
	} else {
		#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
		if (usb_data->inpipe_type[bulk_id] == REALTEK_USB_BULK_IN_EP_IDX)
			usb_fill_bulk_urb(recvurb->urb, usbd, pipe,
				literecvbuf->pbuf,
				inbuf_len,
				rtw_usb_read_port_complete,
				literecvbuf);
		else
			usb_fill_int_urb(recvurb->urb, usbd, pipe,
				literecvbuf->pbuf,
				inbuf_len,
				rtw_usb_read_port_complete,
				literecvbuf,
				1);
		#endif
	}

	//RTW_INFO("%s : NEO: rxbuf: %p, size: %d\n", __func__, literecvbuf->pbuf, inbuf_len);

	err = usb_submit_urb(recvurb->urb, GFP_ATOMIC);
	if ((err) && (err != (-EPERM))) {
		RTW_INFO("cannot submit rx in-token(err = 0x%08x),urb_status = %d\n", err, recvurb->urb->status);
		ret = _FAIL;
		goto exit;
	}

	/* record usb bulk in */
	if (bulk_id == REALTEK_USB_BULK_IN_EP_IDX)
		ATOMIC_INC(&(dvobj->rx_pending_cnt));

	ret = _SUCCESS;
exit:
	if (ret != _SUCCESS) {
		rtw_free_litedatabuf(rx_data_buf_q, literecvbuf);
		rtw_free_dataurb(rx_urb_q, recvurb);
	}

	if (ret == _SUCCESS)
		ret = RTW_PHL_STATUS_SUCCESS;
	else
		ret = RTW_PHL_STATUS_FAILURE;

	return ret;
}

void rtw_usb_read_port_cancel(void *d)
{
	int i;
	struct dvobj_priv *dvobj = (struct dvobj_priv *)d;
	struct data_urb *recvurb = (struct data_urb *)dvobj->recv_urb_q.urb_buf;
	/*Elwin_todo need use correct literecvbuf_nr recvurb_nr */
	u32 recvurb_nr = RTW_RECVURB_NR;
#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
	u32 initinurb_nr = RTW_INTINURB_NR;
#endif

	if (dvobj == NULL) {
		RTW_ERR("%s dvobj is NULL\n", __func__);
		rtw_warn_on(1);
		return;
	}
	RTW_INFO("%s\n", __func__);

	
	for (i = 0; i < recvurb_nr; i++) {
		usb_kill_urb(recvurb->urb);
		recvurb++;
	}


#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
	recvurb = (struct data_urb *)dvobj->intin_urb_q.urb_buf;
	for (i = 0; i < initinurb_nr; i++) {
		usb_kill_urb(recvurb->urb);
		recvurb++;
	}
#endif
}

