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
#define _HAL_RX_C_
#include "hal_headers.h"

void rtw_hal_cfg_rxhci(void *hal, u8 en)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct hal_trx_ops *trx_ops = hal_info->trx_ops;

	PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "%s : enable %d.\n", __func__, en);

	trx_ops->cfg_rxhci(hal_info, en);
}

enum rtw_hal_status rtw_hal_set_rxfltr_by_mode(void *hal,
			u8 band, enum rtw_rx_fltr_mode mode)
{
	RTW_ERR("%s TODO NEO\n", __func__);
	return RTW_HAL_STATUS_FAILURE;
#if 0 // NEO TODO
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct rtw_hal_com_t *hal_com = hal_info->hal_com;
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;


	hstatus = rtw_hal_mac_set_rxfltr_by_mode(hal_com, band, mode);

	if (hstatus == RTW_HAL_STATUS_SUCCESS)
		hal_info->rx_fltr_mode = mode;

	return hstatus;
#endif // if 0 NEO
}


enum rtw_rx_fltr_mode rtw_hal_get_rxfltr_mode(void *hal, u8 band)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;


	return hal_info->rx_fltr_mode;
}

enum rtw_hal_status rtw_hal_scan_set_rxfltr_by_mode(void *hinfo,
	enum phl_phy_idx phy_idx, bool off_channel, u8 *mode)
{
	RTW_ERR("%s NEO TOOD\n", __func__);
	return RTW_HAL_STATUS_FAILURE;
#if 0
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_SUCCESS;

	if (off_channel) {
		/* backup rx filter mode */
		*mode = rtw_hal_get_rxfltr_mode(hinfo, phy_idx);
		hal_status = rtw_hal_set_rxfltr_by_mode(hinfo,
			phy_idx, RX_FLTR_MODE_SCAN);
	} else {
		/* restore rx filter mode */
		hal_status = rtw_hal_set_rxfltr_by_mode(hinfo,
			phy_idx, *mode);
	}
	return hal_status;
#endif
}

#if 0 // NEO TODO

enum rtw_hal_status rtw_hal_acpt_crc_err_pkt(void *hal, u8 band, u8 enable)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct rtw_hal_com_t *hal_com = hal_info->hal_com;


	return rtw_hal_mac_set_rxfltr_acpt_crc_err(hal_com, band, enable);
}

enum rtw_hal_status rtw_hal_set_rxfltr_mpdu_size(void *hal, u8 band, u16 size)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct rtw_hal_com_t *hal_com = hal_info->hal_com;


	return rtw_hal_mac_set_rxfltr_mpdu_size(hal_com, band, size);
}
enum rtw_hal_status rtw_hal_set_rxfltr_by_type(void *hal, u8 band, u8 type, u8 target)
{

	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	enum rtw_hal_status hstats = RTW_HAL_STATUS_FAILURE;

	hstats = rtw_hal_mac_set_rxfltr_by_type(hal_info->hal_com, band, type, target);

	if (RTW_HAL_STATUS_SUCCESS != hstats)
		PHL_ERR("%s : type %u status %u target %u.band %u \n", __func__, type, hstats, target, band);


	return hstats;
}

#ifdef CONFIG_PCI_HCI
/**
 * rtw_hal_rx_res_query - query current HW rx resource with specifc dma channel
 * @hal: see struct hal_info_t
 * @dma_ch: the target dma channel
 * @host_idx: current host index of this channel
 * @hw_idx: current hw index of this channel
 *
 * this function returns the number of available tx resource
 * NOTE, input host_idx and hw_idx ptr shall NOT be NULL
 */
u16 rtw_hal_rx_res_query(void *hal, u8 dma_ch, u16 *host_idx, u16 *hw_idx)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct hal_trx_ops *trx_ops = hal_info->trx_ops;
	u16 res_num = 0;

	res_num = trx_ops->query_rx_res(hal_info->hal_com, dma_ch, host_idx,
					hw_idx);

	return res_num;
}


/**
 * rtw_hal_query_rxch_num - query total hw rx dma channels number
 *
 * returns the number of  hw rx dma channel
 */
u8 rtw_hal_query_rxch_num(void *hal)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct hal_trx_ops *trx_ops = hal_info->trx_ops;
	u8 ch_num = 0;

	ch_num = trx_ops->query_rxch_num();

	return ch_num;
}
u8 rtw_hal_check_rxrdy(struct rtw_phl_com_t *phl_com, void* hal, u8 *rxbuf, u8 dma_ch)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct hal_trx_ops *trx_ops = hal_info->trx_ops;
	u8 res = 0;

	res = trx_ops->check_rxrdy(phl_com, rxbuf, dma_ch);

	return res;
}

u8 rtw_hal_handle_rxbd_info(void* hal, u8 *rxbuf, u16 *buf_size)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct hal_trx_ops *trx_ops = hal_info->trx_ops;
	u8 res = 0;

	res = trx_ops->handle_rxbd_info(hal_info, rxbuf, buf_size);

	return res;
}

enum rtw_hal_status
rtw_hal_update_rxbd(void *hal, struct rx_base_desc *rxbd,
					struct rtw_rx_buf *rxbuf)
{
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct hal_trx_ops *trx_ops = hal_info->trx_ops;

	hstatus = trx_ops->update_rxbd(hal_info, rxbd, rxbuf);

	return hstatus;
}


enum rtw_hal_status rtw_hal_notify_rxdone(void* hal,
				struct rx_base_desc *rxbd, u8 ch, u16 rxcnt)
{
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct hal_trx_ops *trx_ops = hal_info->trx_ops;

	hstatus = trx_ops->notify_rxdone(hal_info, rxbd, ch, rxcnt);

	return hstatus;
}

u16 rtw_hal_handle_wp_rpt(void *hal, u8 *rp, u16 len, u8 *sw_retry, u8 *dma_ch,
			  u16 *wp_seq, u8 *txsts)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct hal_trx_ops *trx_ops = hal_info->trx_ops;
	u16 rsize = 0;

	rsize = trx_ops->handle_wp_rpt(hal_info, rp, len, sw_retry, dma_ch,
				       wp_seq, txsts);
	return rsize;
}


void rtw_hal_query_freerun(struct hal_info_t *hal, u32 *freerun_l,
							    u32 *freerun_h)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct hal_trx_ops *trx_ops = hal_info->trx_ops;

	trx_ops->query_freerun(hal_info, freerun_l, freerun_h);

}


#endif /*CONFIG_PCI_HCI*/

#endif // if 0

#ifdef CONFIG_USB_HCI
enum rtw_hal_status
rtw_hal_query_info(void* hal, u8 info_id, void *value)
{
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct hal_trx_ops *trx_ops = hal_info->trx_ops;

	hstatus = trx_ops->query_hal_info(hal_info, info_id, value);

	return hstatus;
}

enum rtw_hal_status
	rtw_hal_usb_rx_agg_cfg(void *hal, u8 mode, u8 agg_mode,
	u8 drv_define, u8 timeout, u8 size, u8 pkt_num)
{
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_SUCCESS;
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct hal_trx_ops *trx_ops = hal_info->trx_ops;

	hstatus = trx_ops->usb_rx_agg_cfg(hal, mode, agg_mode,
		drv_define, timeout, size, pkt_num);

	return hstatus;
}
#endif

#if 0 // NEO : mark off first

enum rtw_hal_status
rtw_hal_handle_rx_buffer(struct rtw_phl_com_t *phl_com, void* hal,
				u8 *buf, u32 buf_size,
				struct rtw_phl_rx_pkt *rxpkt)
{
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	struct hal_trx_ops *trx_ops = hal_info->trx_ops;

	hstatus = trx_ops->handle_rx_buffer(phl_com, hal_info,
					      buf, buf_size, rxpkt);

	return hstatus;
}

#ifdef CONFIG_SDIO_HCI
void rtw_hal_sdio_rx_agg_cfg(void *hal, bool enable, u8 drv_define,
			     u8 timeout, u8 size, u8 pkt_num)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;


	rtw_hal_mac_sdio_rx_agg_cfg(hal_info->hal_com, enable, drv_define,
				    timeout, size, pkt_num);
}

int rtw_hal_sdio_rx(void *hal, struct rtw_rx_buf *rxbuf)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;


	return rtw_hal_mac_sdio_rx(hal_info->hal_com, rxbuf);
}

int rtw_hal_sdio_parse_rx(void *hal, struct rtw_rx_buf *rxbuf)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;


	return rtw_hal_mac_sdio_parse_rx(hal_info->hal_com, rxbuf);
}
#endif /* CONFIG_SDIO_HCI */

void
hal_rx_ppdu_sts_normal_data(struct rtw_phl_com_t *phl_com,
			    void *hdr,
			    struct rtw_r_meta_data *meta)
{
	struct rtw_phl_ppdu_sts_info *ppdu_info = NULL;
	enum phl_band_idx band = HW_BAND_0;

	do {
		if ((NULL == phl_com) || (NULL == meta))
			break;
		ppdu_info = &phl_com->ppdu_sts_info;
		band = (meta->bb_sel > 0) ? HW_BAND_1 : HW_BAND_0;
		if (ppdu_info->cur_rx_ppdu_cnt[band] == meta->ppdu_cnt)
			break;
		/* start of the PPDU */
		ppdu_info->sts_ent[band][meta->ppdu_cnt].addr_cam_vld = meta->addr_cam_vld;
		ppdu_info->sts_ent[band][meta->ppdu_cnt].frame_type = PHL_GET_80211_HDR_TYPE(hdr);
		ppdu_info->sts_ent[band][meta->ppdu_cnt].crc32 = meta->crc32;
		ppdu_info->sts_ent[band][meta->ppdu_cnt].rx_rate = meta->rx_rate;
		ppdu_info->sts_ent[band][meta->ppdu_cnt].ppdu_type = meta->ppdu_type;
		if((RTW_FRAME_TYPE_BEACON ==
		    ppdu_info->sts_ent[band][meta->ppdu_cnt].frame_type) ||
		   (RTW_FRAME_TYPE_PROBE_RESP ==
		    ppdu_info->sts_ent[band][meta->ppdu_cnt].frame_type)) {
			PHL_GET_80211_HDR_ADDRESS3(phl_com->drv_priv, hdr,
				ppdu_info->sts_ent[band][meta->ppdu_cnt].src_mac_addr);
		} else {
			_os_mem_cpy(phl_com->drv_priv,
				ppdu_info->sts_ent[band][meta->ppdu_cnt].src_mac_addr,
				meta->ta, MAC_ADDRESS_LENGTH);
		}
		ppdu_info->sts_ent[band][meta->ppdu_cnt].valid = false;
		ppdu_info->cur_rx_ppdu_cnt[band] = meta->ppdu_cnt;
		PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_,
				"Start of the PPDU : band %d ; ppdu_cnt %d ; frame_type %d ; addr_cam_vld %d ; size %d ; rate 0x%x ; crc32 %d\n",
				band,
				ppdu_info->cur_rx_ppdu_cnt[band],
				ppdu_info->sts_ent[band][meta->ppdu_cnt].frame_type,
				ppdu_info->sts_ent[band][meta->ppdu_cnt].addr_cam_vld,
				meta->pktlen,
				meta->rx_rate,
				meta->crc32);
	} while (false);

}

void
hal_rx_ppdu_sts(struct rtw_phl_com_t *phl_com,
		struct rtw_phl_rx_pkt *phl_rx,
		struct hal_ppdu_sts *ppdu_sts)
{
	struct rtw_phl_ppdu_sts_info *ppdu_info = NULL;
	struct rtw_phl_rssi_stat *rssi_stat = NULL;
	struct rtw_r_meta_data *meta = &(phl_rx->r.mdata);
	struct rtw_phl_ppdu_phy_info *phy_info = &(phl_rx->r.phy_info);
	u8 i = 0;
	enum phl_band_idx band = HW_BAND_0;
	struct rtw_phl_ppdu_sts_ent *sts_ent = NULL;

	if ((NULL == phl_com) || (NULL == meta) || (NULL == ppdu_sts))
		return;

	if (0 == phy_info->is_valid)
		return;

	ppdu_info = &phl_com->ppdu_sts_info;
	rssi_stat = &phl_com->rssi_stat;
	band = (meta->bb_sel > 0) ? HW_BAND_1 : HW_BAND_0;

	if (ppdu_info->cur_rx_ppdu_cnt[band] != meta->ppdu_cnt) {
		PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_,
			  "[WARNING] ppdu cnt mis-match (band %d ; cur : %d , rxmeta : %d)\n",
			  band,
			  ppdu_info->cur_rx_ppdu_cnt[band],
			  meta->ppdu_cnt);
	}
	sts_ent = &(ppdu_info->sts_ent[band][meta->ppdu_cnt]);

	if (meta->crc32 || sts_ent->crc32) {
		UPDATE_MA_RSSI(rssi_stat, RTW_RSSI_UNKNOWN,
			 phy_info->rssi);
		return;
	}
	if (sts_ent->rx_rate != meta->rx_rate) {
		PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_,
			  "[WARNING] PPDU STS rx rate mis-match\n");
		UPDATE_MA_RSSI(rssi_stat, RTW_RSSI_UNKNOWN,
			       phy_info->rssi);
		return;
	}
	if (sts_ent->ppdu_type != meta->ppdu_type) {
		PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_,
			  "[WARNING] PPDU STS ppdu_type mis-match\n");
		UPDATE_MA_RSSI(rssi_stat, RTW_RSSI_UNKNOWN,
			       phy_info->rssi);
		return;
	}
	if (sts_ent->valid == true) {
		PHL_TRACE(COMP_PHL_PSTS, _PHL_INFO_,
			  "[WARNING] PPDU STS is already updated, skip this ppdu status\n");
		return;
	}

	/* update ppdu_info entry */
	sts_ent->freerun_cnt = meta->freerun_cnt;
	_os_mem_cpy(phl_com->drv_priv,
		    &(sts_ent->phy_info),
		    phy_info, sizeof(struct rtw_phl_ppdu_phy_info));

	for (i = 0; i < ppdu_sts->usr_num; i++) {
		if (ppdu_sts->usr[i].vld) {
			sts_ent->sta[i].macid =
				ppdu_sts->usr[i].macid;
			sts_ent->sta[i].vld = 1;
		} else {
			sts_ent->sta[i].vld = 0;
		}
	}
	sts_ent->phl_done = false;
	sts_ent->valid = true;

	/* update rssi stat */
	_os_spinlock(phl_com->drv_priv, &rssi_stat->lock, _bh, NULL);
	switch (sts_ent->frame_type &
		(BIT(1) | BIT(0))) {
		case RTW_FRAME_TYPE_MGNT :
			if (sts_ent->addr_cam_vld) {
				UPDATE_MA_RSSI(rssi_stat,
					 (1 == meta->a1_match) ?
					  RTW_RSSI_MGNT_ACAM_A1M :
					  RTW_RSSI_MGNT_ACAM,
					 phy_info->rssi);
			} else {
				UPDATE_MA_RSSI(rssi_stat, RTW_RSSI_MGNT_OTHER,
					 phy_info->rssi);
			}
		break;
		case RTW_FRAME_TYPE_CTRL :
			if (sts_ent->addr_cam_vld) {
				UPDATE_MA_RSSI(rssi_stat,
					 (1 == meta->a1_match) ?
					  RTW_RSSI_CTRL_ACAM_A1M :
					  RTW_RSSI_CTRL_ACAM,
					 phy_info->rssi);
			} else {
				UPDATE_MA_RSSI(rssi_stat, RTW_RSSI_CTRL_OTHER,
					 phy_info->rssi);
			}
		break;
		case RTW_FRAME_TYPE_DATA :
			if (sts_ent->addr_cam_vld) {
				UPDATE_MA_RSSI(rssi_stat,
					 (1 == meta->a1_match) ?
					  RTW_RSSI_DATA_ACAM_A1M :
					  RTW_RSSI_DATA_ACAM,
					 phy_info->rssi);
			} else {
				UPDATE_MA_RSSI(rssi_stat, RTW_RSSI_DATA_OTHER,
					 phy_info->rssi);
			}
		break;
		default:
			UPDATE_MA_RSSI(rssi_stat, RTW_RSSI_UNKNOWN,
				       phy_info->rssi);
		break;
	}
	_os_spinunlock(phl_com->drv_priv, &rssi_stat->lock, _bh, NULL);
}

#endif // if 0 NEO
