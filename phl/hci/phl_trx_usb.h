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
#ifndef _PHL_TRX_USB_H_
#define _PHL_TRX_USB_H_

#define MGMT_TID 8
#define PKT_OFFSET_DUMMY	(8)
//
// USB specific.
//
#define WLAN_IN_MPDU_PIPE_IDX			0	//EP1, RxIn Pipe
#define WLAN_IN_INTERRUPT_PIPE_IDX		1	//EP7, HISR or C2H Cmd In Pipe

struct rtw_rx_buf_ring {
	//
	// Memory block of phl_usb_rx_pkt.
	//
	struct rtw_rx_buf *rxbufblock;
	u32 	block_cnt_alloc;	// Total number of rtw_rx_buf allocated.
	u32 	total_blocks_size;	// block_cnt_alloc	*  sizeof( rtw_rx_buf )

	struct phl_queue idle_rxbuf_list;
	struct phl_queue busy_rxbuf_list;
	struct phl_queue pend_rxbuf_list;
	_os_timer deferred_timer;
};


struct phl_usb_tx_buf_resource  {
	struct phl_queue idle_txbuf_list;
	struct phl_queue mgmt_txbuf_list;
	struct phl_queue h2c_txbuf_list;
};






enum rtw_phl_status phl_hook_trx_ops_usb(void *phl);

#endif	/* _PHL_TRX_USB_H_ */
