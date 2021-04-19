/******************************************************************************
 *
 * Copyright(c) 2015 - 2017 Realtek Corporation.
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
#define _RTL8822CU_RECV_C_

#include <drv_types.h>			/* PADAPTER, rtw_xmit.h and etc. */
#include <hal_data.h>			/* HAL_DATA_TYPE */
#include "../../hal_halmac.h"		/* RX desc */
#include "../rtl8822c.h"		/* rtl8822c_query_rx_desc, rtl8822c_c2h_handler_no_io() */

static u8 recvbuf2recvframe_proccess_c2h(PADAPTER padapter, u8 *pbuf, s32 transfer_len)
{
	u8 ret = _SUCCESS;

	/* send rx desc + c2h content to halmac */
	rtl8822c_c2h_handler_no_io(padapter, pbuf, transfer_len);

	return ret;
}

static u8 recvbuf2recvframe_proccess_normal_rx
(PADAPTER padapter, u8 *pbuf, struct rx_pkt_attrib *pattrib, union recv_frame *precvframe, struct sk_buff *pskb)
{
	u8 ret = _SUCCESS;
	struct recv_priv *precvpriv = &adapter_to_dvobj(padapter)->recvpriv;

#ifdef CONFIG_RX_PACKET_APPEND_FCS
	if (check_fwstate(&padapter->mlmepriv, WIFI_MONITOR_STATE) == _FALSE) {
		if (rtl8822c_rx_fcs_appended(padapter))
			pattrib->pkt_len -= IEEE80211_FCS_LEN;
	}
#endif

	if (rtw_os_alloc_recvframe(padapter, precvframe,
		(pbuf + pattrib->shift_sz + pattrib->drvinfo_sz + RXDESC_SIZE), pskb) == _FAIL) {

		rtw_free_recvframe(precvframe);
		ret = _FAIL;
		goto exit;
	}

	recvframe_put(precvframe, pattrib->pkt_len);

	pre_recv_entry(precvframe, pattrib->physt ? (pbuf + RXDESC_OFFSET) : NULL);

exit:
	return ret;
}

