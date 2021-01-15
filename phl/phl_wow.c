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
#define _PHL_WOW_C_
#include "phl_headers.h"

enum rtw_phl_status phl_wow_mdl_init(struct phl_info_t* phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
#ifdef CONFIG_WOWLAN
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct phl_wow_info *info;

	info = (struct phl_wow_info *)_os_mem_alloc(phl_to_drvpriv(phl_info), sizeof(struct phl_wow_info));
	if (info == NULL)
		return RTW_PHL_STATUS_RESOURCE;

	phl_com->wow_info = info;

	info->phl_info = phl_info;

	_os_spinlock_init(phl_to_drvpriv(phl_info), &info->wow_lock);
#endif /* CONFIG_WOWLAN */
	return pstatus;
}

void phl_wow_mdl_deinit(struct phl_info_t* phl_info)
{
#ifdef CONFIG_WOWLAN
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct phl_wow_info *info = phl_com->wow_info;
	_os_spinlock_free(phl_to_drvpriv(phl_info), &info->wow_lock);
	_os_mem_free(phl_to_drvpriv(phl_info), info, sizeof(struct phl_wow_info));
#endif /* CONFIG_WOWLAN */
}

#ifdef CONFIG_WOWLAN

/* TO-DO: Confirm the enum strcut of the algo */
u8 _phl_query_iv_len(u8 algo)
{
	u8 len = 0;

	switch(algo) {
	case RTW_ENC_WEP40:
		len = 4;
		break;
	case RTW_ENC_TKIP:
	case RTW_ENC_CCMP:
	case RTW_ENC_GCMP256:
		len = 8;
		break;
	default:
		len = 0;
		break;
	}

	return len;
}

u8 _phl_query_key_desc_ver(struct phl_info_t *phl_info, u8 algo)
{
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);

	u8 akm_type = wow_info->gtk_ofld_info.akmtype_byte3;

	if (algo == RTW_ENC_TKIP)
		return EAPOLKEY_KEYDESC_VER_1;

	if (akm_type == 1 || akm_type == 2) {
		return EAPOLKEY_KEYDESC_VER_2;
	} else if (akm_type > 2 && akm_type < 7) {
		return EAPOLKEY_KEYDESC_VER_3;
	} else {
		return 0;
	}
}

static void _phl_cfg_pkt_ofld_null_info(
	struct phl_info_t* phl_info,
	struct rtw_phl_stainfo_t *phl_sta,
	struct rtw_pkt_ofld_null_info *null_info)
{
	void *drv_priv = phl_to_drvpriv(phl_info);

	_os_mem_cpy(drv_priv, &(null_info->a1[0]), &(phl_sta->mac_addr[0]), MAC_ADDRESS_LENGTH);
	_os_mem_cpy(drv_priv, &(null_info->a2[0]), &(phl_sta->wrole->mac_addr[0]), MAC_ADDRESS_LENGTH);
	_os_mem_cpy(drv_priv, &(null_info->a3[0]), &(phl_sta->mac_addr[0]), MAC_ADDRESS_LENGTH);

}

static void _phl_cfg_pkt_ofld_arp_rsp_info(struct phl_info_t* phl_info, struct rtw_phl_stainfo_t *phl_sta,
						struct rtw_pkt_ofld_arp_rsp_info *arp_rsp_info)
{
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	u8 pairwise_algo = get_wow_pairwise_algo_type(wow_info);

	_os_mem_cpy(drv_priv, &(arp_rsp_info->a1[0]), &(phl_sta->mac_addr[0]), MAC_ADDRESS_LENGTH);
	_os_mem_cpy(drv_priv, &(arp_rsp_info->a2[0]), &(phl_sta->wrole->mac_addr[0]), MAC_ADDRESS_LENGTH);
	_os_mem_cpy(drv_priv, &(arp_rsp_info->a3[0]), &(phl_sta->mac_addr[0]), MAC_ADDRESS_LENGTH);
	_os_mem_cpy(drv_priv, &(arp_rsp_info->host_ipv4_addr[0]),
		&(wow_info->arp_ofld_info.arp_ofld_content.host_ipv4_addr[0]),
		IPV4_ADDRESS_LENGTH);
	_os_mem_cpy(drv_priv, &(arp_rsp_info->remote_ipv4_addr[0]),
		&(wow_info->arp_ofld_info.arp_ofld_content.remote_ipv4_addr[0]),
		IPV4_ADDRESS_LENGTH);

	arp_rsp_info->sec_hdr = _phl_query_iv_len(pairwise_algo);
}

static void _phl_cfg_pkt_ofld_na_info(struct phl_info_t* phl_info, struct rtw_phl_stainfo_t *phl_sta,
					struct rtw_pkt_ofld_na_info *na_info)
{
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	u8 pairwise_algo = get_wow_pairwise_algo_type(wow_info);

	_os_mem_cpy(drv_priv, &(na_info->a1[0]), &(phl_sta->mac_addr[0]), MAC_ADDRESS_LENGTH);
	_os_mem_cpy(drv_priv, &(na_info->a2[0]), &(phl_sta->wrole->mac_addr[0]), MAC_ADDRESS_LENGTH);
	_os_mem_cpy(drv_priv, &(na_info->a3[0]), &(phl_sta->mac_addr[0]), MAC_ADDRESS_LENGTH);

	na_info->sec_hdr = _phl_query_iv_len(pairwise_algo);

}

static void _phl_cfg_pkt_ofld_eapol_key_info(
	struct phl_info_t* phl_info,
	struct rtw_phl_stainfo_t *phl_sta,
	struct rtw_pkt_ofld_eapol_key_info *eapol_key_info)
{

	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	struct rtw_gtk_ofld_info *gtk_ofld_info = &wow_info->gtk_ofld_info;

	u8 pairwise_algo = get_wow_pairwise_algo_type(wow_info);

	_os_mem_cpy(drv_priv, &(eapol_key_info->a1[0]), &(phl_sta->mac_addr[0]),
		MAC_ADDRESS_LENGTH);

	_os_mem_cpy(drv_priv, &(eapol_key_info->a2[0]), &(phl_sta->wrole->mac_addr[0]),
			MAC_ADDRESS_LENGTH);

	_os_mem_cpy(drv_priv, &(eapol_key_info->a3[0]), &(phl_sta->mac_addr[0]),
			MAC_ADDRESS_LENGTH);

	eapol_key_info->sec_hdr = _phl_query_iv_len(pairwise_algo);
	eapol_key_info->key_desc_ver = _phl_query_key_desc_ver(phl_info, pairwise_algo);
	_os_mem_cpy(drv_priv, eapol_key_info->replay_cnt,
				gtk_ofld_info->gtk_ofld_content.replay_cnt, 8);
}

static void _phl_cfg_pkt_ofld_sa_query_info(
	struct phl_info_t* phl_info,
	struct rtw_phl_stainfo_t *phl_sta,
	struct rtw_pkt_ofld_sa_query_info *sa_query_info)
{
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	u8 pairwise_algo = get_wow_pairwise_algo_type(wow_info);

	_os_mem_cpy(drv_priv, &(sa_query_info->a1[0]), &(phl_sta->mac_addr[0]),
			MAC_ADDRESS_LENGTH);

	_os_mem_cpy(drv_priv, &(sa_query_info->a2[0]), &(phl_sta->wrole->mac_addr[0]),
			MAC_ADDRESS_LENGTH);

	_os_mem_cpy(drv_priv, &(sa_query_info->a3[0]), &(phl_sta->mac_addr[0]),
			MAC_ADDRESS_LENGTH);

	sa_query_info->sec_hdr = _phl_query_iv_len(pairwise_algo);
}

enum rtw_phl_status rtw_phl_cfg_keep_alive_info(void *phl, struct rtw_keep_alive_info *info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_SUCCESS;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	struct rtw_keep_alive_info *keep_alive_info = &wow_info->keep_alive_info;

	FUNCIN();

	keep_alive_info->keep_alive_en = info->keep_alive_en;
	keep_alive_info->keep_alive_period = info->keep_alive_period;

	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] keep_alive_en %d\n", keep_alive_info->keep_alive_en);
	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] keep_alive_period %d\n", keep_alive_info->keep_alive_period);

	return phl_status;
}

enum rtw_phl_status rtw_phl_cfg_disc_det_info(void *phl, struct rtw_disc_det_info *info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_SUCCESS;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	struct rtw_disc_det_info *disc_det_info = &wow_info->disc_det_info;

	FUNCIN();

	disc_det_info->disc_det_en = info->disc_det_en;
	disc_det_info->disc_wake_en = info->disc_wake_en;
	disc_det_info->try_pkt_count = info->try_pkt_count;
	disc_det_info->check_period = info->check_period;
	disc_det_info->cnt_bcn_lost_en = info->cnt_bcn_lost_en;
	disc_det_info->cnt_bcn_lost_limit = info->cnt_bcn_lost_limit;

	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] disc_det_en %d\n", disc_det_info->disc_det_en);
	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] disc_wake_en %d\n", disc_det_info->disc_wake_en);
	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] try_pkt_count %d\n", disc_det_info->try_pkt_count);
	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] check_period %d\n", disc_det_info->check_period);
	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] cnt_bcn_lost_en %d\n", disc_det_info->cnt_bcn_lost_en);
	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] cnt_bcn_lost_limit %d\n", disc_det_info->cnt_bcn_lost_limit);

	return phl_status;
}

enum rtw_phl_status rtw_phl_cfg_nlo_info(void *phl, struct rtw_nlo_info *info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_SUCCESS;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	struct rtw_nlo_info *nlo_info = &wow_info->nlo_info;

	nlo_info->nlo_en = info->nlo_en;

	return phl_status;
}

void rtw_phl_cfg_arp_ofld_info(void *phl, struct rtw_arp_ofld_info *info)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	struct rtw_arp_ofld_info *arp_ofld_info = &wow_info->arp_ofld_info;
	void *drv_priv = phl_to_drvpriv(phl_info);

	FUNCIN();


	arp_ofld_info->arp_en = info->arp_en;

	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] arp_en %u\n",
			arp_ofld_info->arp_en);

	/* If not enabled, the following actions are not necessary */
	if (false == arp_ofld_info->arp_en)
		return;

	arp_ofld_info->arp_action = info->arp_action;

	_os_mem_cpy(drv_priv,
		&(arp_ofld_info->arp_ofld_content.remote_ipv4_addr[0]),
		&(info->arp_ofld_content.remote_ipv4_addr[0]),
		IPV4_ADDRESS_LENGTH);

	_os_mem_cpy(drv_priv,
		&(arp_ofld_info->arp_ofld_content.host_ipv4_addr[0]),
		&(info->arp_ofld_content.host_ipv4_addr[0]),
		IPV4_ADDRESS_LENGTH);

	_os_mem_cpy(drv_priv,
		&(arp_ofld_info->arp_ofld_content.mac_addr[0]),
		&(info->arp_ofld_content.mac_addr[0]),
		MAC_ADDRESS_LENGTH);

	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] arp_action %u\n",
			arp_ofld_info->arp_action);

	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] arp_remote_ipv4  %u:%u:%u:%u\n",
			arp_ofld_info->arp_ofld_content.remote_ipv4_addr[0],
			arp_ofld_info->arp_ofld_content.remote_ipv4_addr[1],
			arp_ofld_info->arp_ofld_content.remote_ipv4_addr[2],
			arp_ofld_info->arp_ofld_content.remote_ipv4_addr[3]);

	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] arp_host_ipv4  %u:%u:%u:%u\n",
			arp_ofld_info->arp_ofld_content.host_ipv4_addr[0],
			arp_ofld_info->arp_ofld_content.host_ipv4_addr[1],
			arp_ofld_info->arp_ofld_content.host_ipv4_addr[2],
			arp_ofld_info->arp_ofld_content.host_ipv4_addr[3]);

	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] arp_mac_addr  %02x:%02x:%02x:%02x:%02x:%02x \n",
			arp_ofld_info->arp_ofld_content.mac_addr[0],
			arp_ofld_info->arp_ofld_content.mac_addr[1],
			arp_ofld_info->arp_ofld_content.mac_addr[2],
			arp_ofld_info->arp_ofld_content.mac_addr[3],
			arp_ofld_info->arp_ofld_content.mac_addr[4],
			arp_ofld_info->arp_ofld_content.mac_addr[5]);

}


void rtw_phl_cfg_ndp_ofld_info(void *phl, struct rtw_ndp_ofld_info *info)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	struct rtw_ndp_ofld_info *ndp_ofld_info = &wow_info->ndp_ofld_info;
	struct rtw_ndp_ofld_content *pcontent;
	void *drv_priv = phl_to_drvpriv(phl_info);
	u8 idx = 0;

	FUNCIN();

	ndp_ofld_info->ndp_en = info->ndp_en;

	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] ndp_en %u\n",
			ndp_ofld_info->ndp_en);

	/* If not enabled, the following actions are not necessary */
	if (false == ndp_ofld_info->ndp_en)
		return;

	for (idx = 0; idx < 2; idx++) {

		pcontent = &ndp_ofld_info->ndp_ofld_content[idx];
		pcontent->ndp_en = info->ndp_ofld_content[idx].ndp_en;

		pcontent->chk_remote_ip =
			info->ndp_ofld_content[idx].chk_remote_ip;
		pcontent->num_target_ip =
			info->ndp_ofld_content[idx].num_target_ip;

		_os_mem_cpy(drv_priv, &(pcontent->mac_addr[0]),
			&(info->ndp_ofld_content[idx].mac_addr[0]),
			MAC_ADDRESS_LENGTH);
		_os_mem_cpy(drv_priv, &(pcontent->remote_ipv6_addr[0]),
			&(info->ndp_ofld_content[idx].remote_ipv6_addr[0]),
			IPV6_ADDRESS_LENGTH);
		_os_mem_cpy(drv_priv, &(pcontent->target_ipv6_addr[0][0]),
			&(info->ndp_ofld_content[idx].target_ipv6_addr[0][0]),
			IPV6_ADDRESS_LENGTH*2);

		PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] ndp_chk_remote_ip %u\n",
			pcontent->chk_remote_ip);

		PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] ndp_num_target_ip %u\n",
			pcontent->num_target_ip);

		PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] ndp_mac_addr  %02x:%02x:%02x:%02x:%02x:%02x \n",
			pcontent->mac_addr[0],
			pcontent->mac_addr[1],
			pcontent->mac_addr[2],
			pcontent->mac_addr[3],
			pcontent->mac_addr[4],
			pcontent->mac_addr[5]);

		PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_,
			"[wow] ndp_remote_ipv6  %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n",
			pcontent->remote_ipv6_addr[0],
			pcontent->remote_ipv6_addr[1],
			pcontent->remote_ipv6_addr[2],
			pcontent->remote_ipv6_addr[3],
			pcontent->remote_ipv6_addr[4],
			pcontent->remote_ipv6_addr[5],
			pcontent->remote_ipv6_addr[6],
			pcontent->remote_ipv6_addr[7],
			pcontent->remote_ipv6_addr[8],
			pcontent->remote_ipv6_addr[9],
			pcontent->remote_ipv6_addr[10],
			pcontent->remote_ipv6_addr[11],
			pcontent->remote_ipv6_addr[12],
			pcontent->remote_ipv6_addr[13],
			pcontent->remote_ipv6_addr[14],
			pcontent->remote_ipv6_addr[15]);

		PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_,
			"[wow] ndp_target_ipv6_addr  %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n",
			pcontent->target_ipv6_addr[0][0],
			pcontent->target_ipv6_addr[0][1],
			pcontent->target_ipv6_addr[0][2],
			pcontent->target_ipv6_addr[0][3],
			pcontent->target_ipv6_addr[0][4],
			pcontent->target_ipv6_addr[0][5],
			pcontent->target_ipv6_addr[0][6],
			pcontent->target_ipv6_addr[0][7],
			pcontent->target_ipv6_addr[0][8],
			pcontent->target_ipv6_addr[0][9],
			pcontent->target_ipv6_addr[0][10],
			pcontent->target_ipv6_addr[0][11],
			pcontent->target_ipv6_addr[0][12],
			pcontent->target_ipv6_addr[0][13],
			pcontent->target_ipv6_addr[0][14],
			pcontent->target_ipv6_addr[0][15]);

		PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_,
			"[wow] ndp_target_ipv6_addr  %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n",
			pcontent->target_ipv6_addr[1][0],
			pcontent->target_ipv6_addr[1][1],
			pcontent->target_ipv6_addr[1][2],
			pcontent->target_ipv6_addr[1][3],
			pcontent->target_ipv6_addr[1][4],
			pcontent->target_ipv6_addr[1][5],
			pcontent->target_ipv6_addr[1][6],
			pcontent->target_ipv6_addr[1][7],
			pcontent->target_ipv6_addr[1][8],
			pcontent->target_ipv6_addr[1][9],
			pcontent->target_ipv6_addr[1][10],
			pcontent->target_ipv6_addr[1][11],
			pcontent->target_ipv6_addr[1][12],
			pcontent->target_ipv6_addr[1][13],
			pcontent->target_ipv6_addr[1][14],
			pcontent->target_ipv6_addr[1][15]);

	}

}

u8 _phl_query_free_cam_entry_idx(struct rtw_pattern_match_info *pattern_match_info)
{
	struct rtw_wowcam_upd_info *wowcam_info = pattern_match_info->wowcam_info;
	u8 i = 0;

	for (i = 0; i < MAX_WOW_CAM_NUM; ++i)
		if (wowcam_info[i].valid == 0)
			break;

	return i;
}

enum rtw_phl_status rtw_phl_remove_wow_ptrn_info(void *phl, u8 wowcam_id)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	struct rtw_pattern_match_info *pattern_match_info = &wow_info->pattern_match_info;
	struct rtw_wowcam_upd_info *wowcam_info = &(pattern_match_info->wowcam_info[wowcam_id]);

	if (wowcam_id < MAX_WOW_CAM_NUM) {
		wowcam_info->valid = 0;
		phl_status = RTW_PHL_STATUS_SUCCESS;
	} else {
		PHL_TRACE(COMP_PHL_WOW, _PHL_WARNING_, "[wow] %s(): Invalid wowcam id(%u), Fail.\n",
						__func__, wowcam_id);
		phl_status = RTW_PHL_STATUS_FAILURE;
	}

	return phl_status;
}

enum rtw_phl_status rtw_phl_add_wow_ptrn_info(void *phl, struct rtw_wowcam_upd_info *info, u8 *wowcam_id)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	struct rtw_pattern_match_info *pattern_match_info = &wow_info->pattern_match_info;
	struct rtw_wowcam_upd_info *wowcam_info = NULL;
	void *d = phl_to_drvpriv(phl_info);

	*wowcam_id = _phl_query_free_cam_entry_idx(pattern_match_info);

	if (*wowcam_id < MAX_WOW_CAM_NUM) {
		wowcam_info = &(pattern_match_info->wowcam_info[*wowcam_id]);

		_os_mem_set(d, wowcam_info, 0, sizeof(struct rtw_wowcam_upd_info));
		_os_mem_cpy(d, wowcam_info, info, sizeof(struct rtw_wowcam_upd_info));

		/* fill in phl */
		wowcam_info->wow_cam_idx = *wowcam_id;
		wowcam_info->rw = 1;
		wowcam_info->is_negative_pattern_match = 0;
		wowcam_info->skip_mac_hdr = 1;
		wowcam_info->valid = 1;

		phl_status = RTW_PHL_STATUS_SUCCESS;
	} else {
		PHL_TRACE(COMP_PHL_WOW, _PHL_WARNING_, "[wow] no free cam entry can be used.\n");
		phl_status = RTW_PHL_STATUS_RESOURCE;
	}

	return phl_status;
}

enum rtw_phl_status rtw_phl_cfg_gtk_ofld_info(void *phl, struct rtw_gtk_ofld_info *info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_SUCCESS;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	struct rtw_gtk_ofld_info *gtk_ofld_info = &wow_info->gtk_ofld_info;
	void *d = phl_to_drvpriv(phl_info);

	FUNCIN();

	if (info == NULL || gtk_ofld_info == NULL) {
		PHL_TRACE(COMP_PHL_WOW, _PHL_WARNING_, "[wow] %s(): some ptr is NULL\n", __func__);
		phl_status = RTW_PHL_STATUS_FAILURE;

	} else {
		_os_mem_set(d, gtk_ofld_info, 0, sizeof(struct rtw_gtk_ofld_info));

		PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] gtk_en(%u), continue to gtk_ofld.\n", info->gtk_en);

		if (info->gtk_en) {
			_os_mem_cpy(d, gtk_ofld_info, info, sizeof(struct rtw_gtk_ofld_info));

			PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] gtk_ofld_info:\n");
			PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] - gtk_en          = %u\n", gtk_ofld_info->gtk_en);
			PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] - ieee80211w_en   = %u\n", gtk_ofld_info->ieee80211w_en);
			PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] - pairwise_wakeup = %u\n", gtk_ofld_info->pairwise_wakeup);
			PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] - bip_sec_algo    = %u\n", gtk_ofld_info->bip_sec_algo);

			PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] gtk_ofld_content:\n");
			PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] - akmtype_byte3   = %u\n", gtk_ofld_info->akmtype_byte3);
			PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] - kck_len         = %u\n", gtk_ofld_info->gtk_ofld_content.kck_len);
			PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] - kek_len         = %u\n", gtk_ofld_info->gtk_ofld_content.kek_len);
			PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] - replay_cnt      = 0x%x%x\n",
						*((u32 *)(gtk_ofld_info->gtk_ofld_content.replay_cnt)+1),
						*((u32 *)(gtk_ofld_info->gtk_ofld_content.replay_cnt)));

			if(info->ieee80211w_en) {
				gtk_ofld_info->hw_11w_en = true;
				PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] - igtk_keyid      = 0x%x\n",
								*((u32 *)(gtk_ofld_info->gtk_ofld_content.igtk_keyid)));
				PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] - ipn             = 0x%x%x\n",
								*((u32 *)(gtk_ofld_info->gtk_ofld_content.ipn)+1),
								*((u32 *)(gtk_ofld_info->gtk_ofld_content.ipn)));
				PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] - igtk_len        = %u\n", gtk_ofld_info->gtk_ofld_content.igtk_len);
				PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] - psk_len         = %u\n", gtk_ofld_info->gtk_ofld_content.psk_len);
			}
		} else {
			PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] gtk_ofld_info:\n");
			PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] - gtk_en          = %u\n", gtk_ofld_info->gtk_en);
		}
	}

	FUNCOUT();

	return phl_status;
}

enum rtw_phl_status rtw_phl_cfg_realwow_info(void *phl, struct rtw_realwow_info *info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_SUCCESS;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	struct rtw_realwow_info *realwow_info = &wow_info->realwow_info;

	realwow_info->realwow_en = info->realwow_en;

	return phl_status;
}

enum rtw_phl_status rtw_phl_cfg_wow_wake(void *phl, struct rtw_wow_wake_info *info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_SUCCESS;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	void *d = phl_to_drvpriv(phl_info);

	struct rtw_wow_wake_info *wow_wake_info = &wow_info->wow_wake_info;

	FUNCIN();

	wow_wake_info->wow_en = info->wow_en;
	wow_wake_info->drop_all_pkt = info->drop_all_pkt;
	wow_wake_info->rx_parse_after_wake = info->rx_parse_after_wake;
	wow_wake_info->pairwise_sec_algo = info->pairwise_sec_algo;
	wow_wake_info->group_sec_algo = info->group_sec_algo;
	wow_wake_info->pattern_match_en = info->pattern_match_en;
	wow_wake_info->magic_pkt_en = info->magic_pkt_en;
	wow_wake_info->hw_unicast_en = info->hw_unicast_en;
	wow_wake_info->fw_unicast_en = info->fw_unicast_en;
	wow_wake_info->deauth_wakeup = info->deauth_wakeup;
	wow_wake_info->rekey_wakeup = info->rekey_wakeup;
	wow_wake_info->eap_wakeup = info->eap_wakeup;
	wow_wake_info->all_data_wakeup = info->all_data_wakeup;
	_os_mem_cpy(d, &wow_wake_info->remote_wake_ctrl_info,
		&info->remote_wake_ctrl_info, sizeof(struct rtw_remote_wake_ctrl_info));

	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] wow_en %d\n", wow_wake_info->wow_en);
	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] drop_all_pkt %d\n", wow_wake_info->drop_all_pkt);
	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] rx_parse_after_wake %d\n", wow_wake_info->rx_parse_after_wake);
	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] pairwise_sec_algo %d\n", wow_wake_info->pairwise_sec_algo);
	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] group_sec_algo %d\n", wow_wake_info->group_sec_algo);
	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] bip_sec_algo %d\n", wow_wake_info->bip_sec_algo);
	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] pattern_match_en %d\n", wow_wake_info->pattern_match_en);
	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] magic_pkt_en %d\n", wow_wake_info->magic_pkt_en);
	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] hw_unicast_en %d\n", wow_wake_info->hw_unicast_en);
	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] fw_unicast_en %d\n", wow_wake_info->fw_unicast_en);
	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] deauth_wakeup %d\n", wow_wake_info->deauth_wakeup);
	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] rekey_wakeup %d\n", wow_wake_info->rekey_wakeup);
	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] eap_wakeup %d\n", wow_wake_info->eap_wakeup);
	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] all_data_wakeup %d\n", wow_wake_info->all_data_wakeup);

	return phl_status;
}

void phl_record_wow_stat(struct phl_info_t *phl_info)
{
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	struct phl_wow_stat *wow_stat = &wow_info->wow_stat;

	/* init */
	wow_stat->enter_wow = wow_info->enter_wow;
	wow_stat->func_en = wow_info->func_en;
	wow_stat->keep_alive_en = wow_info->keep_alive_info.keep_alive_en;
	wow_stat->disc_det_en = wow_info->disc_det_info.disc_det_en;
	wow_stat->arp_en = wow_info->arp_ofld_info.arp_en;
	wow_stat->ndp_en = wow_info->ndp_ofld_info.ndp_en;
	wow_stat->gtk_en = wow_info->gtk_ofld_info.gtk_en;
	wow_stat->err.init = wow_info->err.init;
	/* deinit */
	wow_stat->mac_pwr = wow_info->mac_pwr;
	wow_stat->wake_rsn = wow_info->wake_rsn;
	wow_stat->err.deinit = wow_info->err.deinit;
}

#ifdef CONFIG_PCI_HCI
enum rtw_phl_status _init_precfg(struct phl_info_t *phl_info)
{
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	u8 b_status = false;
	u8 empty = 0;
	u8 dma_ch = 0;

	do {

		/* 1. stop Tx DMA */
		rtw_hal_wow_cfg_txdma(phl_info->hal, false);

		/* 2. check all queue empty */
		hstatus = rtw_hal_chk_allq_empty(phl_info->hal, &empty);
		if (RTW_HAL_STATUS_SUCCESS != hstatus) {
			PHL_ERR("[wow] rtw_hal_chk_allq_empty failed, status(%u)\n", hstatus);
		} else {
			if (!empty)
				PHL_WARN("[wow] %s : queue is not empty!\n", __func__);
		}
		/* 3. poll dma idle */
		b_status = rtw_hal_poll_txdma_idle(phl_info->hal);

		if (!b_status)
			PHL_ERR("[wow] rtw_hal_poll_txdma_idle failed \n");

		/* 4. configure pcie for wowlan */
		hstatus = rtw_hal_set_wowlan(phl_info->phl_com, phl_info->hal,
						true);

		if (RTW_HAL_STATUS_SUCCESS != hstatus) {
			PHL_ERR("[wow] rtw_hal_cfg_wow_pcie failed, status(%u)\n", hstatus);
			break;
		}
	} while (0);

	if (RTW_HAL_STATUS_SUCCESS != hstatus)
		pstatus = RTW_PHL_STATUS_FAILURE;
	else
		pstatus = RTW_PHL_STATUS_SUCCESS;

	FUNCOUT_WSTS(pstatus);

	return pstatus;
}
#elif defined(CONFIG_USB_HCI)
enum rtw_phl_status _init_precfg(struct phl_info_t *phl_info)
{
	return RTW_PHL_STATUS_SUCCESS;
}
#elif defined(CONFIG_SDIO_HCI)
enum rtw_phl_status _init_precfg(struct phl_info_t *phl_info)
{
	return RTW_PHL_STATUS_SUCCESS;
}
#endif

static enum rtw_phl_status _init_precfg_set_rxfltr(struct phl_info_t *phl_info)
{
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_SUCCESS;

	do {
		hstatus = rtw_hal_set_rxfltr_by_type(phl_info->hal, 0, RTW_PHL_PKT_TYPE_DATA, 0);
	 	if (RTW_HAL_STATUS_SUCCESS != hstatus) {
			PHL_ERR("[wow] set rx filter data drop fail, status(%u)\n", hstatus);
			break;
		}

		hstatus = rtw_hal_set_rxfltr_by_type(phl_info->hal, 0, RTW_PHL_PKT_TYPE_MGNT, 0);
		if (RTW_HAL_STATUS_SUCCESS != hstatus) {
			PHL_ERR("[wow] set rx filter mgnt drop fail, status(%u)\n", hstatus);
			break;
		}

		hstatus = rtw_hal_set_rxfltr_by_type(phl_info->hal, 0, RTW_PHL_PKT_TYPE_CTRL, 0);
		if (RTW_HAL_STATUS_SUCCESS != hstatus) {
			PHL_ERR("[wow] set rx filter ctrl drop fail, status(%u)\n", hstatus);
			break;
		}
	} while (0);

	return (hstatus == RTW_HAL_STATUS_SUCCESS) ?
			RTW_PHL_STATUS_SUCCESS : RTW_PHL_STATUS_FAILURE;
}

static enum rtw_phl_status _init_postcfg_set_rxfltr(struct phl_info_t *phl_info)
{
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_SUCCESS;

	do {
		hstatus = rtw_hal_set_rxfltr_by_type(phl_info->hal, 0, RTW_PHL_PKT_TYPE_DATA, 1);
		if (RTW_HAL_STATUS_SUCCESS != hstatus) {
			PHL_ERR("[wow] set rx filter data to host fail, status(%u)\n", hstatus);
			break;
		}

		hstatus = rtw_hal_set_rxfltr_by_type(phl_info->hal, 0, RTW_PHL_PKT_TYPE_MGNT, 1);
		if (RTW_HAL_STATUS_SUCCESS != hstatus) {
			PHL_ERR("[wow] set rx filter mgnt to host fail, status(%u)\n", hstatus);
			break;
		}

		hstatus = rtw_hal_set_rxfltr_by_type(phl_info->hal, 0, RTW_PHL_PKT_TYPE_CTRL, 1);
		if (RTW_HAL_STATUS_SUCCESS != hstatus) {
			PHL_ERR("[wow] set rx filter ctrl to host fail, status(%u)\n", hstatus);
			break;
		}
	} while (0);

	return (hstatus == RTW_HAL_STATUS_SUCCESS) ?
			RTW_PHL_STATUS_SUCCESS : RTW_PHL_STATUS_FAILURE;
}

#define MAX_POLLING_TRX_STOP 100 /* us */
enum rtw_phl_status phl_wow_init_precfg(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_SUCCESS;
	struct phl_hci_trx_ops *trx_ops = phl_info->hci_trx_ops;
	u32 wait_cnt = 0;

	FUNCIN();

	/* pause sw Tx */
	trx_ops->req_tx_stop(phl_info);

	/* schedule current existing tx handler */
	pstatus = rtw_phl_tx_req_notify(phl_info);
	if (RTW_PHL_STATUS_SUCCESS != pstatus)
		PHL_ERR("[wow] rtw_phl_tx_req_notify fail, status(%u)\n", pstatus);

	/* polling pause sw Tx done */
	while (wait_cnt < MAX_POLLING_TRX_STOP) {
		if (trx_ops->is_tx_pause(phl_info)) {
			PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] sw tx pause succeed.\n");
			break;
		}
		_os_delay_us(phl_info->phl_com->drv_priv, 1);
		wait_cnt++;
	}
	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] stop sw tx wait_cnt %d.\n", wait_cnt);

	/* init pre-configuration for different interfaces */
	pstatus = _init_precfg(phl_info);
	if (RTW_PHL_STATUS_SUCCESS != pstatus)
		return pstatus;

	/* set packet drop by setting rx filter */
	pstatus = _init_precfg_set_rxfltr(phl_info);
	if (RTW_PHL_STATUS_SUCCESS != pstatus)
		return pstatus;

	pstatus = RTW_PHL_STATUS_SUCCESS;

	return pstatus;
}

enum rtw_phl_status phl_wow_init_postcfg(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	struct phl_hci_trx_ops *trx_ops = phl_info->hci_trx_ops;
	u32 wait_cnt = 0;

	FUNCIN();

	/* stop tx/rx hci */
	rtw_hal_cfg_txhci(phl_info->hal, 0);
	rtw_hal_cfg_rxhci(phl_info->hal, 0);

	/* stop dma io */
	rtw_hal_cfg_dma_io(phl_info->hal, 0);

	/* configure wow sleep */
	hstatus = rtw_hal_cfg_wow_sleep(phl_info->hal, true);
	if (RTW_HAL_STATUS_SUCCESS != hstatus)
		return RTW_PHL_STATUS_FAILURE;

	/* stop sw rx */
	trx_ops->req_rx_stop(phl_info);
	pstatus = rtw_phl_start_rx_process(phl_info);
	if (RTW_PHL_STATUS_SUCCESS != pstatus)
		PHL_ERR("[wow] rtw_phl_start_rx_process failed.\n");

	while (wait_cnt < MAX_POLLING_TRX_STOP) {
		if (trx_ops->is_rx_pause(phl_info)) {
			PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] sw rx pause succeed.\n");
			break;
		}
		_os_delay_us(phl_info->phl_com->drv_priv, 1);
		wait_cnt++;
	}

	/* forward rx packet to host by setting rx filter */
	pstatus = _init_postcfg_set_rxfltr(phl_info);

	/* disable interrupt */
	rtw_hal_disable_interrupt(phl_info->phl_com, phl_info->hal);

	/* reset trx */
	trx_ops->trx_reset(phl_info);

	return pstatus;
}

static void _phl_indic_wake_sec_upd(struct phl_info_t *phl_info, u8 aoac_report_get_ok, u8 rx_ready)
{
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	struct rtw_phl_evt_ops *ops = &phl_info->phl_com->evt_ops;
	void *drv_priv = phl_to_drvpriv(phl_info);

	if (NULL != ops->wow_handle_sec_info_update)
		ops->wow_handle_sec_info_update(drv_priv, &wow_info->aoac_info, aoac_report_get_ok, rx_ready);
	else
		PHL_TRACE(COMP_PHL_WOW, _PHL_ERR_, "[wow] %s : evt_ops->wow_handle_sec_info_update is NULL.\n"
			, __func__);
}

static void _phl_handle_aoac_rpt_action(struct phl_info_t *phl_info, bool rx_ready)
{
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	u8 aoac_report_get_ok = false;
	static u8 phase_0_ok = false;

	if (wow_info->wow_wake_info.pairwise_sec_algo) {
		if (rx_ready == false) {
			hstatus = rtw_hal_get_wow_aoac_rpt(phl_info->hal, &wow_info->aoac_info, rx_ready);
			aoac_report_get_ok = (hstatus) ? false : true;
			_phl_indic_wake_sec_upd(phl_info, aoac_report_get_ok, rx_ready);

			phase_0_ok = aoac_report_get_ok;
		}

		if (rx_ready == true && phase_0_ok) {
			hstatus = rtw_hal_get_wow_aoac_rpt(phl_info->hal, &wow_info->aoac_info, rx_ready);
			aoac_report_get_ok = (hstatus) ? false : true;
			_phl_indic_wake_sec_upd(phl_info, aoac_report_get_ok, rx_ready);

			phase_0_ok = false;
		}
	}
}

static enum rtw_phl_status _phl_indic_wake_rsn(struct phl_info_t *phl_info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_SUCCESS;
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	struct rtw_phl_evt_ops *evt_ops = &(phl_info->phl_com->evt_ops);

	FUNCIN_WSTS(phl_status);

	if (NULL != evt_ops->indicate_wake_rsn) {
		evt_ops->indicate_wake_rsn(phl_to_drvpriv(phl_info), wow_info->wake_rsn);
	}

	FUNCOUT_WSTS(phl_status);

	return phl_status;
}

#ifdef CONFIG_PCI_HCI
enum rtw_phl_status _deinit_precfg(struct phl_info_t *phl_info)
{
	rtw_hal_set_wowlan(phl_info->phl_com, phl_info->hal, false);

#ifdef RTW_WKARD_WOW_BDRAM
	rtw_hal_rst_bdram(phl_info->hal);
#endif /* RTW_WKARD_WOW_BDRAM */

	rtw_hal_cfg_dma_io(phl_info->hal, 1);
	rtw_hal_cfg_txhci(phl_info->hal, 1);
	rtw_hal_cfg_rxhci(phl_info->hal, 1);

	/* start tx dma */
	rtw_hal_wow_cfg_txdma(phl_info->hal, true);

	return RTW_PHL_STATUS_SUCCESS;
}
#elif defined(CONFIG_USB_HCI)
enum rtw_phl_status _deinit_precfg(struct phl_info_t *phl_info)
{
	return RTW_PHL_STATUS_SUCCESS;
}
#elif defined(CONFIG_SDIO_HCI)
enum rtw_phl_status _deinit_precfg(struct phl_info_t *phl_info)
{
	return RTW_PHL_STATUS_SUCCESS;
}
#endif

enum rtw_phl_status phl_wow_deinit_precfg(struct phl_info_t *phl_info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_SUCCESS;
	struct phl_hci_trx_ops *trx_ops = phl_info->hci_trx_ops;
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;

	FUNCIN();

	rtw_hal_clear_rwptr(phl_info->hal);
	rtw_hal_set_default_var(phl_info->hal);

	_deinit_precfg(phl_info);

	rtw_hal_get_wake_rsn(phl_info->hal, &wow_info->wake_rsn);
	_phl_indic_wake_rsn(phl_info);

	rtw_hal_cfg_wow_sleep(phl_info->hal, false);

	_phl_handle_aoac_rpt_action(phl_info, false);

	/* resume sw rx */
	trx_ops->trx_resume(phl_info, PHL_REQ_PAUSE_RX);

	rtw_hal_enable_interrupt(phl_info->phl_com, phl_info->hal);

	_phl_handle_aoac_rpt_action(phl_info, true);

	return phl_status;
}

void phl_reset_wow_info(struct phl_info_t *phl_info)
{
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	void *d = phl_to_drvpriv(phl_info);
	u8 i = 0;

	wow_info->enter_wow = 0;
	wow_info->func_en = 0;
	wow_info->pwr_saving = 0;
	wow_info->mac_pwr = 0;
	_os_mem_set(d, &wow_info->err, 0, sizeof(struct phl_wow_error));
	_os_mem_set(d, &wow_info->keep_alive_info, 0, sizeof(struct rtw_keep_alive_info));
	_os_mem_set(d, &wow_info->disc_det_info, 0, sizeof(struct rtw_disc_det_info));
	_os_mem_set(d, &wow_info->nlo_info, 0, sizeof(struct rtw_nlo_info));
	_os_mem_set(d, &wow_info->arp_ofld_info, 0, sizeof(struct rtw_arp_ofld_info));
	_os_mem_set(d, &wow_info->ndp_ofld_info, 0, sizeof(struct rtw_ndp_ofld_info));
	_os_mem_set(d, &wow_info->gtk_ofld_info, 0, sizeof(struct rtw_gtk_ofld_info));
	_os_mem_set(d, &wow_info->realwow_info, 0, sizeof(struct rtw_realwow_info));
	_os_mem_set(d, &wow_info->wow_wake_info, 0, sizeof(struct rtw_wow_wake_info));
	_os_mem_set(d, &wow_info->aoac_info, 0, sizeof(struct rtw_aoac_report));
	for (i = 0; i < MAX_WOW_CAM_NUM; i++) {
		_os_mem_set(d, &wow_info->pattern_match_info.wowcam_info[i],
					0, sizeof(struct rtw_wowcam_upd_info));
	}
	wow_info->wake_rsn = RTW_WOW_RSN_UNKNOWN;
}

enum rtw_phl_status phl_wow_deinit_postcfg(struct phl_info_t *phl_info)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_SUCCESS;
	struct phl_hci_trx_ops *trx_ops = phl_info->hci_trx_ops;

	FUNCIN();

	/* resume sw tx */
	trx_ops->trx_resume(phl_info, PHL_REQ_PAUSE_TX);

	return phl_status;
}

u8 phl_wow_nlo_exist(struct phl_info_t *phl_info)
{
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);

	return wow_info->nlo_info.nlo_exist;
}

enum rtw_phl_status _phl_wow_cfg_pkt_ofld(struct phl_wow_info *wow_info, u8 pkt_type, u8 *pkt_id, void *buf)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_com_t *phl_com = wow_info->phl_info->phl_com;
	u8 macid = (u8)wow_info->sta->macid;
	u32 *token;

	switch(pkt_type) {
	case PKT_TYPE_NULL_DATA:
		token = &wow_info->null_pkt_token;
		break;
	case PKT_TYPE_ARP_RSP:
		token = &wow_info->arp_pkt_token;
		break;
	case PKT_TYPE_NDP:
		token = &wow_info->ndp_pkt_token;
		break;
	case PKT_TYPE_EAPOL_KEY:
		token = &wow_info->eapol_key_pkt_token;
		break;
	case PKT_TYPE_SA_QUERY:
		token = &wow_info->sa_query_pkt_token;
		break;
	default:
		PHL_TRACE(COMP_PHL_WOW, _PHL_ERR_, "[wow] %s : unknown pkt_type %d.\n"
			, __func__, pkt_type);
		return pstatus;
	}

	pstatus = RTW_PHL_PKT_OFLD_REQ(phl_com, macid, pkt_type, token, buf);

	if (pstatus == RTW_PHL_STATUS_SUCCESS)
		*pkt_id = rtw_phl_pkt_ofld_get_id(phl_com, macid, pkt_type);

	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] %s : pkt_type %s, pkt_id %d, token %u, status(%u)\n",
		__func__, phl_get_pkt_ofld_str(pkt_type), *pkt_id, *token, pstatus);

	return pstatus;
}

enum rtw_phl_status phl_wow_func_en(struct phl_info_t *phl_info, struct rtw_phl_stainfo_t *sta)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);
	struct rtw_pkt_ofld_null_info null_info = {0};
	struct rtw_pkt_ofld_arp_rsp_info arp_rsp_info = {0};
	struct rtw_pkt_ofld_na_info na_info = {0};
	struct rtw_pkt_ofld_eapol_key_info eapol_key_info = {0};
	struct rtw_pkt_ofld_sa_query_info sa_query_info = {0};
	struct rtw_hal_wow_cfg cfg;

	FUNCIN();

	if (!wow_info->wow_wake_info.wow_en) {
		PHL_WARN("%s : wow func is not enabled!\n", __func__);
		return pstatus;
	}

	wow_info->sta = sta; /* need to be organized */

	do {

		hstatus = rtw_hal_reset_pkt_ofld_state(phl_info->hal);

		if (RTW_HAL_STATUS_SUCCESS != hstatus) {
			pstatus = RTW_PHL_STATUS_FAILURE;
			break;
		}

		if (wow_info->keep_alive_info.keep_alive_en) {

			_phl_cfg_pkt_ofld_null_info(phl_info, sta, &null_info);

			pstatus = _phl_wow_cfg_pkt_ofld(wow_info,
					PKT_TYPE_NULL_DATA,
					&wow_info->keep_alive_info.null_pkt_id,
					(void *)&null_info);

			if (pstatus != RTW_PHL_STATUS_SUCCESS)
				break;
		}

		if (wow_info->arp_ofld_info.arp_en) {

			_phl_cfg_pkt_ofld_arp_rsp_info(phl_info, sta, &arp_rsp_info);

			pstatus = _phl_wow_cfg_pkt_ofld(wow_info,
					PKT_TYPE_ARP_RSP,
					&wow_info->arp_ofld_info.arp_rsp_id,
					(void *)&arp_rsp_info);

			if (pstatus != RTW_PHL_STATUS_SUCCESS)
				break;
		}

		if (wow_info->ndp_ofld_info.ndp_en) {

			_phl_cfg_pkt_ofld_na_info(phl_info, sta, &na_info);

			pstatus = _phl_wow_cfg_pkt_ofld(wow_info,
					PKT_TYPE_NDP, &wow_info->ndp_ofld_info.ndp_id,
					(void *)&na_info);

			if (pstatus != RTW_PHL_STATUS_SUCCESS)
				break;
		}

		if (wow_info->gtk_ofld_info.gtk_en) {
			_phl_cfg_pkt_ofld_eapol_key_info(phl_info, sta, &eapol_key_info);

			pstatus = _phl_wow_cfg_pkt_ofld(wow_info,
					PKT_TYPE_EAPOL_KEY, &wow_info->gtk_ofld_info.gtk_rsp_id,
					(void *)&eapol_key_info);

			if (pstatus != RTW_PHL_STATUS_SUCCESS)
				break;

			if (wow_info->gtk_ofld_info.ieee80211w_en) {
				_phl_cfg_pkt_ofld_sa_query_info(phl_info, sta, &sa_query_info);

				pstatus = _phl_wow_cfg_pkt_ofld(wow_info,
					PKT_TYPE_SA_QUERY, &wow_info->gtk_ofld_info.sa_query_id,
					(void *)&sa_query_info);

				if (pstatus != RTW_PHL_STATUS_SUCCESS)
					break;
			}
		}

		cfg.keep_alive_cfg = &wow_info->keep_alive_info;
		cfg.disc_det_cfg = &wow_info->disc_det_info;
		cfg.nlo_cfg = &wow_info->nlo_info;
		cfg.arp_ofld_cfg = &wow_info->arp_ofld_info;
		cfg.ndp_ofld_cfg = &wow_info->ndp_ofld_info;
		cfg.gtk_ofld_cfg = &wow_info->gtk_ofld_info;
		cfg.realwow_cfg = &wow_info->realwow_info;
		cfg.wow_wake_cfg = &wow_info->wow_wake_info;
		cfg.pattern_match_info = &wow_info->pattern_match_info;

		hstatus = rtw_hal_wow_func_en(phl_info->phl_com, phl_info->hal, sta->macid, &cfg);

		if (hstatus != RTW_HAL_STATUS_SUCCESS) {
			pstatus = RTW_PHL_STATUS_FAILURE;
			break;
		}

		pstatus = RTW_PHL_STATUS_SUCCESS;

	} while (0);

	PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] %s status (%u).\n", __func__, pstatus);

	return pstatus;
}

enum rtw_phl_status phl_wow_func_dis(struct phl_info_t *phl_info, struct rtw_phl_stainfo_t *sta)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;
	struct phl_wow_info *wow_info = phl_to_wow_info(phl_info);

	FUNCIN_WSTS(pstatus);

	if (!wow_info->wow_wake_info.wow_en) {
		PHL_WARN("%s : wow func is not enabled!\n", __func__);
		return pstatus;
	}

	wow_info->sta = sta; /* need to be organized */

	if (wow_info->keep_alive_info.keep_alive_en) {
		rtw_phl_pkt_ofld_cancel(phl_info->phl_com, (u8)sta->macid,
							PKT_TYPE_NULL_DATA, &wow_info->null_pkt_token);
	}

	if (wow_info->arp_ofld_info.arp_en) {
		rtw_phl_pkt_ofld_cancel(phl_info->phl_com, (u8)sta->macid,
							PKT_TYPE_ARP_RSP, &wow_info->arp_pkt_token);
	}

	if (wow_info->ndp_ofld_info.ndp_en) {
		rtw_phl_pkt_ofld_cancel(phl_info->phl_com, (u8)sta->macid,
							PKT_TYPE_NDP, &wow_info->ndp_pkt_token);
	}

	if (wow_info->gtk_ofld_info.gtk_en) {
		rtw_phl_pkt_ofld_cancel(phl_info->phl_com, (u8)sta->macid,
							PKT_TYPE_EAPOL_KEY, &wow_info->eapol_key_pkt_token);
		if (wow_info->gtk_ofld_info.ieee80211w_en) {
			rtw_phl_pkt_ofld_cancel(phl_info->phl_com, (u8)sta->macid,
								PKT_TYPE_SA_QUERY, &wow_info->sa_query_pkt_token);
		}
	}

	hstatus = rtw_hal_wow_func_dis(phl_info->phl_com, phl_info->hal, sta->macid);

	if (RTW_HAL_STATUS_SUCCESS == hstatus)
		PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] %s successfully done.\n", __func__);
	else
		PHL_TRACE(COMP_PHL_WOW, _PHL_INFO_, "[wow] %s fail, hstatus(%u)\n", __func__, hstatus);

	return RTW_PHL_STATUS_SUCCESS;
}

void phl_wow_pwr_cfg(struct phl_info_t *phl_info,
							struct rtw_phl_stainfo_t *sta, u8 enter_wow)
{
	FUNCIN();
#if 0
	struct rtw_hal_lps_info info;

	info.en_lps = (enter_wow) ? true : false;
	info.macid = sta->macid;
	info.listen_bcn_mode = RTW_HAL_LPS_RLBM_MAX;
	info.awake_interval = 100;

	if (sta->wrole->mstate == MLME_NO_LINK) {
		/* IPS */
	} else if (sta->wrole->mstate == MLME_LINKED) {
		/* LPS */
		if (info.en_lps) {
			rtw_hal_ps_lps_cfg(phl_info->hal, &info);
			rtw_hal_ps_pwr_lvl_cfg(phl_info->hal, PS_PWR_STATE_ACTIVE, PS_PWR_STATE_CLK_GATED);
		}
		else {
			rtw_hal_ps_pwr_lvl_cfg(phl_info->hal, PS_PWR_STATE_CLK_GATED, PS_PWR_STATE_ACTIVE);
			rtw_hal_ps_lps_cfg(phl_info->hal, &info);
		}
	} else {
		PHL_ERR("%s : error sta mlme state!\n", __func__);
	}
#endif
}

#define case_rsn(rsn) \
	case RTW_WOW_RSN_##rsn: return #rsn

const char *rtw_phl_get_wow_rsn_str(void *phl, enum rtw_wow_wake_reason wake_rsn)
{
	switch (wake_rsn) {
	case_rsn(UNKNOWN); /* RTW_WOW_RSN_UNKNOWN */
	case_rsn(RX_PAIRWISEKEY);
	case_rsn(RX_GTK);
	case_rsn(RX_FOURWAY_HANDSHAKE);
	case_rsn(RX_DISASSOC);
	case_rsn(RX_DEAUTH);
	case_rsn(RX_ARP_REQUEST);
	case_rsn(RX_NS);
	case_rsn(RX_EAPREQ_IDENTIFY);
	case_rsn(FW_DECISION_DISCONNECT);
	case_rsn(RX_MAGIC_PKT);
	case_rsn(RX_UNICAST_PKT);
	case_rsn(RX_PATTERN_PKT);
	case_rsn(RTD3_SSID_MATCH);
	case_rsn(RX_DATA_PKT);
	case_rsn(RX_SSDP_MATCH);
	case_rsn(RX_WSD_MATCH);
	case_rsn(RX_SLP_MATCH);
	case_rsn(RX_LLTD_MATCH);
	case_rsn(RX_MDNS_MATCH);
	case_rsn(RX_REALWOW_V2_WAKEUP_PKT);
	case_rsn(RX_REALWOW_V2_ACK_LOST);
	case_rsn(RX_REALWOW_V2_TX_KAPKT);
	case_rsn(ENABLE_FAIL_DMA_IDLE);
	case_rsn(ENABLE_FAIL_DMA_PAUSE);
	case_rsn(RTIME_FAIL_DMA_IDLE);
	case_rsn(RTIME_FAIL_DMA_PAUSE);
	case_rsn(RX_SNMP_MISMATCHED_PKT);
	case_rsn(RX_DESIGNATED_MAC_PKT);
	case_rsn(NLO_SSID_MACH);
	case_rsn(AP_OFFLOAD_WAKEUP);
	case_rsn(DMAC_ERROR_OCCURRED);
	case_rsn(EXCEPTION_OCCURRED);
	case_rsn(CLK_32K_UNLOCK);
	case_rsn(CLK_32K_LOCK);
	default:
		return "UNDEFINED"; /* RTW_WOW_RSN_MAX */
	}
}

#endif /* CONFIG_WOWLAN */
