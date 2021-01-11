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
#define _PHL_INIT_C_
#include "phl_headers.h"



void _phl_com_init_rssi_stat(struct rtw_phl_com_t *phl_com)
{
	u8 i = 0, j = 0;
	for (i = 0; i < RTW_RSSI_TYPE_MAX; i++) {
		phl_com->rssi_stat.ma_rssi_ele_idx[i] = 0;
		phl_com->rssi_stat.ma_rssi_ele_cnt[i] = 0;
		phl_com->rssi_stat.ma_rssi_ele_sum[i] = 0;
		phl_com->rssi_stat.ma_rssi[i] = 0;
		for (j = 0; j < PHL_RSSI_MAVG_NUM; j++)
			phl_com->rssi_stat.ma_rssi_ele[i][j] = 0;
	}
	_os_spinlock_init(phl_com->drv_priv, &(phl_com->rssi_stat.lock));
}

void _phl_com_deinit_rssi_stat(struct rtw_phl_com_t *phl_com)
{
	_os_spinlock_free(phl_com->drv_priv, &(phl_com->rssi_stat.lock));
}

/**
 * rtw_phl_init_ppdu_sts_para(...)
 * Description:
 * 	1. Do not call this api after rx started.
 * 	2. PPDU Status per PKT settings
 **/
void rtw_phl_init_ppdu_sts_para(struct rtw_phl_com_t *phl_com,
				bool en_psts_per_pkt, bool psts_ampdu,
				u8 rx_fltr)
{
#ifdef CONFIG_PHL_RX_PSTS_PER_PKT
	phl_com->ppdu_sts_info.en_psts_per_pkt = en_psts_per_pkt;
	phl_com->ppdu_sts_info.psts_ampdu = psts_ampdu;
	phl_com->ppdu_sts_info.ppdu_sts_filter = rx_fltr;
#else
	return;
#endif
}

void _phl_com_deinit_ppdu_sts(struct rtw_phl_com_t *phl_com)
{
#ifdef CONFIG_PHL_RX_PSTS_PER_PKT
	u8 i = 0;
	u8 j = 0;
	for (j = 0; j < HW_BAND_MAX; j++) {
		for (i = 0; i < PHL_MAX_PPDU_CNT; i++) {
			if (phl_com->ppdu_sts_info.sts_ent[j][i].frames.cnt != 0) {
				PHL_INFO("[Error] deinit_ppdu_sts : frame queue is not empty\n");
			}
			pq_deinit(phl_com->drv_priv,
				  &(phl_com->ppdu_sts_info.sts_ent[j][i].frames));
		}
	}
#else
	return;
#endif
}

void _phl_com_init_ppdu_sts(struct rtw_phl_com_t *phl_com)
{
#ifdef CONFIG_PHL_RX_PSTS_PER_PKT
	u8 i = 0;
#endif
	u8 j = 0;
	for (j = 0; j < HW_BAND_MAX; j++) {
		phl_com->ppdu_sts_info.cur_rx_ppdu_cnt[j] = 0xFF;
	}
#ifdef CONFIG_PHL_RX_PSTS_PER_PKT
	/* Default enable when compile flag is set. */
	phl_com->ppdu_sts_info.en_psts_per_pkt = true;
	/**
	 * Filter of buffer pkt for phy status:
	 *	if the correspond bit is set to 1,
	 *	the pkt will be buffer till ppdu sts or next ppdu is processed.
	 **/
	phl_com->ppdu_sts_info.ppdu_sts_filter =
			RTW_PHL_PSTS_FLTR_MGNT | RTW_PHL_PSTS_FLTR_CTRL |
			RTW_PHL_PSTS_FLTR_DATA | RTW_PHL_PSTS_FLTR_EXT_RSVD;

	/* if set to false, only the first mpdu in ppdu has phy status */
	phl_com->ppdu_sts_info.psts_ampdu = false;

	phl_com->ppdu_sts_info.en_fake_psts = false;

	for (j = 0; j < HW_BAND_MAX; j++) {
		for (i = 0; i < PHL_MAX_PPDU_CNT; i++) {
			pq_init(phl_com->drv_priv,
				&(phl_com->ppdu_sts_info.sts_ent[j][i].frames));
		}
	}
#endif
#ifdef CONFIG_PHY_INFO_NTFY
	phl_com->ppdu_sts_info.msg_aggr_cnt = 0;
#endif
}

static void phl_msg_entry(void* priv, struct phl_msg *msg)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)priv;
	u8 mdl_id = MSG_MDL_ID_FIELD(msg->msg_id);
	u16 evt_id = MSG_EVT_ID_FIELD(msg->msg_id);

	PHL_DBG("[PHL]%s, mdl_id(%d)\n", __FUNCTION__, mdl_id);

	/* dispatch received PHY msg here */
	switch(mdl_id) {
		case PHL_MDL_PHY_MGNT:
			phl_msg_hub_phy_mgnt_evt_hdlr(phl_info, evt_id);
			break;
		case PHL_MDL_RX:
			phl_msg_hub_rx_evt_hdlr(phl_info, evt_id, msg->inbuf, msg->inlen);
			break;
		case PHL_MDL_MRC:
			phl_msg_hub_mrc_evt_hdlr(phl_info, msg);
			break;
		case PHL_MDL_POWER_MGNT:
			phl_msg_hub_power_mgnt_evt_hdlr(phl_info, msg);
			break;
		case PHL_MDL_BTC:
			rtw_phl_btc_hub_msg_hdl(phl_info, msg);
			break;
		default:
			break;
	}
}

static enum rtw_phl_status phl_register_msg_entry(struct phl_info_t *phl_info)
{
	struct phl_msg_receiver ctx;
	void *d = phl_to_drvpriv(phl_info);
	u8 imr[] = {PHL_MDL_PHY_MGNT, PHL_MDL_RX, PHL_MDL_MRC, PHL_MDL_POWER_MGNT
			, PHL_MDL_BTC};
	_os_mem_set(d, &ctx, 0, sizeof(struct phl_msg_receiver));
	ctx.incoming_evt_notify = phl_msg_entry;
	ctx.priv = (void*)phl_info;
	if( phl_msg_hub_register_recver((void*)phl_info,
				&ctx, MSG_RECV_PHL) == RTW_PHL_STATUS_SUCCESS) {
		/* PHL layer module should set IMR for receiving
		desired PHY msg  and handle it in phl_phy_evt_entry*/
		phl_msg_hub_update_recver_mask((void*)phl_info, MSG_RECV_PHL,
						imr, sizeof(imr), false);
		return RTW_PHL_STATUS_SUCCESS;
	}
	else
		return RTW_PHL_STATUS_FAILURE;

}

static enum rtw_phl_status phl_deregister_msg_entry(
					struct phl_info_t *phl_info)
{
	return phl_msg_hub_deregister_recver((void*)phl_info, MSG_RECV_PHL);
}

static enum rtw_phl_status phl_fw_init(struct phl_info_t *phl_info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_RESOURCE;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct rtw_fw_info_t *fw_info = &phl_com->fw_info;

	FUNCIN_WSTS(phl_status);

	fw_info->rom_buff = _os_mem_alloc(phl_to_drvpriv(phl_info), RTW_MAX_FW_SIZE);

	if (!fw_info->rom_buff) {
		PHL_ERR("%s : rom buff allocate fail!!\n", __func__);
		goto mem_alloc_fail;
	}

	fw_info->ram_buff = _os_mem_alloc(phl_to_drvpriv(phl_info), RTW_MAX_FW_SIZE);

	if (!fw_info->ram_buff) {
		PHL_ERR("%s : ram buff allocate fail!!\n", __func__);
		goto mem_alloc_fail;
	}

	phl_status = RTW_PHL_STATUS_SUCCESS;

	FUNCOUT_WSTS(phl_status);

mem_alloc_fail:
	return phl_status;
}

static void phl_fw_deinit(struct phl_info_t *phl_info)
{
	struct rtw_fw_info_t *fw_info = &phl_info->phl_com->fw_info;

	if (fw_info->rom_buff)
		_os_mem_free(phl_to_drvpriv(phl_info), fw_info->rom_buff, RTW_MAX_FW_SIZE);
	if (fw_info->ram_buff)
		_os_mem_free(phl_to_drvpriv(phl_info), fw_info->ram_buff, RTW_MAX_FW_SIZE);
}
static enum rtw_phl_status
phl_register_background_module_entry(struct phl_info_t *phl_info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
#ifdef CONFIG_CMD_DISP
	/*
	 * setup struct phl_module_ops & call dispr_register_module
	 * to register background module instance.
	 * call dispr_deregister_module if you need to dynamically
	 * deregister the instance of background module.
	*/

	/* power manager / btc / Multi-Role Controller(MRC) belong to BK module */
	phl_status = phl_register_mrc_module(phl_info);
	if(phl_status != RTW_PHL_STATUS_SUCCESS)
		return phl_status;
	phl_status = phl_register_custom_module(phl_info, HW_BAND_0);
	if(phl_status != RTW_PHL_STATUS_SUCCESS)
		return phl_status;

	phl_status = phl_register_led_module(phl_info);
	if(phl_status != RTW_PHL_STATUS_SUCCESS)
		return phl_status;

	phl_status = phl_register_cmd_general(phl_info);
	if(phl_status != RTW_PHL_STATUS_SUCCESS)
		return phl_status;

#else
	phl_status = RTW_PHL_STATUS_SUCCESS;
#endif
	return phl_status;
}

static enum rtw_phl_status phl_com_init(void *drv_priv,
					struct phl_info_t *phl_info,
					struct rtw_ic_info *ic_info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;

	phl_info->phl_com = _os_mem_alloc(drv_priv,
						sizeof(struct rtw_phl_com_t));
	if (phl_info->phl_com == NULL) {
		phl_status = RTW_PHL_STATUS_RESOURCE;
		PHL_ERR("alloc phl_com failed\n");
		goto error_phl_com_mem;
	}

	phl_info->phl_com->phl_priv = phl_info;
	phl_info->phl_com->drv_priv = drv_priv;
	phl_info->phl_com->hci_type = ic_info->hci_type;
	phl_info->phl_com->edcca_mode = RTW_EDCCA_NORMAL;

	phl_sw_cap_init(phl_info->phl_com);

	_os_spinlock_init(drv_priv, &phl_info->phl_com->evt_info.evt_lock);

	phl_fw_init(phl_info);
	#ifdef CONFIG_PHL_CHANNEL_INFO
	phl_status = phl_chaninfo_init(phl_info);
	if (phl_status)
		goto error_phl_com_mem;
	#endif /* CONFIG_PHL_CHANNEL_INFO */

	_phl_com_init_rssi_stat(phl_info->phl_com);
	_phl_com_init_ppdu_sts(phl_info->phl_com);

	phl_status = RTW_PHL_STATUS_SUCCESS;
	return phl_status;

error_phl_com_mem:
	return phl_status;
}


static enum rtw_phl_status phl_hci_init(struct phl_info_t *phl_info,
									struct rtw_ic_info *ic_info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;

	phl_info->hci = _os_mem_alloc(phl_to_drvpriv(phl_info),
					sizeof(struct hci_info_t));
	if (phl_info->hci == NULL) {
		phl_status = RTW_PHL_STATUS_RESOURCE;
		goto error_hci_mem;
	}
#ifdef CONFIG_USB_HCI
	phl_info->hci->usb_bulkout_size = ic_info->usb_info.usb_bulkout_size;
#endif

	/* init variable of hci_info_t struct */

	phl_status = RTW_PHL_STATUS_SUCCESS;

error_hci_mem:
	return phl_status;
}

static void phl_com_deinit(struct phl_info_t *phl_info,
				struct rtw_phl_com_t *phl_com)
{
	void *drv_priv = phl_to_drvpriv(phl_info);

	/* deinit variable or stop mechanism. */
	if (phl_com) {
		phl_sw_cap_deinit(phl_info->phl_com);
		_os_spinlock_free(drv_priv, &phl_com->evt_info.evt_lock);
		_phl_com_deinit_rssi_stat(phl_info->phl_com);
		_phl_com_deinit_ppdu_sts(phl_info->phl_com);
		phl_fw_deinit(phl_info);
		#ifdef CONFIG_PHL_CHANNEL_INFO
		phl_chaninfo_deinit(phl_info);
		#endif /* CONFIG_PHL_CHANNEL_INFO */
		_os_mem_free(drv_priv, phl_com, sizeof(struct rtw_phl_com_t));
	}
}

static void phl_hci_deinit(struct phl_info_t *phl_info, struct hci_info_t *hci)
{

	/* deinit variable or stop mechanism. */
	if (hci)
		_os_mem_free(phl_to_drvpriv(phl_info), hci,
						sizeof(struct hci_info_t));
}

static enum rtw_phl_status _phl_hci_ops_check(struct phl_info_t *phl_info)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	struct phl_hci_trx_ops *trx_ops = phl_info->hci_trx_ops;

	if (!trx_ops->hci_trx_init) {
		phl_ops_error_msg("hci_trx_init");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->hci_trx_deinit) {
		phl_ops_error_msg("hci_trx_deinit");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->prepare_tx) {
		phl_ops_error_msg("prepare_tx");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->recycle_rx_buf) {
		phl_ops_error_msg("recycle_rx_buf");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->tx) {
		phl_ops_error_msg("tx");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->rx) {
		phl_ops_error_msg("rx");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->trx_cfg) {
		phl_ops_error_msg("trx_cfg");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->pltfm_tx) {
		phl_ops_error_msg("pltfm_tx");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->alloc_h2c_pkt_buf) {
		phl_ops_error_msg("alloc_h2c_pkt_buf");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->free_h2c_pkt_buf) {
		phl_ops_error_msg("free_h2c_pkt_buf");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->trx_reset) {
		phl_ops_error_msg("trx_reset");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->trx_resume) {
		phl_ops_error_msg("trx_resume");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->req_tx_stop) {
		phl_ops_error_msg("req_tx_stop");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->req_rx_stop) {
		phl_ops_error_msg("req_rx_stop");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->is_tx_pause) {
		phl_ops_error_msg("is_tx_pause");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->is_rx_pause) {
		phl_ops_error_msg("is_rx_pause");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->get_txbd_buf) {
		phl_ops_error_msg("get_txbd_buf");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->get_rxbd_buf) {
		phl_ops_error_msg("get_rxbd_buf");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->recycle_rx_pkt) {
		phl_ops_error_msg("recycle_rx_pkt");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->register_trx_hdlr) {
		phl_ops_error_msg("register_trx_hdlr");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->rx_handle_normal) {
		phl_ops_error_msg("rx_handle_normal");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->tx_watchdog) {
		phl_ops_error_msg("tx_watchdog");
		status = RTW_PHL_STATUS_FAILURE;
	}

#ifdef CONFIG_PCI_HCI
	if (!trx_ops->recycle_busy_wd) {
		phl_ops_error_msg("recycle_busy_wd");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->recycle_busy_h2c) {
		phl_ops_error_msg("recycle_busy_h2c");
		status = RTW_PHL_STATUS_FAILURE;
	}
#endif

#ifdef CONFIG_USB_HCI
	if (!trx_ops->pend_rxbuf) {
		phl_ops_error_msg("pend_rxbuf");
		status = RTW_PHL_STATUS_FAILURE;
	}
	if (!trx_ops->recycle_tx_buf) {
		phl_ops_error_msg("recycle_tx_buf");
		status = RTW_PHL_STATUS_FAILURE;
	}
#endif

	return status;
}

static enum rtw_phl_status phl_set_hci_ops(struct phl_info_t *phl_info)
{
	#ifdef CONFIG_PCI_HCI
	if (phl_get_hci_type(phl_info->phl_com) == RTW_HCI_PCIE)
		phl_hook_trx_ops_pci(phl_info);
	#endif

	#ifdef CONFIG_USB_HCI
	if (phl_get_hci_type(phl_info->phl_com) == RTW_HCI_USB)
		phl_hook_trx_ops_usb(phl_info);
	#endif

	#ifdef CONFIG_SDIO_HCI
	if (phl_get_hci_type(phl_info->phl_com) == RTW_HCI_SDIO)
		phl_hook_trx_ops_sdio(phl_info);
	#endif

	return _phl_hci_ops_check(phl_info);
}


static enum rtw_phl_status phl_cmd_init(struct phl_info_t *phl_info)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;

	if (phl_info->cmd_fsm != NULL)
		return RTW_PHL_STATUS_FAILURE;

	phl_info->cmd_fsm = phl_cmd_new_fsm(phl_info->fsm_root, phl_info);
	if (phl_info->cmd_fsm == NULL)
		return RTW_PHL_STATUS_FAILURE;

	if (phl_info->cmd_obj != NULL)
		goto obj_fail;

	phl_info->cmd_obj = phl_cmd_new_obj(phl_info->cmd_fsm, phl_info);
	if (phl_info->cmd_obj == NULL)
		goto obj_fail;

	return RTW_PHL_STATUS_SUCCESS;

obj_fail:
	phl_fsm_deinit_fsm(phl_info->cmd_fsm);
	phl_info->cmd_fsm = NULL;
	return RTW_PHL_STATUS_FAILURE;
}

static void phl_cmd_deinit(struct phl_info_t *phl_info)
{
	phl_cmd_destory_obj(phl_info->cmd_obj);
	phl_info->cmd_obj = NULL;
	phl_cmd_destory_fsm(phl_info->cmd_fsm);
	phl_info->cmd_fsm = NULL;
}

static enum rtw_phl_status phl_ser_init(struct phl_info_t *phl_info)
{
	if (phl_info->ser_fsm != NULL)
		return RTW_PHL_STATUS_FAILURE;

	phl_info->ser_fsm = phl_ser_new_fsm(phl_info->fsm_root, phl_info);
	if (phl_info->ser_fsm == NULL)
		return RTW_PHL_STATUS_FAILURE;

	if (phl_info->ser_obj != NULL)
		goto obj_fail;

	phl_info->ser_obj = phl_ser_new_obj(phl_info->ser_fsm, phl_info);
	if (phl_info->ser_obj == NULL)
		goto obj_fail;

	return RTW_PHL_STATUS_SUCCESS;

obj_fail:
	phl_ser_destory_fsm(phl_info->ser_fsm);
	phl_info->ser_fsm = NULL;
	return RTW_PHL_STATUS_FAILURE;
}

static void phl_ser_deinit(struct phl_info_t *phl_info)
{
	phl_ser_destory_obj(phl_info->ser_obj);
	phl_info->ser_obj = NULL;

	phl_ser_destory_fsm(phl_info->ser_fsm);
	phl_info->ser_fsm = NULL;
}

static enum rtw_phl_status phl_btc_init(struct phl_info_t *phl_info)
{
	if (phl_info->btc_fsm != NULL)
		return RTW_PHL_STATUS_FAILURE;

	phl_info->btc_fsm = phl_btc_new_fsm(phl_info->fsm_root, phl_info);
	if (phl_info->btc_fsm == NULL)
		return RTW_PHL_STATUS_FAILURE;

	phl_info->btc_obj = phl_btc_new_obj(phl_info->btc_fsm, phl_info);
	if (phl_info->btc_obj == NULL)
		goto obj_fail;

	return RTW_PHL_STATUS_SUCCESS;

obj_fail:
	phl_fsm_deinit_fsm(phl_info->btc_fsm);
	phl_info->btc_fsm = NULL;
	return RTW_PHL_STATUS_FAILURE;

}

static void phl_btc_deinit(struct phl_info_t *phl_info)
{
	phl_btc_destory_obj(phl_info->btc_obj);
	phl_info->btc_obj = NULL;

	phl_btc_destory_fsm(phl_info->btc_fsm);
	phl_info->btc_fsm = NULL;
}

static enum rtw_phl_status phl_scan_init(struct phl_info_t *phl_info)
{
	if (phl_info->scan_fsm != NULL)
		return RTW_PHL_STATUS_FAILURE;

	phl_info->scan_fsm = phl_scan_new_fsm(phl_info->fsm_root, phl_info);
	if (phl_info->scan_fsm == NULL)
		return RTW_PHL_STATUS_FAILURE;

	if (phl_info->scan_obj != NULL)
		goto obj_fail;

	phl_info->scan_obj = phl_scan_new_obj(phl_info->scan_fsm, phl_info);
	if (phl_info->scan_obj == NULL)
		goto obj_fail;

	return RTW_PHL_STATUS_SUCCESS;

obj_fail:
	phl_fsm_deinit_fsm(phl_info->scan_fsm);
	phl_info->scan_fsm = NULL;
	return RTW_PHL_STATUS_FAILURE;
}

static void phl_scan_deinit(struct phl_info_t *phl_info)
{
	phl_scan_destory_obj(phl_info->scan_obj);
	phl_info->scan_obj = NULL;
	phl_scan_destory_fsm(phl_info->scan_fsm);
	phl_info->scan_fsm = NULL;
}

#if 0 // NEO TODO 

static enum rtw_phl_status phl_sound_init(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;

	if (phl_info->snd_fsm!= NULL)
		return RTW_PHL_STATUS_FAILURE;

	phl_info->snd_fsm = phl_sound_new_fsm(phl_info->fsm_root, phl_info);
	if (phl_info->snd_fsm == NULL)
		return RTW_PHL_STATUS_FAILURE;

	pstatus = phl_snd_new_obj(phl_info->snd_fsm, phl_info);
	if (pstatus != RTW_PHL_STATUS_SUCCESS)
		goto obj_fail;

	return pstatus;

obj_fail:
	phl_fsm_deinit_fsm(phl_info->snd_fsm);
	phl_info->snd_fsm = NULL;
	return RTW_PHL_STATUS_FAILURE;
}

static void phl_sound_deinit(struct phl_info_t *phl_info)
{
	phl_snd_destory_obj(phl_info->snd_obj);
	phl_info->snd_obj = NULL;
	phl_snd_destory_fsm(phl_info->snd_fsm);
	phl_info->snd_fsm = NULL;
}

static enum rtw_phl_status phl_fsm_init(struct phl_info_t *phl_info)
{
	if (phl_info->fsm_root != NULL)
		return RTW_PHL_STATUS_FAILURE;

	/* allocate memory for fsm to do version control */
	phl_info->fsm_root = phl_fsm_init_root(phl_info);
	if (phl_info->fsm_root == NULL)
		return RTW_PHL_STATUS_FAILURE;

	return RTW_PHL_STATUS_SUCCESS;
}

static enum rtw_phl_status phl_test_modeule_init(struct phl_info_t *phl_info)
{
	if (rtw_test_module_alloc(phl_info))
		return RTW_PHL_STATUS_SUCCESS;
	return RTW_PHL_STATUS_FAILURE;
}

static void phl_test_modeule_deinit(struct phl_info_t *phl_info)
{
	rtw_test_module_free(phl_info->phl_com);
}

static enum rtw_phl_status phl_test_modeule_start(struct phl_info_t *phl_info)
{
	if (rtw_test_module_init(phl_info->phl_com))
		return RTW_PHL_STATUS_SUCCESS;
	return RTW_PHL_STATUS_FAILURE;
}

static void phl_test_modeule_stop(struct phl_info_t *phl_info)
{
	rtw_test_module_deinit(phl_info->phl_com);
}

static void phl_fsm_deinit(struct phl_info_t *phl_info)
{
	/* free memory for fsm */
	phl_fsm_deinit_root(phl_info->fsm_root);
	phl_info->fsm_root = NULL;
}

static enum rtw_phl_status phl_fsm_module_init(struct phl_info_t *phl_info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_SUCCESS;

	phl_status = phl_cmd_init(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_cmd_init failed\n");
		goto cmd_fail;
	}

	phl_status = phl_ser_init(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_ser_init failed\n");
		goto ser_fail;
	}

	phl_status = phl_btc_init(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_btc_init failed\n");
		goto btc_fail;
	}

	phl_status = phl_scan_init(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_scan_init failed\n");
		goto scan_fail;
	}

	phl_status = phl_sound_init(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_sound_init failed\n");
		goto sound_fail;
	}

	phl_status = phl_ps_init(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_ps_init failed\n");
		goto ps_fail;
	}

	return phl_status;

ps_fail:
	phl_sound_deinit(phl_info);
sound_fail:
	phl_scan_deinit(phl_info);
scan_fail:
	phl_btc_deinit(phl_info);
btc_fail:
	phl_ser_deinit(phl_info);
ser_fail:
	phl_cmd_deinit(phl_info);
cmd_fail:
	return phl_status;
}

static void phl_fsm_module_deinit(struct phl_info_t *phl_info)
{
	phl_sound_deinit(phl_info);
	phl_scan_deinit(phl_info);
	phl_btc_deinit(phl_info);
	phl_ser_deinit(phl_info);
	phl_cmd_deinit(phl_info);
	phl_ps_deinit(phl_info);
}

static enum rtw_phl_status phl_module_init(struct phl_info_t *phl_info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_SUCCESS;

	phl_status = phl_msg_hub_init(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_msg_hub_init failed\n");
		goto msg_hub_fail;
	}

	phl_status = phl_wow_mdl_init(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_wow_mdl_init failed\n");
		goto wow_init_fail;
	}

	phl_status = phl_pkt_ofld_init(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_pkt_ofld_init failed\n");
		goto pkt_ofld_init_fail;
	}

	phl_status = phl_test_modeule_init(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_test_modeule_init failed\n");
		goto error_test_module_init;
	}

	phl_status = phl_disp_eng_init(phl_info, HW_BAND_MAX);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_disp_eng_init failed\n");
		goto error_disp_eng_init;
	}

	phl_status = phl_register_background_module_entry(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_register_disp_eng_module_entry failed\n");
		goto error_disp_eng_reg_init;
	}

	return phl_status;

error_disp_eng_reg_init:
	phl_disp_eng_deinit(phl_info);
error_disp_eng_init:
	phl_test_modeule_deinit(phl_info);
error_test_module_init:
	phl_pkt_ofld_deinit(phl_info);
pkt_ofld_init_fail:
	phl_wow_mdl_deinit(phl_info);
wow_init_fail:
	phl_msg_hub_deinit(phl_info);
msg_hub_fail:
	return phl_status;
}

static void phl_module_deinit(struct phl_info_t *phl_info)
{
	phl_disp_eng_deinit(phl_info);
	phl_test_modeule_deinit(phl_info);
	phl_pkt_ofld_deinit(phl_info);
	phl_wow_mdl_deinit(phl_info);
	phl_msg_hub_deinit(phl_info);
}

static enum rtw_phl_status phl_fsm_start(struct phl_info_t *phl_info)
{
	return phl_fsm_start_root(phl_info->fsm_root);
}

static enum rtw_phl_status phl_fsm_stop(struct phl_info_t *phl_info)
{
	return phl_fsm_stop_root(phl_info->fsm_root);
}

static enum rtw_phl_status phl_fsm_module_start(struct phl_info_t *phl_info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_SUCCESS;

	phl_status = phl_fsm_start_fsm(phl_info->ser_fsm);
	if (phl_status != RTW_PHL_STATUS_SUCCESS)
		goto ser_fail;

	phl_status = phl_btc_start(phl_info->btc_obj);
	if (phl_status != RTW_PHL_STATUS_SUCCESS)
		goto btc_fail;

	phl_status = phl_fsm_start_fsm(phl_info->scan_fsm);
	if (phl_status != RTW_PHL_STATUS_SUCCESS)
		goto scan_fail;

	phl_status = phl_ps_fsm_start(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS)
		goto ps_fail;

	phl_status = phl_cmd_start(phl_info->cmd_obj);
	if (phl_status != RTW_PHL_STATUS_SUCCESS)
		goto cmd_fail;

	phl_status = phl_fsm_start_fsm(phl_info->snd_fsm);
	if (phl_status != RTW_PHL_STATUS_SUCCESS)
		goto snd_fail;

	return phl_status;

snd_fail:
	phl_fsm_stop_fsm(phl_info->cmd_fsm);
	phl_ps_fsm_stop(phl_info);
ps_fail:
	phl_fsm_stop_fsm(phl_info->scan_fsm);
scan_fail:
	phl_fsm_stop_fsm(phl_info->btc_fsm);
btc_fail:
	phl_fsm_stop_fsm(phl_info->ser_fsm);
ser_fail:
	phl_fsm_cmd_stop(phl_info);
cmd_fail:
	return phl_status;
}

static enum rtw_phl_status phl_fsm_module_stop(struct phl_info_t *phl_info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_SUCCESS;

	phl_fsm_stop_fsm(phl_info->snd_fsm);
	phl_fsm_stop_fsm(phl_info->scan_fsm);
	phl_fsm_stop_fsm(phl_info->btc_fsm);
	phl_fsm_stop_fsm(phl_info->ser_fsm);
	phl_fsm_cmd_stop(phl_info);
	phl_ps_fsm_stop(phl_info);

	return phl_status;
}

static enum rtw_phl_status phl_module_start(struct phl_info_t *phl_info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_SUCCESS;

	phl_status = phl_test_modeule_start(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_test_modeule_start failed\n");
		goto error_test_mdl_start;
	}

	phl_status = phl_disp_eng_start(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_disp_eng_start failed\n");
		goto error_disp_eng_start;
	}

	if(phl_info->msg_hub) {
		phl_msg_hub_start(phl_info);
		phl_register_msg_entry(phl_info);
	}

	return phl_status;

error_disp_eng_start:
	phl_test_modeule_stop(phl_info);
error_test_mdl_start:
	return phl_status;
}

static enum rtw_phl_status phl_module_stop(struct phl_info_t *phl_info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_SUCCESS;

	phl_disp_eng_stop(phl_info);
	phl_test_modeule_stop(phl_info);

	if(phl_info->msg_hub) {
		phl_deregister_msg_entry(phl_info);
		phl_msg_hub_stop(phl_info);
	}

	return phl_status;
}

#endif // if 0 NEO

static enum rtw_phl_status phl_var_init(struct phl_info_t *phl_info)
{
	return RTW_PHL_STATUS_SUCCESS;
}

static void phl_var_deinit(struct phl_info_t *phl_info)
{

}

struct rtw_phl_com_t *rtw_phl_get_com(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	return phl_info->phl_com;
}

static void phl_regulation_init(void *drv_priv, void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_regulation *rg = NULL;

	if (!drv_priv || !phl)
		return;

	rg = &phl_info->regulation;

	_os_spinlock_init(drv_priv, &rg->lock);
	rg->init = 1;
	rg->domain.code = INVALID_DOMAIN_CODE;
}

static void phl_regulation_deinit(void *drv_priv, void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_regulation *rg = NULL;

	if (!drv_priv || !phl)
		return;

	rg = &phl_info->regulation;

	_os_spinlock_free(drv_priv, &rg->lock);
}

enum rtw_phl_status rtw_phl_init(void *drv_priv, void **phl,
					struct rtw_ic_info *ic_info)
{
	struct phl_info_t *phl_info = NULL;
	struct rtw_phl_com_t *phl_com = NULL;
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

	FUNCIN();
	phl_info = _os_mem_alloc(drv_priv, sizeof(struct phl_info_t));
	if (phl_info == NULL) {
		phl_status = RTW_PHL_STATUS_RESOURCE;
		PHL_ERR("alloc phl_info failed\n");
		goto error_phl_mem;
	}
	_os_mem_set(drv_priv, phl_info, 0, sizeof(struct phl_info_t));
	*phl = phl_info;

	phl_regulation_init(drv_priv, phl_info);

	phl_status = phl_com_init(drv_priv, phl_info, ic_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		phl_status = RTW_PHL_STATUS_RESOURCE;
		PHL_ERR("alloc phl_com failed\n");
		goto error_phl_com_mem;
	}

	phl_status = phl_hci_init(phl_info, ic_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_hci_init failed\n");
		goto error_hci_init;
	}

	phl_status = phl_set_hci_ops(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_set_hci_ops failed\n");
		goto error_set_hci_ops;
	}

#if 0 // NEO TODO
	phl_status = phl_fsm_init(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_fsm_init failed\n");
		goto error_fsm_init;
	}

	/* init FSM modules */
	phl_status = phl_fsm_module_init(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_fsm_module_init failed\n");
		goto error_fsm_module_init;
	}

	hal_status = rtw_hal_init(drv_priv, phl_info->phl_com,
					&(phl_info->hal), ic_info->ic_id);
	if ((hal_status != RTW_HAL_STATUS_SUCCESS) || (phl_info->hal == NULL)) {
		phl_status = RTW_PHL_STATUS_HAL_INIT_FAILURE;
		PHL_ERR("rtw_hal_init failed status(%d),phl_info->hal(%p)\n",
			hal_status, phl_info->hal);
		goto error_hal_init;
	}

	/*send bus info to hal*/
	rtw_hal_hci_cfg(phl_info->phl_com, phl_info->hal, ic_info);

	/*get hw capability from mac/bb/rf/btc/efuse/fw-defeature-rpt*/
	hal_status = rtw_hal_read_chip_info(phl_info->phl_com, phl_info->hal);
	if (hal_status != RTW_HAL_STATUS_SUCCESS) {
		phl_status = RTW_PHL_STATUS_HAL_INIT_FAILURE;
		PHL_ERR("rtw_hal_read_chip_info failed\n");
		goto error_hal_read_chip_info;
	}

	hal_status = rtw_hal_var_init(phl_info->phl_com, phl_info->hal);
	if (hal_status != RTW_HAL_STATUS_SUCCESS) {
		phl_status = RTW_PHL_STATUS_HAL_INIT_FAILURE;
		PHL_ERR("rtw_hal_var_init failed\n");
		goto error_hal_var_init;
	}

	phl_status = phl_var_init(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_var_init failed\n");
		goto error_phl_var_init;
	}

	/* init mr_ctrl, wifi_role[] */
	phl_status = phl_mr_ctrl_init(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_mr_ctrl_init failed\n");
		goto error_wifi_role_ctrl_init;
	}

	/* init modules */
	phl_status = phl_module_init(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_module_init failed\n");
		goto error_module_init;
	}

	/* init led_ctrl */
	phl_status = phl_led_ctrl_init(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_led_ctrl_init failed\n");
		goto error_led_ctrl_init;
	}

	/* init macid_ctrl , stainfo_ctrl*/
	/* init after get hw cap - macid number*/
	phl_status = phl_macid_ctrl_init(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_macid_ctrl_init failed\n");
		goto error_macid_ctrl_init;
	}

	/*init after hal_init - hal_sta_info*/
	phl_status = phl_stainfo_ctrl_init(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_stainfo_ctrl_init failed\n");
		goto error_stainfo_ctrl_init;
	}
	FUNCOUT();
#endif // if 0 NEO
	return phl_status;

#if 0 // NEO TODO
error_stainfo_ctrl_init:
	phl_macid_ctrl_deinit(phl_info);
error_macid_ctrl_init:
	phl_led_ctrl_deinit(phl_info);
error_led_ctrl_init:
	phl_module_deinit(phl_info);
error_module_init:
	phl_mr_ctrl_deinit(phl_info);
error_wifi_role_ctrl_init:
	phl_var_deinit(phl_info);
error_phl_var_init:
error_hal_var_init:
error_hal_read_chip_info:
	rtw_hal_deinit(phl_info->phl_com, phl_info->hal);


error_hal_init:
	phl_fsm_module_deinit(phl_info);
error_fsm_module_init:
	phl_fsm_deinit(phl_info);
error_fsm_init:
	/* Do nothing */

#endif // if 0

error_set_hci_ops:
	phl_hci_deinit(phl_info, phl_info->hci);
error_hci_init:
	phl_com_deinit(phl_info, phl_info->phl_com);
error_phl_com_mem:
	if (phl_info) {
		phl_regulation_deinit(drv_priv, phl_info);
		_os_mem_free(drv_priv, phl_info, sizeof(struct phl_info_t));
		*phl = phl_info = NULL;
	}

error_phl_mem:
	return phl_status;
}

void rtw_phl_deinit(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = phl_to_drvpriv(phl_info);

	if (phl_info) {
#if 0 // NEO TODO
		phl_stainfo_ctrl_deinie(phl_info);
		phl_macid_ctrl_deinit(phl_info);
		phl_led_ctrl_deinit(phl_info);
		/*deinit mr_ctrl, wifi_role[]*/
		phl_module_deinit(phl_info);
		phl_mr_ctrl_deinit(phl_info);
		rtw_hal_deinit(phl_info->phl_com, phl_info->hal);
		phl_var_deinit(phl_info);
		phl_fsm_module_deinit(phl_info);
		phl_fsm_deinit(phl_info);
#endif // if 0
		phl_hci_deinit(phl_info, phl_info->hci);
		phl_com_deinit(phl_info, phl_info->phl_com);
		phl_regulation_deinit(drv_priv, phl_info);
		_os_mem_free(drv_priv, phl_info,
					sizeof(struct phl_info_t));
	}
}

#if 0 // NEO : TODO : mark off first

enum rtw_phl_status
rtw_phl_trx_alloc(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;

	phl_status = phl_datapath_init(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS) {
		PHL_ERR("phl_datapath_init failed\n");
		goto error_datapath;
	}
	phl_trx_test_init(phl);
	return phl_status;

error_datapath:
	phl_datapath_deinit(phl_info);
	return phl_status;
}

void
rtw_phl_trx_free(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	phl_datapath_deinit(phl_info);
	phl_trx_test_deinit(phl);
}


bool rtw_phl_is_init_completed(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	return rtw_hal_is_inited(phl_info->phl_com, phl_info->hal);
}

#ifdef RTW_PHL_BCN

enum rtw_phl_status
rtw_phl_add_beacon(void *phl, struct rtw_bcn_info_cmn *bcn_cmn)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	void *hal = phl_info->hal;
	struct rtw_bcn_entry *bcn_entry = NULL;

	if(rtw_hal_add_beacon(phl_com, hal, bcn_cmn) == RTW_HAL_STATUS_SUCCESS)
		return RTW_PHL_STATUS_SUCCESS;
	else
		return RTW_PHL_STATUS_FAILURE;
}

enum rtw_phl_status rtw_phl_update_beacon(void *phl, u8 bcn_id)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	void *hal = phl_info->hal;
	struct rtw_bcn_entry *bcn_entry = NULL;

	if(rtw_hal_update_beacon(phl_com, hal, bcn_id) == RTW_HAL_STATUS_SUCCESS)
		return RTW_PHL_STATUS_SUCCESS;
	else
		return RTW_PHL_STATUS_FAILURE;
}

enum rtw_phl_status rtw_phl_free_bcn_entry(void *phl, struct rtw_wifi_role_t *wrole)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct rtw_bcn_info_cmn *bcn_cmn = &wrole->bcn_cmn;
	void *hal = phl_info->hal;
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_SUCCESS;

	if (bcn_cmn->bcn_added == 1) {
		if (rtw_hal_free_beacon(phl_com, hal, bcn_cmn->bcn_id) == RTW_HAL_STATUS_SUCCESS) {
			bcn_cmn->bcn_added = 0;
			phl_status = RTW_PHL_STATUS_SUCCESS;
		} else {
			phl_status = RTW_PHL_STATUS_FAILURE;
		}
	}

	return phl_status;
}

#endif

void rtw_phl_cap_pre_config(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	/* FW Pre-config */
	rtw_hal_fw_cap_pre_config(phl_info->phl_com,phl_info->hal);
	/* Bus Pre-config */
	rtw_hal_bus_cap_pre_config(phl_info->phl_com,phl_info->hal);
}

enum rtw_phl_status rtw_phl_preload(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_SUCCESS;

#ifdef RTW_WKARD_PRELOAD_TRX_RESET
	struct phl_hci_trx_ops *ops = phl_info->hci_trx_ops;
#endif
	FUNCIN();

	hal_status = rtw_hal_preload(phl_info->phl_com, phl_info->hal);

#ifdef RTW_WKARD_PRELOAD_TRX_RESET
	ops->trx_reset(phl_info);
#endif
	if (hal_status != RTW_HAL_STATUS_SUCCESS)
		return RTW_PHL_STATUS_FAILURE;

	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status rtw_phl_start(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_SUCCESS;

	hal_status = rtw_hal_start(phl_info->phl_com, phl_info->hal);
	if (hal_status == RTW_HAL_STATUS_MAC_INIT_FAILURE) {
		phl_status = RTW_PHL_STATUS_HAL_INIT_FAILURE;
		goto error_hal_start;
	} else if (hal_status == RTW_HAL_STATUS_BB_INIT_FAILURE) {
		phl_status = RTW_PHL_STATUS_HAL_INIT_FAILURE;
		goto error_hal_start;
	} else if (hal_status == RTW_HAL_STATUS_RF_INIT_FAILURE) {
		phl_status = RTW_PHL_STATUS_HAL_INIT_FAILURE;
		goto error_hal_start;
	} else if (hal_status == RTW_HAL_STATUS_BTC_INIT_FAILURE) {
		phl_status = RTW_PHL_STATUS_HAL_INIT_FAILURE;
		goto error_hal_start;
	} else if (hal_status != RTW_HAL_STATUS_SUCCESS) {
		phl_status = RTW_PHL_STATUS_HAL_INIT_FAILURE;
		goto error_hal_start;
	}

	/* start FSM framework */
	phl_status = phl_fsm_start(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS)
		goto error_phl_fsm_start;

	/* start FSM modules */
	phl_status = phl_fsm_module_start(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS)
		goto error_phl_fsm_module_start;

	/* start modules */
	phl_status = phl_module_start(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS)
		goto error_phl_module_start;

	phl_status = phl_datapath_start(phl_info);
	if (phl_status != RTW_PHL_STATUS_SUCCESS)
		goto error_phl_datapath_start;

	rtw_hal_enable_interrupt(phl_info->phl_com, phl_info->hal);

	phl_status = RTW_PHL_STATUS_SUCCESS;

	return phl_status;

error_phl_datapath_start:
	phl_module_stop(phl_info);
error_phl_module_start:
	phl_fsm_module_stop(phl_info);
error_phl_fsm_module_start:
	phl_fsm_stop(phl_info);
error_phl_fsm_start:
	rtw_hal_stop(phl_info->phl_com, phl_info->hal);
error_hal_start:
	return phl_status;
}

void rtw_phl_stop(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	rtw_hal_disable_interrupt(phl_info->phl_com, phl_info->hal);
	phl_module_stop(phl_info);
#ifdef DBG_PHL_MR
	phl_mr_info_dbg(phl_info);
#endif
	phl_fsm_module_stop(phl_info);
	phl_fsm_stop(phl_info);
	rtw_hal_stop(phl_info->phl_com, phl_info->hal);
	phl_datapath_stop(phl_info);
}

enum rtw_phl_status phl_wow_start(struct phl_info_t *phl_info, struct rtw_phl_stainfo_t *sta)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
#ifdef CONFIG_WOWLAN
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	enum mlme_state s_mstat = sta->wrole->mstate;
	struct phl_hci_trx_ops *trx_ops = phl_info->hci_trx_ops;
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	u8 nlo_exist = phl_wow_nlo_exist(phl_info);

	wow_info->enter_wow = false;
	wow_info->func_en = false;

	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] %s enter with sta state(%d)\n.", __func__, s_mstat);

	do {
		/* stop all active features */
		pstatus = phl_module_stop(phl_info);
		if (RTW_PHL_STATUS_SUCCESS != pstatus) {
			PHL_ERR("[wow] phl_module_stop failed.\n");
			break;
		}

		pstatus = phl_fsm_module_stop(phl_info);
		if (RTW_PHL_STATUS_SUCCESS != pstatus) {
			PHL_ERR("[wow] phl_fsm_module_stop failed.\n");
			break;
		}

		pstatus = phl_fsm_stop(phl_info);
		if (RTW_PHL_STATUS_SUCCESS != pstatus) {
			PHL_ERR("[wow] phl_fsm_stop failed.\n");
			break;
		}

		pstatus = phl_wow_init_precfg(phl_info);
		if (RTW_PHL_STATUS_SUCCESS != pstatus) {
			PHL_ERR("[wow] phl_wow_init_precfg failed.\n");
			break;
		}

		/* check mlme state */
		if (s_mstat == MLME_NO_LINK && !nlo_exist) {
			/* currently do nothing */
		} else if (s_mstat == MLME_LINKED || nlo_exist) {

			hstatus = rtw_hal_wow_start(phl_info->phl_com, phl_info->hal, sta);
			if (RTW_HAL_STATUS_SUCCESS != hstatus) {
				pstatus = RTW_PHL_STATUS_FAILURE;
				break;
			}

			pstatus = phl_wow_func_en(phl_info, sta);
			if (RTW_PHL_STATUS_SUCCESS != pstatus)
				break;

			wow_info->func_en = true;

		} else {
			pstatus = RTW_PHL_STATUS_FAILURE;
			break;
		}

		/* power saving */
		phl_wow_pwr_cfg(phl_info, sta, true);

		pstatus = phl_wow_init_postcfg(phl_info);
		if (RTW_PHL_STATUS_SUCCESS != pstatus) {
			PHL_ERR("[wow] phl_wow_init_postcfg failed!!!\n");
			break;
		}

		wow_info->enter_wow = true;

		pstatus = RTW_PHL_STATUS_SUCCESS;
	} while (0);


	if (RTW_PHL_STATUS_SUCCESS != pstatus) {
		PHL_ERR("[wow] %s : error entering wowlan!\n", __func__);
		rtw_hal_stop(phl_info->phl_com, phl_info->hal);
		phl_datapath_stop(phl_info);
		rtw_hal_disable_interrupt(phl_info->phl_com, phl_info->hal);
		wow_info->enter_wow = false;
	} else {
		PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_,
			"[wow] %s success, set enter_wow with func enabled(%d).\n", __func__, wow_info->func_en);
	}

	return pstatus;
#else

	return RTW_PHL_STATUS_SUCCESS;
#endif /* CONFIG_WOWLAN */
}

void phl_wow_stop(struct phl_info_t *phl_info, struct rtw_phl_stainfo_t *sta)
{
#ifdef CONFIG_WOWLAN
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	enum mlme_state s_mstat = sta->wrole->mstate;
	u8 nlo_exist = phl_wow_nlo_exist(phl_info);
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);

	/* leave power saving */
	/* phl_wow_pwr_cfg(phl_info, sta, false); */

	phl_module_start(phl_info);

	phl_wow_deinit_precfg(phl_info);

	if (wow_info->func_en == false) {
		/* currently do nothing */
	} else if (wow_info->func_en == true) {

		phl_wow_func_dis(phl_info, sta);

		hstatus = rtw_hal_wow_stop(phl_info->phl_com, phl_info->hal, sta);
		if (hstatus)
			PHL_ERR("%s : wowlan stop fail!\n", __func__);
	}

	phl_wow_deinit_postcfg(phl_info);

	phl_fsm_start(phl_info);
	phl_fsm_module_start(phl_info);

#endif /* CONFIG_WOWLAN */
}

enum rtw_phl_status rtw_phl_radio_on(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_hci_trx_ops *ops = phl_info->hci_trx_ops;
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_SUCCESS;

	ops->trx_resume(phl_info, PHL_REQ_PAUSE_TX|PHL_REQ_PAUSE_RX);

	rtw_hal_set_default_var(phl_info->hal);

	hal_status = rtw_hal_start(phl_info->phl_com, phl_info->hal);
	if (hal_status == RTW_HAL_STATUS_MAC_INIT_FAILURE) {
		phl_status = RTW_PHL_STATUS_HAL_INIT_FAILURE;
		goto error_hal_start;
	} else if (hal_status == RTW_HAL_STATUS_BB_INIT_FAILURE) {
		phl_status = RTW_PHL_STATUS_HAL_INIT_FAILURE;
		goto error_hal_start;
	} else if (hal_status == RTW_HAL_STATUS_RF_INIT_FAILURE) {
		phl_status = RTW_PHL_STATUS_HAL_INIT_FAILURE;
		goto error_hal_start;
	} else if (hal_status == RTW_HAL_STATUS_BTC_INIT_FAILURE) {
		phl_status = RTW_PHL_STATUS_HAL_INIT_FAILURE;
		goto error_hal_start;
	}

	phl_role_recover(phl);
	rtw_hal_enable_interrupt(phl_info->phl_com, phl_info->hal);

	return RTW_PHL_STATUS_SUCCESS;
error_hal_start:
	PHL_ERR("error_hal_start\n");
	return phl_status;
}

enum rtw_phl_status rtw_phl_radio_off(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_hci_trx_ops *ops = phl_info->hci_trx_ops;

	rtw_hal_disable_interrupt(phl_info->phl_com, phl_info->hal);
	phl_role_suspend(phl_info);
	rtw_hal_stop(phl_info->phl_com, phl_info->hal);
	ops->trx_reset(phl_info);

	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status rtw_phl_suspend(void *phl, struct rtw_phl_stainfo_t *sta, u8 wow_en)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;

	FUNCIN();

	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "%s enter with wow_en(%d)\n.", __func__, wow_en);


	/* Leave Power Saving */
#ifdef CONFIG_WOWLAN
	if (wow_en) {
		pstatus = phl_wow_start(phl_info, sta);
	} else {
		phl_role_suspend(phl_info);
		rtw_phl_stop(phl);
	}
#else
	phl_role_suspend(phl_info);
	rtw_phl_stop(phl);
#endif

	FUNCOUT_WSTS(pstatus);

	return pstatus;
}

enum rtw_phl_status rtw_phl_resume(void *phl, struct rtw_phl_stainfo_t *sta, u8 *hw_reinit)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	struct phl_hci_trx_ops *trx_ops = phl_info->hci_trx_ops;
#ifdef CONFIG_WOWLAN
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
#endif

	FUNCIN();

#ifdef CONFIG_WOWLAN
	rtw_hal_get_pwr_state(phl_info->hal, &wow_info->mac_pwr);

	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "%s enter with mac power %s\n.",
			  __func__, (wow_info->mac_pwr == 1) ? "on" : "off");

	if (wow_info->mac_pwr && wow_info->enter_wow) {
		phl_wow_stop(phl_info, sta);
		*hw_reinit = false;
	} else {
		pstatus = rtw_phl_start(phl);
		if (wow_info->enter_wow) {
			PHL_WARN("enter suspend with wow enabled but mac is power down\n");
			phl_role_suspend(phl_info);
		}
		phl_role_recover(phl_info);
		*hw_reinit = true;
	}
	#if defined(RTW_WKARD_WOW_L2_PWR) && defined(CONFIG_PCI_HCI)
	rtw_hal_set_l2_leave(phl_info->hal);
	#endif
	phl_record_wow_stat(phl_info);
	phl_reset_wow_info(phl_info);
#else
	pstatus = rtw_phl_start(phl);
	phl_role_recover(phl_info);
	*hw_reinit = true;
#endif

	FUNCOUT_WSTS(pstatus);

	return pstatus;
}

enum rtw_phl_status rtw_phl_reset(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_hci_trx_ops *ops = phl_info->hci_trx_ops;
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;

	if(rtw_phl_is_init_completed(phl_info))
		phl_status = RTW_PHL_STATUS_SUCCESS;

	rtw_hal_stop(phl_info->phl_com, phl_info->hal);

	ops->trx_reset(phl_info);
	ops->trx_resume(phl_info, PHL_REQ_PAUSE_TX|PHL_REQ_PAUSE_RX);

	rtw_hal_start(phl_info->phl_com, phl_info->hal);
	/* Leave power save */
	/* scan abort */
	/* STA disconnect/stop AP/Stop p2p function */

	return phl_status;
}

enum rtw_phl_status rtw_phl_restart(void *phl)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;

	phl_status = RTW_PHL_STATUS_SUCCESS;

	return phl_status;
}


/******************* IO  APIs *******************/
u8 rtw_phl_read8(void *phl, u32 addr)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	return rtw_hal_read8(phl_info->hal, addr);
}
u16 rtw_phl_read16(void *phl, u32 addr)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	return rtw_hal_read16(phl_info->hal, addr);
}
u32 rtw_phl_read32(void *phl, u32 addr)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	return rtw_hal_read32(phl_info->hal, addr);
}
void rtw_phl_write8(void *phl, u32 addr, u8 val)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	rtw_hal_write8(phl_info->hal, addr, val);
}
void rtw_phl_write16(void *phl, u32 addr, u16 val)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	rtw_hal_write16(phl_info->hal, addr, val);
}
void rtw_phl_write32(void *phl, u32 addr, u32 val)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	rtw_hal_write32(phl_info->hal, addr, val);
}
u32 rtw_phl_read_macreg(void *phl, u32 offset, u32 bit_mask)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	return rtw_hal_read_macreg(phl_info->hal, offset, bit_mask);
}
void rtw_phl_write_macreg(void *phl,
			u32 offset, u32 bit_mask, u32 data)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	rtw_hal_write_macreg(phl_info->hal, offset, bit_mask, data);

}
u32 rtw_phl_read_bbreg(void *phl, u32 offset, u32 bit_mask)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	return rtw_hal_read_bbreg(phl_info->hal, offset, bit_mask);
}
void rtw_phl_write_bbreg(void *phl,
			u32 offset, u32 bit_mask, u32 data)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	rtw_hal_write_bbreg(phl_info->hal, offset, bit_mask, data);

}
u32 rtw_phl_read_rfreg(void *phl,
			enum rf_path path, u32 offset, u32 bit_mask)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	return rtw_hal_read_rfreg(phl_info->hal, path, offset, bit_mask);
}
void rtw_phl_write_rfreg(void *phl,
			enum rf_path path, u32 offset, u32 bit_mask, u32 data)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	rtw_hal_write_rfreg(phl_info->hal, path, offset, bit_mask, data);

}

void rtw_phl_restore_interrupt(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	rtw_hal_restore_interrupt(phl_info->phl_com, phl_info->hal);
}

enum rtw_phl_status rtw_phl_interrupt_handler(void *phl)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_SUCCESS;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;
	u32 int_hdler_msk = 0x0;
#ifdef CONFIG_WIN_HANDLE_INTERRUPT
	struct rtw_phl_evt_ops *ops = &phl_info->phl_com->evt_ops;
#endif
	int_hdler_msk = rtw_hal_interrupt_handler(phl_info->hal);

	if (!int_hdler_msk) {
		PHL_WARN("%s : 0x%x\n", __func__, int_hdler_msk);
		phl_status = RTW_PHL_STATUS_FAILURE;
		goto end;
	}

	PHL_DBG("%s : 0x%x\n", __func__, int_hdler_msk);
	/* beacon interrupt */
	if (int_hdler_msk & BIT0)
		;/* todo */

	/* rx interrupt */
	if (int_hdler_msk & BIT1) {
		phl_status = rtw_phl_start_rx_process(phl);
#if defined(CONFIG_PCI_HCI) && !defined(CONFIG_DYNAMIC_RX_BUF)
		/* phl_status = hci_trx_ops->recycle_busy_wd(phl); */
#endif
	}

	/* tx interrupt */
	if (int_hdler_msk & BIT2)
		;

	/* cmd interrupt */
	if (int_hdler_msk & BIT3)
		;/* todo */

	/* halt c2h interrupt */
	if (int_hdler_msk & BIT4)
		phl_status = phl_ser_event_notify(phl, NULL);

	/* halt c2h interrupt */
	if (int_hdler_msk & BIT5)
		phl_status = phl_fw_watchdog_timeout_notify(phl);

	/* halt c2h interrupt - send msg to SER FSM to check ser event */
	if (int_hdler_msk & BIT6)
		phl_status = phl_ser_event_check(phl);

	if (phl_status != RTW_PHL_STATUS_SUCCESS)
		PHL_INFO("rtw_phl_interrupt_handler fail !!\n");

	/* schedule tx process */
	phl_status = phl_schedule_handler(phl_info->phl_com, &phl_info->phl_tx_handler);
end:

#ifdef CONFIG_WIN_HANDLE_INTERRUPT
	ops->interrupt_restore(phl_to_drvpriv(phl_info), false);
#endif
	return phl_status;
}

void rtw_phl_enable_interrupt(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	rtw_hal_enable_interrupt(phl_info->phl_com, phl_info->hal);
}

void rtw_phl_disable_interrupt(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	rtw_hal_disable_interrupt(phl_info->phl_com, phl_info->hal);
}

bool rtw_phl_recognize_interrupt(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	return rtw_hal_recognize_interrupt(phl_info->hal);
}

void rtw_phl_clear_interrupt(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	rtw_hal_clear_interrupt(phl_info->hal);
}


u8 rtw_phl_SER_inprogress(void *phl)
{
	return phl_ser_inprogress(phl);
}

void rtw_phl_SER_clear_status(void *phl, u32 serstatus)
{
	 phl_ser_clear_status(phl, serstatus);
}

void rtw_phl_notify_watchdog_status(void *phl, bool inprogress)
{
	phl_ser_notify_from_upper_watchdog_status(phl, inprogress);
}

enum rtw_phl_status rtw_phl_msg_hub_register_recver(void* phl,
		struct phl_msg_receiver* ctx, enum phl_msg_recver_layer layer)
{
	return phl_msg_hub_register_recver(phl, ctx, layer);
}
enum rtw_phl_status rtw_phl_msg_hub_update_recver_mask(void* phl,
		enum phl_msg_recver_layer layer, u8* mdl_id, u32 len, u8 clr)
{
	return phl_msg_hub_update_recver_mask(phl, layer, mdl_id, len, clr);
}
enum rtw_phl_status rtw_phl_msg_hub_deregister_recver(void* phl,
					enum phl_msg_recver_layer layer)
{
	return phl_msg_hub_deregister_recver(phl, layer);
}
enum rtw_phl_status rtw_phl_msg_hub_send(void* phl,
			struct phl_msg_attribute* attr, struct phl_msg* msg)
{
	return phl_msg_hub_send((struct phl_info_t*)phl, attr, msg);
}
#ifdef PHL_PLATFORM_LINUX
void rtw_phl_mac_reg_dump(void *sel, void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	rtw_hal_mac_reg_dump(sel, phl_info->hal);
}

void rtw_phl_bb_reg_dump(void *sel, void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	rtw_hal_bb_reg_dump(sel, phl_info->hal);
}

void rtw_phl_bb_reg_dump_ex(void *sel, void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	rtw_hal_bb_reg_dump_ex(sel, phl_info->hal);
}

void rtw_phl_rf_reg_dump(void *sel, void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	rtw_hal_rf_reg_dump(sel, phl_info->hal);
}
#endif

/**
 * rtw_phl_get_sec_cam() - get the security cam raw data from HW
 * @phl:		struct phl_info_t *
 * @num:		How many cam you wnat to dump from the first one.
 * @buf:		ptr to buffer which store the content from HW.
 *			If buf is NULL, use console as debug path.
 * @size		Size of allocated memroy for @buf.
 *			The size should be @num * size of security cam offset(0x20).
 *
 * Return true when function successfully works, otherwise, return fail.
 */
bool rtw_phl_get_sec_cam(void *phl, u16 num, u8 *buf, u16 size)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_hal_status ret = RTW_HAL_STATUS_SUCCESS;

	ret = rtw_hal_get_sec_cam(phl_info->hal, num, buf, size);
	if (ret != RTW_HAL_STATUS_SUCCESS)
		return false;

	return true;
}

/**
 * rtw_phl_get_addr_cam() - get the address cam raw data from HW
 * @phl:		struct phl_info_t *
 * @num:		How many cam you wnat to dump from the first one.
 * @buf:		ptr to buffer which store the content from HW.
 *			If buf is NULL, use console as debug path.
 * @size		Size of allocated memroy for @buf.
 *			The size should be @num * size of Addr cam offset(0x40).
 *
 * Return true when function successfully works, otherwise, return fail.
 */
bool rtw_phl_get_addr_cam(void *phl, u16 num, u8 *buf, u16 size)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_hal_status ret = RTW_HAL_STATUS_SUCCESS;

	ret = rtw_hal_get_addr_cam(phl_info->hal, num, buf, size);
	if (ret != RTW_HAL_STATUS_SUCCESS)
		return false;

	return true;
}

void rtw_phl_mac_dbg_status_dump(void *phl, u32 *val, u8 *en)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	rtw_hal_dbg_status_dump(phl_info->hal, val, en);
}

void rtw_phl_watchdog_callback(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_com_t* phl_com = phl_info->phl_com;
	enum rtw_phl_status pstatus;

	phl_tx_watchdog(phl_info);
	phl_rx_watchdog(phl_info);
	phl_pcie_trx_mit_watchdog(phl_info);
	phl_mr_watchdog(phl_info);

	pstatus = rtw_phl_ps_notify(phl, PHL_PS_NTFY_WATCHDOG_START, NULL);
	if(pstatus == RTW_PHL_STATUS_SUCCESS) {
		rtw_hal_watchdog(phl_info->hal);
		pstatus = rtw_phl_ps_notify(phl, PHL_PS_NTFY_WATCHDOG_STOP, NULL);
	} else {
		PHL_TRACE(COMP_PHL_PS, _PHL_DEBUG_,
			"[PM] Bypass hal watchdog by power (%d) !!\n", pstatus);
	}
}
enum rtw_phl_status rtw_phl_force_usb_switch(void *phl, u32 speed)
{
#ifdef CONFIG_USB_HCI
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	if(speed <= RTW_USB_SPEED_HIGH)
		rtw_hal_force_usb_switch(phl_info->hal, USB_2_0);
	else if(speed < RTW_USB_SPEED_MAX)
		rtw_hal_force_usb_switch(phl_info->hal, USB_3_0);
	PHL_INFO("rtw_phl_force_usb_switch (%d) !!\n",speed);
#endif
	return RTW_PHL_STATUS_SUCCESS;
}
enum rtw_phl_status rtw_phl_get_cur_usb_speed(void *phl, u32 *speed)
{
#ifdef CONFIG_USB_HCI
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	u32 mode = rtwl_hal_get_cur_usb_mode(phl_info->hal);
	struct hci_info_t *hci = phl_info->hci;

	switch(mode) {
		case USB_1_1:
			*speed = RTW_USB_SPEED_FULL;
			hci->usb_bulkout_size = USB_FULL_SPEED_BULK_SIZE;
			break;
		case USB_2_0:
			*speed = RTW_USB_SPEED_HIGH;
			hci->usb_bulkout_size = USB_HIGH_SPEED_BULK_SIZE;
			break;
		case USB_3_0:
			*speed = RTW_USB_SPEED_SUPER;
			hci->usb_bulkout_size = USB_SUPER_SPEED_BULK_SIZE;
			break;
		default:
			*speed = RTW_USB_SPEED_UNKNOWN;
			break;
	}
	PHL_PRINT("rtw_phl_get_cur_usb_speed (%d) bulk size(%d) !!\n", *speed,
					phl_info->hci->usb_bulkout_size);
#endif
	return RTW_PHL_STATUS_SUCCESS;
}
enum rtw_phl_status
rtw_phl_get_usb_support_ability(void *phl, u32 *ability)
{
#ifdef CONFIG_USB_HCI
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	*ability = rtwl_hal_get_usb_support_ability(phl_info->hal);
#else
	*ability = 0;
#endif
	/* refer enum rtw_usb_sw_ability for definition of ability */
	PHL_INFO("rtw_phl_get_usb_support_ability (%d) !!\n",*ability);
	return RTW_PHL_STATUS_SUCCESS;
}

#endif // if 0 NEO

enum rtw_phl_status rtw_phl_get_mac_addr_efuse(void* phl, u8 *addr)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_SUCCESS;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *d = phl_to_drvpriv(phl_info);
	u8 addr_efuse[MAC_ADDRESS_LENGTH] = {0};

	hstatus = rtw_hal_get_efuse_info(phl_info->hal,
			EFUSE_INFO_MAC_ADDR,
			(void *)addr_efuse,
			MAC_ADDRESS_LENGTH);
	if (is_broadcast_mac_addr(addr_efuse)) {
		PHL_INFO("[WARNING] MAC Address from EFUSE is FF:FF:FF:FF:FF:FF\n");
		hstatus = RTW_HAL_STATUS_FAILURE;
	}
	if (RTW_HAL_STATUS_SUCCESS != hstatus) {
		pstatus = RTW_PHL_STATUS_FAILURE;
	} else {
		_os_mem_cpy(d, addr, addr_efuse, MAC_ADDRESS_LENGTH);
		PHL_INFO("%s : 0x%2x - 0x%2x - 0x%2x - 0x%2x - 0x%2x - 0x%2x\n",
			 __func__, addr[0], addr[1], addr[2],
			 addr[3], addr[4], addr[5]);

	}
	return pstatus;
}

#if 0 // NEO : TODO : mark off first

enum rtw_phl_status
rtw_phl_cfg_trx_path(void* phl, enum rf_path tx, u8 tx_nss,
		     enum rf_path rx, u8 rx_nss)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_SUCCESS;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	hstatus = rtw_hal_cfg_trx_path(phl_info->hal, tx, tx_nss, rx, rx_nss);

	if (RTW_HAL_STATUS_SUCCESS != hstatus)
		pstatus = RTW_PHL_STATUS_FAILURE;

	return pstatus;
}


void rtw_phl_reset_stat_ma_rssi(struct rtw_phl_com_t *phl_com)
{
	u8 i = 0, j = 0;
	PHL_INFO("--> %s\n", __func__);
	do{
		if (NULL == phl_com)
			break;

		_os_spinlock(phl_com->drv_priv,
			     &(phl_com->rssi_stat.lock), _bh, NULL);
		for (i = 0; i < RTW_RSSI_TYPE_MAX; i++) {
			phl_com->rssi_stat.ma_rssi_ele_idx[i] = 0;
			phl_com->rssi_stat.ma_rssi_ele_cnt[i] = 0;
			phl_com->rssi_stat.ma_rssi_ele_sum[i] = 0;
			phl_com->rssi_stat.ma_rssi[i] = 0;
			for (j = 0; j < PHL_RSSI_MAVG_NUM; j++)
				phl_com->rssi_stat.ma_rssi_ele[i][j] = 0;
		}
		_os_spinunlock(phl_com->drv_priv,
			       &(phl_com->rssi_stat.lock), _bh, NULL);
	} while (0);

	PHL_INFO("<-- %s\n", __func__);
}

u8
rtw_phl_get_ma_rssi(struct rtw_phl_com_t *phl_com,
		    enum rtw_rssi_type rssi_type)
{

	u8 ret = 0;
	if (NULL == phl_com)
		return ret;

	_os_spinlock(phl_com->drv_priv,
		     &(phl_com->rssi_stat.lock), _bh, NULL);
	ret = phl_com->rssi_stat.ma_rssi[rssi_type];
	_os_spinunlock(phl_com->drv_priv,
		       &(phl_com->rssi_stat.lock), _bh, NULL);

	return ret;
}

#ifdef RTW_WKARD_DYNAMIC_BFEE_CAP
enum rtw_phl_status
rtw_phl_bfee_ctrl(void *phl, struct rtw_wifi_role_t *wrole, bool ctrl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	if (RTW_HAL_STATUS_SUCCESS !=
	    rtw_hal_bf_bfee_ctrl(phl_info->hal, wrole->hw_band, ctrl)) {
		pstatus = RTW_PHL_STATUS_FAILURE;
	}
	return pstatus;
}
#endif

enum rtw_phl_status
rtw_phl_beacon_stop(void *phl, struct rtw_wifi_role_t *wrole, u8 stop)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_SUCCESS;

	hstatus = rtw_hal_beacon_stop(phl_info->hal, wrole, stop);
	if (hstatus != RTW_HAL_STATUS_SUCCESS)
		pstatus = RTW_PHL_STATUS_FAILURE;

	return pstatus;
}

#endif
