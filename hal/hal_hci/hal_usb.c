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

