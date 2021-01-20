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


//int usbctrl_vendorreq(struct intf_hdl *pintfhdl, u8 request, u16 value, u16 index, void *pdata, u16 len, u8 requesttype)
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

struct rtw_async_write_data {
	u8 data[VENDOR_CMD_MAX_DATA_LEN];
	struct usb_ctrlrequest dr;
};

#ifdef CONFIG_USB_SUPPORT_ASYNC_VDN_REQ
static void _usbctrl_vendorreq_async_callback(struct urb *urb, struct pt_regs *regs)
{
	if (urb) {
		if (urb->context)
			rtw_mfree(urb->context, sizeof(struct rtw_async_write_data));
		usb_free_urb(urb);
	}
}

int _usbctrl_vendorreq_async_write(struct usb_device *udev, u8 request,
	u16 value, u16 index, void *pdata, u16 len, u8 requesttype)
{
	int rc;
	unsigned int pipe;
	u8 reqtype;
	struct usb_ctrlrequest *dr;
	struct urb *urb;
	struct rtw_async_write_data *buf;


	if (requesttype == VENDOR_READ) {
		pipe = usb_rcvctrlpipe(udev, 0);/* read_in */
		reqtype =  REALTEK_USB_VENQT_READ;
	} else {
		pipe = usb_sndctrlpipe(udev, 0);/* write_out */
		reqtype =  REALTEK_USB_VENQT_WRITE;
	}

	buf = (struct rtl819x_async_write_data *)rtw_zmalloc(sizeof(*buf));
	if (!buf) {
		rc = -ENOMEM;
		goto exit;
	}

	urb = usb_alloc_urb(0, GFP_ATOMIC);
	if (!urb) {
		rtw_mfree((u8 *)buf, sizeof(*buf));
		rc = -ENOMEM;
		goto exit;
	}

	dr = &buf->dr;

	dr->bRequestType = reqtype;
	dr->bRequest = request;
	dr->wValue = cpu_to_le16(value);
	dr->wIndex = cpu_to_le16(index);
	dr->wLength = cpu_to_le16(len);

	_rtw_memcpy(buf, pdata, len);

	usb_fill_control_urb(urb, udev, pipe, (unsigned char *)dr, buf, len,
		_usbctrl_vendorreq_async_callback, buf);

	rc = usb_submit_urb(urb, GFP_ATOMIC);
	if (rc < 0) {
		rtw_mfree((u8 *)buf, sizeof(*buf));
		usb_free_urb(urb);
	}

exit:
	return rc;
}


#endif /* CONFIG_USB_SUPPORT_ASYNC_VDN_REQ */


#if (KERNEL_VERSION(2, 5, 0) > LINUX_VERSION_CODE) ||\
	(KERNEL_VERSION(2, 6, 18) < LINUX_VERSION_CODE)
/*#define _usbctrl_vendorreq_async_callback(urb, regs)\*/
	/*_usbctrl_vendorreq_async_callback(urb)*/
/*#define usb_bulkout_zero_complete(purb, regs)\*/
	/*usb_bulkout_zero_complete(purb)*/
#define rtw_usb_write_port_complete(purb, regs)\
	rtw_usb_write_port_complete(purb)
#define rtw_usb_g6_read_port_complete(purb, regs)\
	rtw_usb_g6_read_port_complete(purb)
#define rtw_usb_read_interrupt_complete(purb, regs)\
	rtw_usb_read_interrupt_complete(purb)
#endif

unsigned int bulkid2pipe(struct dvobj_priv *pdvobj, u32 addr, u8 bulk_out)
{
	unsigned int pipe = 0, ep_num = 0;
	PUSB_DATA pusb_data = dvobj_to_usb(pdvobj);
	struct usb_device *pusbd = pusb_data->pusbdev;

	RTW_INFO("%s : NEO : addr == 0x%x, bulk_out=%d\n", __func__, addr, bulk_out);

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

struct zero_bulkout_context {
	void *pbuf;
	void *purb;
	void *pirp;
	void *padapter;
};

void usb_read_mem(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *rmem)
{

}


void usb_read_port_cancel(struct intf_hdl *pintfhdl)
{
	int i;
	struct recv_buf *precvbuf;
	_adapter	*padapter = pintfhdl->padapter;
	struct registry_priv *regsty = adapter_to_regsty(padapter);
	precvbuf = (struct recv_buf *)adapter_to_dvobj(padapter)->recvpriv.precv_buf;

	RTW_INFO("%s\n", __func__);

	for (i = 0; i < regsty->recvbuf_nr ; i++) {

		if (precvbuf->purb)	 {
			/* RTW_INFO("usb_read_port_cancel : usb_kill_urb\n"); */
			usb_kill_urb(precvbuf->purb);
		}
		precvbuf++;
	}

#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
	usb_kill_urb(adapter_to_dvobj(padapter)->recvpriv.int_in_urb);
#endif
}

static void rtw_usb_write_port_complete(struct urb *purb, struct pt_regs *regs)
{
	_irqL irqL;
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
		RTW_INFO("%s(): TX Warning! bDriverStopped(%s) OR bSurpriseRemoved(%s) pxmitbuf->buf_tag(%x)\n"
			 , __func__
			 , dev_is_drv_stopped(pdvobj) ? "True" : "False"
			 , dev_is_surprise_removed(pdvobj) ? "True" : "False"
			 , pxmitbuf->buf_tag);

		goto check_completion;
	}


	if (purb->status == 0) {

	} else {
		RTW_INFO("###=> urb_write_port_complete status(%d)\n", purb->status);
		if ((purb->status == -EPIPE) || (purb->status == -EPROTO)) {
			/* usb_clear_halt(pusbdev, purb->pipe);	 */
			/* msleep(10); */
			sreset_set_wifi_error_status(padapter, USB_WRITE_PORT_FAIL);
		} else if (purb->status == -EINPROGRESS) {
			goto check_completion;

		} else if (purb->status == -ENOENT) {
			RTW_INFO("%s: -ENOENT\n", __func__);
			goto check_completion;

		} else if (purb->status == -ECONNRESET) {
			RTW_INFO("%s: -ECONNRESET\n", __func__);
			goto check_completion;

		} else if (purb->status == -ESHUTDOWN) {
			rtw_set_drv_stopped(padapter);

			goto check_completion;
		} else {
			rtw_set_surprise_removed(padapter);
			RTW_INFO("bSurpriseRemoved=TRUE\n");

			goto check_completion;
		}
	}

	#ifdef DBG_CONFIG_ERROR_DETECT
	{
		HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
		pHalData->srestpriv.last_tx_complete_time = rtw_get_current_time();
	}
	#endif

check_completion:
	_enter_critical(&pxmitpriv->lock_sctx, &irqL);
	rtw_sctx_done_err(&pxmitbuf->sctx,
		purb->status ? RTW_SCTX_DONE_WRITE_PORT_ERR : RTW_SCTX_DONE_SUCCESS);
	_exit_critical(&pxmitpriv->lock_sctx, &irqL);

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
		RTW_INFO(" DBG_TX %s:%d bDriverStopped%s, bSurpriseRemoved:%s\n", __func__, __LINE__
			 , dev_is_drv_stopped(pdvobj) ? "True" : "False"
			, dev_is_surprise_removed(pdvobj) ? "True" : "False");
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

	//purb	= pxmitbuf->pxmit_urb[0];
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
		#ifdef DBG_CONFIG_ERROR_DETECT
		{
			HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
			pHalData->srestpriv.last_tx_time = rtw_get_current_time();
		}
		#endif
	} else {
		rtw_sctx_done_err(&pxmitbuf->sctx, RTW_SCTX_DONE_WRITE_PORT_ERR);
		RTW_INFO("usb_write_port, status=%d\n", status);

		switch (status) {
		case -ENODEV:
			rtw_set_drv_stopped(padapter);
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

#if 0
	RTW_INFO("%s\n", __func__);

	for (i = 0; i < NR_XMITBUFF; i++) {
		for (j = 0; j < 8; j++) {
			if (pxmitbuf->pxmit_urb[j])
				usb_kill_urb(pxmitbuf->pxmit_urb[j]);
		}
		pxmitbuf++;
	}

	pxmitbuf = (struct xmit_buf *)padapter->xmitpriv.pxmit_extbuf;
	for (i = 0; i < NR_XMIT_EXTBUFF ; i++) {
		for (j = 0; j < 8; j++) {
			if (pxmitbuf->pxmit_urb[j])
				usb_kill_urb(pxmitbuf->pxmit_urb[j]);
		}
		pxmitbuf++;
	}
#endif
}

void usb_init_recvbuf(_adapter *padapter, struct recv_buf *precvbuf)
{

	precvbuf->transfer_len = 0;

	precvbuf->len = 0;

	precvbuf->ref_cnt = 0;

	if (precvbuf->pbuf) {
		precvbuf->pdata = precvbuf->phead = precvbuf->ptail = precvbuf->pbuf;
		precvbuf->pend = precvbuf->pdata + MAX_RECVBUF_SZ;
	}

}

int recvbuf2recvframe(PADAPTER padapter, void *ptr);

#ifdef CONFIG_USE_USB_BUFFER_ALLOC_RX
aa
void usb_recv_tasklet(void *priv)
{
	struct recv_buf *precvbuf = NULL;
	_adapter	*padapter = (_adapter *)priv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct recv_priv	*precvpriv = &adapter_to_dvobj(padapter)->recvpriv;

	while (NULL != (precvbuf = rtw_dequeue_recvbuf(&precvpriv->recv_buf_pending_queue))) {
		if (RTW_CANNOT_RUN(dvobj)) {
			RTW_INFO("recv_tasklet => bDriverStopped(%s) OR bSurpriseRemoved(%s)\n"
				, dev_is_drv_stopped(dvobj)? "True" : "False"
				, dev_is_surprise_removed(dvobj)? "True" : "False");
			break;
		}

		recvbuf2recvframe(padapter, precvbuf);

		rtw_read_port(padapter, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
	}
}

void usb_read_port_complete(struct urb *purb, struct pt_regs *regs)
{
	struct recv_buf	*precvbuf = (struct recv_buf *)purb->context;
	_adapter			*padapter = (_adapter *)precvbuf->adapter;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct recv_priv	*precvpriv = &adapter_to_dvobj(padapter)->recvpriv;

	ATOMIC_DEC(&(precvpriv->rx_pending_cnt));

	if (RTW_CANNOT_RX(dvobj)) {
		RTW_INFO("%s() RX Warning! bDriverStopped(%s) OR bSurpriseRemoved(%s)\n"
			 , __func__
			 , dev_is_drv_stopped(dvobj) ? "True" : "False"
			, dev_is_surprise_removed(dvobj) ? "True" : "False");
		return;
	}

	if (purb->status == 0) {

		if ((purb->actual_length > MAX_RECVBUF_SZ) || (purb->actual_length < RXDESC_SIZE)) {
			RTW_INFO("%s()-%d: urb->actual_length:%u, MAX_RECVBUF_SZ:%u, RXDESC_SIZE:%u\n"
				, __FUNCTION__, __LINE__, purb->actual_length, MAX_RECVBUF_SZ, RXDESC_SIZE);
			rtw_read_port(padapter, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
		} else {
			rtw_reset_continual_io_error(adapter_to_dvobj(padapter));

			precvbuf->transfer_len = purb->actual_length;

			rtw_enqueue_recvbuf(precvbuf, &precvpriv->recv_buf_pending_queue);

			tasklet_schedule(&precvpriv->recv_tasklet);
		}
	} else {

		RTW_INFO("###=> usb_read_port_complete => urb.status(%d)\n", purb->status);

		if (rtw_inc_and_chk_continual_io_error(adapter_to_dvobj(padapter)) == _TRUE)
			rtw_set_surprise_removed(padapter);

		switch (purb->status) {
		case -EINVAL:
		case -EPIPE:
		case -ENODEV:
		case -ESHUTDOWN:
		case -ENOENT:
			rtw_set_drv_stopped(padapter);
			break;
		case -EPROTO:
		case -EILSEQ:
		case -ETIME:
		case -ECOMM:
		case -EOVERFLOW:
			#ifdef DBG_CONFIG_ERROR_DETECT
			{
				HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
				pHalData->srestpriv.Wifi_Error_Status = USB_READ_PORT_FAIL;
			}
			#endif
			rtw_read_port(padapter, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
			break;
		case -EINPROGRESS:
			RTW_INFO("ERROR: URB IS IN PROGRESS!/n");
			break;
		default:
			break;
		}
	}

}

u32 rtw_usb_read_port(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *rmem)
{
	int err;
	unsigned int pipe;
	u32 ret = _SUCCESS;
	PURB purb = NULL;
	struct recv_buf	*precvbuf = (struct recv_buf *)rmem;
	_adapter		*adapter = pintfhdl->padapter;
	struct dvobj_priv	*pdvobj = adapter_to_dvobj(adapter);
	struct pwrctrl_priv *pwrctl = dvobj_to_pwrctl(pdvobj);
	struct recv_priv	*precvpriv = &adapter_to_dvobj(adapter)->recvpriv;
	struct usb_device	*pusbd = pdvobj->pusbdev;


	if (RTW_CANNOT_RX(pdvobj) || (precvbuf == NULL)) {
		return _FAIL;
	}

	usb_init_recvbuf(adapter, precvbuf);

	if (precvbuf->pbuf) {
		ATOMIC_INC(&(precvpriv->rx_pending_cnt));
		purb = precvbuf->purb;

		/* translate DMA FIFO addr to pipehandle */
		pipe = bulkid2pipe(pdvobj, addr, _FALSE);

		usb_fill_bulk_urb(purb, pusbd, pipe,
			precvbuf->pbuf,
			MAX_RECVBUF_SZ,
			usb_read_port_complete,
			precvbuf);/* context is precvbuf */

		purb->transfer_dma = precvbuf->dma_transfer_addr;
		purb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

		err = usb_submit_urb(purb, GFP_ATOMIC);
		if ((err) && (err != (-EPERM))) {
			RTW_INFO("cannot submit rx in-token(err = 0x%08x),urb_status = %d\n", err, purb->status);
			ret = _FAIL;
		}

	}


	return ret;
}
#else	/* CONFIG_USE_USB_BUFFER_ALLOC_RX */
//NEO
void usb_recv_tasklet(void *priv)
{
	struct sk_buff		*pskb;
	_adapter		*padapter = (_adapter *)priv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct recv_priv	*precvpriv = &dvobj->recvpriv;
	struct recv_buf	*precvbuf = NULL;

	while (NULL != (pskb = skb_dequeue(&precvpriv->rx_skb_queue))) {

		if (RTW_CANNOT_RUN(dvobj)) {
			RTW_INFO("recv_tasklet => bDriverStopped(%s) OR bSurpriseRemoved(%s)\n"
				, dev_is_drv_stopped(dvobj) ? "True" : "False"
				, dev_is_surprise_removed(dvobj) ? "True" : "False");
			#ifdef CONFIG_PREALLOC_RX_SKB_BUFFER
			if (rtw_free_skb_premem(pskb) != 0)
			#endif /* CONFIG_PREALLOC_RX_SKB_BUFFER */
				rtw_skb_free(pskb);
			break;
		}

		recvbuf2recvframe(padapter, pskb);

		skb_reset_tail_pointer(pskb);
		pskb->len = 0;

		skb_queue_tail(&precvpriv->free_recv_skb_queue, pskb);

		precvbuf = rtw_dequeue_recvbuf(&precvpriv->recv_buf_pending_queue);
		if (NULL != precvbuf) {
			precvbuf->pskb = NULL;
			rtw_read_port(padapter, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
		}
	}
}

void usb_read_port_complete(struct urb *purb, struct pt_regs *regs)
{
	struct recv_buf	*precvbuf = (struct recv_buf *)purb->context;
	_adapter			*padapter = (_adapter *)precvbuf->adapter;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct recv_priv	*precvpriv = &adapter_to_dvobj(padapter)->recvpriv;

	ATOMIC_DEC(&(precvpriv->rx_pending_cnt));

	if (RTW_CANNOT_RX(dvobj)) {
		RTW_INFO("%s() RX Warning! bDriverStopped(%s) OR bSurpriseRemoved(%s)\n"
			, __func__
			, dev_is_drv_stopped(dvobj) ? "True" : "False"
			, dev_is_surprise_removed(dvobj) ? "True" : "False");
		goto exit;
	}

	if (purb->status == 0) {

		if ((purb->actual_length > MAX_RECVBUF_SZ) || (purb->actual_length < RXDESC_SIZE)) {
			RTW_INFO("%s()-%d: urb->actual_length:%u, MAX_RECVBUF_SZ:%u, RXDESC_SIZE:%u\n"
				, __FUNCTION__, __LINE__, purb->actual_length, MAX_RECVBUF_SZ, RXDESC_SIZE);
			rtw_read_port(padapter, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
		} else {
			rtw_reset_continual_io_error(adapter_to_dvobj(padapter));

			precvbuf->transfer_len = purb->actual_length;
			skb_put(precvbuf->pskb, purb->actual_length);
			skb_queue_tail(&precvpriv->rx_skb_queue, precvbuf->pskb);

			#ifndef CONFIG_FIX_NR_BULKIN_BUFFER
			if (skb_queue_len(&precvpriv->rx_skb_queue) <= 1)
			#endif
				tasklet_schedule(&precvpriv->recv_tasklet);

			precvbuf->pskb = NULL;
			rtw_read_port(padapter, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
		}
	} else {

		RTW_INFO("###=> usb_read_port_complete => urb.status(%d)\n", purb->status);

		if (rtw_inc_and_chk_continual_io_error(adapter_to_dvobj(padapter)) == _TRUE)
			rtw_set_surprise_removed(padapter);

		switch (purb->status) {
		case -EINVAL:
		case -EPIPE:
		case -ENODEV:
		case -ESHUTDOWN:
		case -ENOENT:
			rtw_set_drv_stopped(padapter);
			break;
		case -EPROTO:
		case -EILSEQ:
		case -ETIME:
		case -ECOMM:
		case -EOVERFLOW:
			#ifdef DBG_CONFIG_ERROR_DETECT
			{
				HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
				pHalData->srestpriv.Wifi_Error_Status = USB_READ_PORT_FAIL;
			}
			#endif
			rtw_read_port(padapter, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
			break;
		case -EINPROGRESS:
			RTW_INFO("ERROR: URB IS IN PROGRESS!/n");
			break;
		default:
			break;
		}
	}

exit:
	return;
}

u32 rtw_usb_read_port(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *rmem)
{
	int err;
	unsigned int pipe;
	u32 ret = _FAIL;
	PURB purb = NULL;
	struct recv_buf	*precvbuf = (struct recv_buf *)rmem;
	_adapter		*adapter = pintfhdl->padapter;
	struct dvobj_priv	*pdvobj = adapter_to_dvobj(adapter);
	struct recv_priv	*precvpriv = &adapter_to_dvobj(adapter)->recvpriv;
	PUSB_DATA 		pusb_data = dvobj_to_usb(pdvobj);
	struct usb_device 	*pusbd = pusb_data->pusbdev;


	if (RTW_CANNOT_RX(pdvobj) || (precvbuf == NULL)) {
		goto exit;
	}

	usb_init_recvbuf(adapter, precvbuf);

	if (precvbuf->pskb == NULL) {
		SIZE_PTR tmpaddr = 0;
		SIZE_PTR alignment = 0;

		precvbuf->pskb = skb_dequeue(&precvpriv->free_recv_skb_queue);
		if (NULL != precvbuf->pskb)
			goto recv_buf_hook;

		#ifndef CONFIG_FIX_NR_BULKIN_BUFFER
		precvbuf->pskb = rtw_skb_alloc(MAX_RECVBUF_SZ + RECVBUFF_ALIGN_SZ);
		#endif

		if (precvbuf->pskb == NULL) {
			if (0)
				RTW_INFO("usb_read_port() enqueue precvbuf=%p\n", precvbuf);
			/* enqueue precvbuf and wait for free skb */
			rtw_enqueue_recvbuf(precvbuf, &precvpriv->recv_buf_pending_queue);
			goto exit;
		}

		tmpaddr = (SIZE_PTR)precvbuf->pskb->data;
		alignment = tmpaddr & (RECVBUFF_ALIGN_SZ - 1);
		skb_reserve(precvbuf->pskb, (RECVBUFF_ALIGN_SZ - alignment));
	}

recv_buf_hook:
	precvbuf->phead = precvbuf->pskb->head;
	precvbuf->pdata = precvbuf->pskb->data;
	precvbuf->ptail = skb_tail_pointer(precvbuf->pskb);
	precvbuf->pend = skb_end_pointer(precvbuf->pskb);
	precvbuf->pbuf = precvbuf->pskb->data;

	purb = precvbuf->purb;

	/* translate DMA FIFO addr to pipehandle */
	pipe = bulkid2pipe(pdvobj, addr, _FALSE);

	usb_fill_bulk_urb(purb, pusbd, pipe,
		precvbuf->pbuf,
		MAX_RECVBUF_SZ,
		usb_read_port_complete,
		precvbuf);

	err = usb_submit_urb(purb, GFP_ATOMIC);
	if (err && err != (-EPERM)) {
		RTW_INFO("cannot submit rx in-token(err = 0x%08x),urb_status = %d\n"
			, err, purb->status);
		goto exit;
	}

	ATOMIC_INC(&(precvpriv->rx_pending_cnt));
	ret = _SUCCESS;

exit:


	return ret;
}
#endif /* CONFIG_USE_USB_BUFFER_ALLOC_RX */

#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
void usb_read_interrupt_complete(struct urb *purb, struct pt_regs *regs)
{
	int	err;
	_adapter	*padapter = (_adapter *)purb->context;

	if (RTW_CANNOT_RX(adapter_to_dvobj(padapter))) {
		RTW_INFO("%s() RX Warning! bDriverStopped(%s) OR bSurpriseRemoved(%s)\n"
			, __func__
			, rtw_is_drv_stopped(padapter) ? "True" : "False"
			, rtw_is_surprise_removed(padapter) ? "True" : "False");

		return;
	}

	if (purb->status == 0) {/*SUCCESS*/
		if (purb->actual_length > INTERRUPT_MSG_FORMAT_LEN)
			RTW_INFO("usb_read_interrupt_complete: purb->actual_length > INTERRUPT_MSG_FORMAT_LEN(%d)\n", INTERRUPT_MSG_FORMAT_LEN);

		rtw_hal_interrupt_handler(padapter, purb->actual_length, purb->transfer_buffer);

		err = usb_submit_urb(purb, GFP_ATOMIC);
		if ((err) && (err != (-EPERM)))
			RTW_INFO("cannot submit interrupt in-token(err = 0x%08x),urb_status = %d\n", err, purb->status);
	} else {
		RTW_INFO("###=> usb_read_interrupt_complete => urb status(%d)\n", purb->status);

		switch (purb->status) {
		case -EINVAL:
		case -EPIPE:
		case -ENODEV:
		case -ESHUTDOWN:
		case -ENOENT:
			rtw_set_drv_stopped(padapter);
			break;
		case -EPROTO:
			break;
		case -EINPROGRESS:
			RTW_INFO("ERROR: URB IS IN PROGRESS!/n");
			break;
		default:
			break;
		}
	}
}

u32 usb_read_interrupt(struct intf_hdl *pintfhdl, u32 addr)
{
	int	err;
	unsigned int pipe;
	u32	ret = _SUCCESS;
	_adapter			*adapter = pintfhdl->padapter;
	struct dvobj_priv	*pdvobj = adapter_to_dvobj(adapter);
	struct recv_priv	*precvpriv = &adapter_to_dvobj(adapter)->recvpriv;
	struct usb_device	*pusbd = pdvobj->pusbdev;


	if (RTW_CANNOT_RX(pdvobj)) {
		return _FAIL;
	}

	/*translate DMA FIFO addr to pipehandle*/
	pipe = bulkid2pipe(pdvobj, addr, _FALSE);

	usb_fill_int_urb(precvpriv->int_in_urb, pusbd, pipe,
			precvpriv->int_in_buf,
			INTERRUPT_MSG_FORMAT_LEN,
			usb_read_interrupt_complete,
			adapter,
			1);

	err = usb_submit_urb(precvpriv->int_in_urb, GFP_ATOMIC);
	if ((err) && (err != (-EPERM))) {
		RTW_INFO("cannot submit interrupt in-token(err = 0x%08x), urb_status = %d\n", err, precvpriv->int_in_urb->status);
		ret = _FAIL;
	}

	return ret;
}
#endif /* CONFIG_USB_INTERRUPT_IN_PIPE */


static void rtw_usb_g6_read_port_complete(struct urb *urb, struct pt_regs *regs)
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

	rtw_phl_post_in_complete(dvobj->phl, literecvbuf->phl_buf_ptr, actual_length, status);
	rtw_free_litedatabuf(rx_data_buf_q, literecvbuf);
	rtw_free_dataurb(rx_urb_q, recvurb);
}

u32 rtw_usb_g6_read_port(void *d, void *rxobj,
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
			rtw_usb_g6_read_port_complete,
			literecvbuf);
	} else {
		#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
		if (usb_data->inpipe_type[bulk_id] == REALTEK_USB_BULK_IN_EP_IDX)
			usb_fill_bulk_urb(recvurb->urb, usbd, pipe,
				literecvbuf->pbuf,
				inbuf_len,
				rtw_usb_g6_read_port_complete,
				literecvbuf);
		else
			usb_fill_int_urb(recvurb->urb, usbd, pipe,
				literecvbuf->pbuf,
				inbuf_len,
				rtw_usb_g6_read_port_complete,
				literecvbuf,
				1);
		#endif
	}

	RTW_INFO("%s : NEO: rxbuf: %p, size: %d\n", __func__, literecvbuf->pbuf, inbuf_len);

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

void rtw_usb_g6_read_port_cancel(void *d)
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

