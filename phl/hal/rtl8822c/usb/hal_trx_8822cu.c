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
#define _HAL_TRX_8822AU_C_
#include "../../hal_headers.h"
#include "../rtl8822c_hal.h"
#include "hal_trx_8822cu.h"


#if 0 // NEO mark off first

static void _hal_dump_rxdesc(u8 *buf, struct rtw_r_meta_data *mdata)
{
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "%s ==>\n", __FUNCTION__);

	debug_dump_data(buf, 56, "_hal_dump_rxdesc:: ");

	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->pktlen = 0x%X\n", mdata->pktlen);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->shift = 0x%X\n", mdata->shift);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->wl_hd_iv_len = 0x%X\n",
											mdata->wl_hd_iv_len);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->bb_sel = 0x%X\n",
											mdata->bb_sel);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->mac_info_vld = 0x%X\n",
											mdata->mac_info_vld);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->rpkt_type = 0x%X\n",
											mdata->rpkt_type);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->drv_info_size = 0x%X\n",
											mdata->drv_info_size);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->long_rxd = 0x%X\n",
											mdata->long_rxd);

	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->ppdu_type = 0x%X\n",
											mdata->ppdu_type);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->ppdu_cnt = 0x%X\n",
											mdata->ppdu_cnt);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->sr_en = 0x%X\n",
											mdata->sr_en);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->user_id = 0x%X\n",
											mdata->user_id);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->rx_rate = 0x%X\n",
											mdata->rx_rate);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->rx_gi_ltf = 0x%X\n",
											mdata->rx_gi_ltf);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->non_srg_ppdu = 0x%X\n",
											mdata->non_srg_ppdu);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->inter_ppdu = 0x%X\n",
											mdata->inter_ppdu);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->bw = 0x%X\n",
											mdata->bw );

	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->freerun_cnt = 0x%X\n",
											mdata->freerun_cnt);

	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->a1_match = 0x%X\n",
											mdata->a1_match);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->sw_dec = 0x%X\n",
											mdata->sw_dec);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->hw_dec = 0x%X\n",
											mdata->hw_dec);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->ampdu = 0x%X\n",
											mdata->ampdu);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->ampdu_end_pkt = 0x%X\n",
											mdata->ampdu_end_pkt);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->amsdu = 0x%X\n",
											mdata->amsdu);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->amsdu_cut = 0x%X\n",
											mdata->amsdu_cut);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->last_msdu = 0x%X\n",
											mdata->last_msdu);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->bypass = 0x%X\n",
											mdata->bypass);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->crc32 = 0x%X\n",
											mdata->crc32);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->icverr = 0x%X\n",
											mdata->icverr);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->magic_wake = 0x%X\n",
											mdata->magic_wake);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->unicast_wake = 0x%X\n",
											mdata->unicast_wake);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->pattern_wake = 0x%X\n",
											mdata->pattern_wake);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->get_ch_info = 0x%X \n",
											mdata->get_ch_info);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->rx_statistics = 0x%X\n",
											mdata->rx_statistics);

	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->pattern_idx = 0x%X\n",
											mdata->pattern_idx);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->target_idc = 0x%X\n",
											mdata->target_idc);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->chksum_ofld_en = 0x%X\n",
											mdata->chksum_ofld_en);
	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->with_llc = 0x%X\n",
											mdata->with_llc);


	if (mdata->long_rxd==1)
	{
		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->frame_type = 0x%X\n",
											mdata->frame_type);
		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->mc = 0x%X\n",
											mdata->mc);
		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->bc = 0x%X\n",
											mdata->bc);
		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->more_data = 0x%X\n",
											mdata->more_data);
		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->more_frag = 0x%X\n",
											mdata->more_frag);
		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->pwr_bit = 0x%X\n",
											mdata->pwr_bit);
		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->qos = 0x%X\n",
											mdata->qos);

		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->tid = 0x%X\n",
											mdata->tid);
		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->eosp = 0x%X\n",
											mdata->eosp);
		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->htc = 0x%X\n",
											mdata->htc);
		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->q_null = 0x%X\n",
											mdata->q_null);

		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->seq = 0x%X\n",
											mdata->seq);
		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->frag_num = 0x%X\n",
											mdata->frag_num);

		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->sec_cam_idx = 0x%X\n",
											mdata->sec_cam_idx);
		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->addr_cam = 0x%X\n",
											mdata->addr_cam);

	PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->macid = 0x%X\n\n",
											mdata->macid);
		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->rx_pl_id = 0x%X\n",
											mdata->rx_pl_id);
		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->addr_cam_vld = 0x%X\n",
											mdata->addr_cam_vld);
		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->addr_fwd_en = 0x%X\n",
											mdata->addr_fwd_en);
		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_, "mdata->rx_pl_match = 0x%X\n",
											mdata->rx_pl_match);

		debug_dump_data(mdata->mac_addr, 6, "mdata->mac_addr = \n");
	}



}


/**
 * the function will initializing 8852au specific data and hw configuration
 */
enum rtw_hal_status hal_trx_init_8852au(struct hal_info_t *hal)
{
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;

	return hstatus;
}
/**
 * the function will deinitializing 8852au specific data and hw configuration
 */
static void hal_trx_deinit_8852au(struct hal_info_t *hal)
{
}

static u8 hal_mapping_hw_tx_chnl_8852au(u16 macid, enum rtw_phl_ring_cat cat,
					u8 band)
{
	u8 dma_ch = 0;

	/* hana_todo, decided by tid only currently,
	   we should consider more situation later */

	if (0 == band) {
		switch (cat) {
		case RTW_PHL_RING_CAT_TID0:/*AC_BE*/
		case RTW_PHL_RING_CAT_TID3:
		case RTW_PHL_RING_CAT_TID6:/*AC_VO*/
		case RTW_PHL_RING_CAT_TID7:
			dma_ch = ACH0_QUEUE_IDX_8852A;
			break;
		case RTW_PHL_RING_CAT_TID1:/*AC_BK*/
		case RTW_PHL_RING_CAT_TID2:
		case RTW_PHL_RING_CAT_TID4:/*AC_VI*/
		case RTW_PHL_RING_CAT_TID5:
			dma_ch = ACH2_QUEUE_IDX_8852A;
			break;
		case RTW_PHL_RING_CAT_MGNT:
			dma_ch = MGQ_B0_QUEUE_IDX_8852A;
			break;
		case RTW_PHL_RING_CAT_HIQ:
			dma_ch = HIQ_B0_QUEUE_IDX_8852A;
			break;
		default:
			dma_ch = ACH0_QUEUE_IDX_8852A;
			PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "[WARNING]unknown category (%d)\n",
				  cat);
			break;
		}
	} else if (1 == band) {
		switch (cat) {
		case RTW_PHL_RING_CAT_TID0:/*AC_BE*/
		case RTW_PHL_RING_CAT_TID3:
		case RTW_PHL_RING_CAT_TID6:/*AC_VO*/
		case RTW_PHL_RING_CAT_TID7:
			dma_ch = ACH4_QUEUE_IDX_8852A;
			break;
		case RTW_PHL_RING_CAT_TID1:/*AC_BK*/
		case RTW_PHL_RING_CAT_TID2:
		case RTW_PHL_RING_CAT_TID4:/*AC_VI*/
		case RTW_PHL_RING_CAT_TID5:
			dma_ch = ACH6_QUEUE_IDX_8852A;
			break;
		case RTW_PHL_RING_CAT_MGNT:
			dma_ch = MGQ_B1_QUEUE_IDX_8852A;
			break;
		case RTW_PHL_RING_CAT_HIQ:
			dma_ch = HIQ_B1_QUEUE_IDX_8852A;
			break;
		default:
			dma_ch = ACH0_QUEUE_IDX_8852A;
			PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "[WARNING]unknown category (%d)\n",
				  cat);
			break;
		}
	} else {
		dma_ch = ACH0_QUEUE_IDX_8852A;
		PHL_TRACE(COMP_PHL_DBG, _PHL_WARNING_, "[WARNING]unknown band (%d)\n",
			  band);
	}

	return dma_ch;
}

#endif // if 0 NEO

static enum rtw_hal_status hal_query_info_8822cu(struct hal_info_t *hal, u8 info_id, void *value)
{
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_SUCCESS;

	switch (info_id){
		case RTW_HAL_RXDESC_SIZE:
			/* wifi packet(RXD.RPKT_TYPE = 0x0) = 32 bytes, otherwise 16 bytes */
			*((u8 *)value) = RX_DESC_S_SIZE_8822C;
			break;
		default:
			hstatus = RTW_HAL_STATUS_FAILURE;
			break;
	}
	return hstatus;
}

#if 0 // NEO mark off first

static enum rtw_hal_status hal_pltfm_tx_8852au(void *hal,
							struct rtw_h2c_pkt *pkt)
{
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;

	return hstatus;
}

u8 hal_get_bulkout_id_8852au(struct hal_info_t *hal, u8 dma_ch, u8 mode)
{
	return hal_mac_get_bulkout_id(hal, dma_ch, mode);
}

u8 hal_get_max_bulkout_wd_num_8852au(struct hal_info_t *hal)
{
	return hal_mac_usb_get_max_bulkout_wd_num(hal);
}

/**
 * the function update wd page, including wd info, wd body, seq info
 * @hal: see struct hal_info_t
 * @phl_pkt_req: see struct rtw_phl_pkt_req
 */
enum rtw_hal_status
hal_fill_wd_8852au(struct hal_info_t *hal, struct rtw_xmit_req *tx_req,
			u8 *wd_buf, u32 *wd_len)
{
	return rtw_hal_mac_ax_fill_txdesc(hal->mac, tx_req, wd_buf, wd_len);
}

enum rtw_hal_status
hal_usb_tx_agg_cfg_8852au(struct hal_info_t *hal, u8* wd_buf, u8 agg_num)
{
	return hal_mac_usb_tx_agg_cfg(hal, wd_buf, agg_num);
}

enum rtw_hal_status
hal_usb_rx_agg_cfg_8852au(struct hal_info_t *hal, u8 mode, u8 agg_mode,
	u8 drv_define, u8 timeout, u8 size, u8 pkt_num)
{
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	/*u8 drv_define, u8 timeout, u8 size, u8 pkt_num*/
	/*mode 0: disable*/
	/*mode 1: default (0x2005)*/
	/*mode 2: 0x0101*/
	switch (mode){
		case PHL_RX_AGG_DISABLE:
			hstatus = hal_mac_usb_rx_agg_cfg(hal, 0, 0, 0, 0, 0);
			break;
		case PHL_RX_AGG_DEFAULT:
			hstatus = hal_mac_usb_rx_agg_cfg(hal, MAC_AX_RX_AGG_MODE_USB,
				0, 0, 0, 0);
			break;
		case PHL_RX_AGG_SMALL_PKT:
			hstatus = hal_mac_usb_rx_agg_cfg(hal, MAC_AX_RX_AGG_MODE_USB,
				1, 0x01, 0x01, 0);
			break;
		case PHL_RX_AGG_USER_DEFINE:
			hstatus = hal_mac_usb_rx_agg_cfg(hal, agg_mode,
				drv_define, timeout, size, pkt_num);
			break;
		default:
			hstatus = RTW_HAL_STATUS_FAILURE;
			break;
	}
	return hstatus;
}

u8 hal_get_fwcmd_queue_idx_8852au(void)
{
	return FWCMD_QUEUE_IDX_8852A;
}

static void hal_cfg_dma_io_8852au(struct hal_info_t *hal, u8 en)
{
}

static void hal_cfg_txdma_8852au(struct hal_info_t *hal, u8 en, u8 dma_ch)
{
}

static void hal_cfg_wow_txdma_8852au(struct hal_info_t *hal, u8 en)
{
}

static void hal_cfg_txhci_8852au(struct hal_info_t *hal, u8 en)
{
}

static void hal_cfg_rxhci_8852au(struct hal_info_t *hal, u8 en)
{
}

static void hal_clr_rwptr_8852au(struct hal_info_t *hal)
{
}

static void hal_rst_bdram_8852au(struct hal_info_t *hal)
{
}

static u8 hal_poll_txdma_idle_8852au(struct hal_info_t *hal)
{
	return true;
}

static void hal_cfg_rsvd_ctrl_8852au(struct hal_info_t *hal)
{
}

#endif // if 0 NEO

static struct hal_trx_ops ops= {
#if 0 // NEO
	.init = hal_trx_init_8852au,
	.deinit = hal_trx_deinit_8852au,
	.map_hw_tx_chnl = hal_mapping_hw_tx_chnl_8852au,
	.get_bulkout_id = hal_get_bulkout_id_8852au,
	.hal_fill_wd = hal_fill_wd_8852au,
#endif 
	.handle_rx_buffer = hal_handle_rx_buffer_8822c,
	.query_hal_info = hal_query_info_8822cu,
#if 0 // NEO
	.usb_tx_agg_cfg = hal_usb_tx_agg_cfg_8852au,
	.usb_rx_agg_cfg = hal_usb_rx_agg_cfg_8852au,
	.get_fwcmd_queue_idx = hal_get_fwcmd_queue_idx_8852au,
	.get_max_bulkout_wd_num = hal_get_max_bulkout_wd_num_8852au,
	.cfg_dma_io = hal_cfg_dma_io_8852au,
	.cfg_txdma = hal_cfg_txdma_8852au,
	.cfg_wow_txdma = hal_cfg_wow_txdma_8852au,
	.cfg_txhci = hal_cfg_txhci_8852au,
	.cfg_rxhci = hal_cfg_rxhci_8852au,
	.clr_rwptr = hal_clr_rwptr_8852au,
	.rst_bdram = hal_rst_bdram_8852au,
	.poll_txdma_idle = hal_poll_txdma_idle_8852au,
	.cfg_rsvd_ctrl = hal_cfg_rsvd_ctrl_8852au,
#endif
};


u32 hal_hook_trx_ops_8822cu(struct hal_info_t *hal_info)
{
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;

	if (NULL != hal_info) {
		hal_info->trx_ops = &ops;
		hstatus = RTW_HAL_STATUS_SUCCESS;
	}

	return hstatus;
}
