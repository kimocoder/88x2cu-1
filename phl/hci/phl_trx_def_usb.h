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
#ifndef _PHL_TRX_DEF_USB_H_
#define _PHL_TRX_DEF_USB_H_

/* for tx */
struct phl_usb_buf {
	_os_list list;
	u8 *buffer;
	u32 buf_len;
	u8 type;
};


/* for rx */
struct rtw_rx_buf {
	_os_list list;
	u8 *buffer;
	u32 buf_len;
	u32 transfer_len;
	u8 pipe_idx;

	_os_lock lock;	// using phl_queue??
	int pktcnt;		// for usb aggregation
	void *os_priv;
};

#endif	/* _PHL_TRX_DEF_USB_H_ */
