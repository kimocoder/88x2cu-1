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
#define _PHL_MCC_C_
#include "phl_headers.h"

#ifdef CONFIG_MCC_SUPPORT
#include "phl_mcc.h"
void _mcc_dump_state(enum phl_mcc_state *state)
{
	if (MCC_NONE == *state) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_dump_state(): MCC_NONE\n");
	} else if (MCC_TRIGGER_FW_EN == *state) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_dump_state(): MCC_TRIGGER_FW_EN\n");
	} else if (MCC_FW_EN_FAIL == *state) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_dump_state(): MCC_FW_EN_FAIL\n");
	} else if (MCC_RUNING == *state) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_dump_state(): MCC_RUNING\n");
	} else if (MCC_TRIGGER_FW_DIS == *state) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_dump_state(): MCC_TRIGGER_FW_DIS\n");
	} else if (MCC_FW_DIS_FAIL == *state) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_dump_state(): MCC_FW_DIS_FAIL\n");
	} else if (MCC_STOP == *state) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_dump_state(): MCC_STOP\n");
	} else {
		PHL_TRACE(COMP_PHL_MCC, _PHL_WARNING_, "_mcc_dump_state(): Undefined state(%d)\n",
			*state);
	}
}

void _mcc_dump_mode(enum rtw_phl_mcc_mode *mode)
{
	if (RTW_PHL_MCC_AP_CLIENT_MODE == *mode) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_dump_mode(): RTW_PHL_MCC_AP_CLIENT_MODE\n");
	} else if (RTW_PHL_MCC_2CLIENTS_MODE == *mode) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_dump_mode(): RTW_PHL_MCC_2CLIENTS_MODE\n");
	} else {
		PHL_TRACE(COMP_PHL_MCC, _PHL_WARNING_, "_mcc_dump_mode(): Undefined mode(%d)\n",
			*mode);
	}
}

void _mcc_dump_sync_tsf_info(struct rtw_phl_mcc_sync_tsf_info *info)
{
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_dump_sync_tsf_info(): sync_en(%d), source macid(%d), target macid(%d), offset(%d)\n",
		info->sync_en, info->source, info->target, info->offset);
}

void _mcc_dump_role_info(struct rtw_phl_mcc_role *mrole)
{
	struct rtw_phl_mcc_policy_info *policy = &mrole->policy;
	u8 i = 0;

	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, ">>> _mcc_dump_role_info(): wrole id(%d), type(%d), macid(%d), bcn_intvl(%d)\n",
		mrole->wrole->id, mrole->wrole->type, mrole->macid,
		mrole->bcn_intvl);
	for (i = 0; i < PHL_MACID_MAX_ARRAY_NUM; i++) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_dump_role_info(): macid_map[%d]= 0x%08X\n",
			i, mrole->used_macid.bitmap[i]);
	}
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_dump_role_info(): chan(%d), center_ch(%d), bw(%d), offset(%d)\n",
		mrole->chandef->chan, mrole->chandef->center_ch,
		mrole->chandef->bw, mrole->chandef->offset);
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_dump_role_info(): group(%d), c2h_rpt(%d), tx_null_early(%d)\n",
		mrole->group, policy->c2h_rpt, policy->tx_null_early);
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_dump_role_info(): dis_tx_null(%d), in_curr_ch(%d), dis_sw_retry(%d)\n",
		policy->dis_tx_null, policy->in_curr_ch, policy->dis_sw_retry);
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "<<< _mcc_dump_role_info(): sw_retry_count(%d), duration(%d)\n",
		policy->sw_retry_count, policy->duration);
}

void _mcc_dump_pattern(struct rtw_phl_mcc_pattern *m_pattern)
{
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_dump_pattern(): tob_r(%d), toa_r(%d), tob_a(%d), toa_a(%d), bcns_offset(%d)\n",
		m_pattern->tob_r, m_pattern->toa_r, m_pattern->tob_a,
		m_pattern->toa_a, m_pattern->bcns_offset);
}

void _mcc_dump_ref_role_info(struct rtw_phl_mcc_en_info *info)
{
	struct rtw_phl_mcc_role *ref_role = get_ref_role(info);

	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_dump_ref_role_info(): mrole idx(%d), wrole id(%d), macid(%d) chan(%d), bw(%d), offset(%d)\n",
		info->ref_role_idx, ref_role->wrole->id, ref_role->macid,
		ref_role->chandef->chan, ref_role->chandef->bw,
		ref_role->chandef->offset);
}

void _mcc_dump_en_info(struct rtw_phl_mcc_en_info *info)
{
	struct rtw_phl_mcc_role *m_role = NULL;
	struct rtw_phl_mcc_role *ref_role = get_ref_role(info);
	u8 midx = 0;

	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_dump_en_info(): mrole_map(0x%x), role_num(%d), mcc_intvl(%d), Start tsf(0x%08X %08X)\n",
		info->role_map, info->role_num, info->mcc_intvl,
		info->tsf_high, info->tsf_low);
	_mcc_dump_ref_role_info(info);
	_mcc_dump_sync_tsf_info(&info->sync_tsf_info);
	_mcc_dump_pattern(&info->m_pattern);
	for (midx = 0; midx < MCC_ROLE_NUM; midx++) {
		if (!(info->role_map & BIT(midx)))
			continue;
		m_role = &info->mcc_role[midx];
		_mcc_dump_role_info(m_role);
	}
}

void _mcc_dump_bt_ino(struct rtw_phl_mcc_bt_info *bt_info)
{
	u8 seg_num = BT_SEG_NUM;

	if (seg_num < 2)
		return;
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_dump_bt_ino(): bt_dur(%d), bt_seg_num(%d), bt_seg[0](%d), bt_seg[1](%d), add_bt_role(%d)\n",
		bt_info->bt_dur, bt_info->bt_seg_num, bt_info->bt_seg[0],
		bt_info->bt_seg[1], bt_info->add_bt_role);
}

void _mcc_dump_mcc_info(struct phl_mcc_info *minfo)
{
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, ">>> _mcc_dump_mcc_info():\n");
	_mcc_dump_mode(&minfo->mcc_mode);
	_mcc_dump_state(&minfo->state);
	_mcc_dump_bt_ino(&minfo->bt_info);
	_mcc_dump_en_info(&minfo->en_info);
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "<<< _mcc_dump_mcc_info():\n");
}

void _mcc_set_state(struct phl_mcc_info *minfo, enum phl_mcc_state state)
{
	PHL_TRACE(COMP_PHL_MCC, _PHL_ALWAYS_, "_mcc_set_state(): Set from (%d) to (%d)\n",
		minfo->state, state);
	minfo->state = state;
	_mcc_dump_state(&minfo->state);
}

bool _mcc_is_ap_category(struct rtw_wifi_role_t *wrole)
{
	bool ret = false;

	if (wrole->type == PHL_RTYPE_AP || wrole->type == PHL_RTYPE_P2P_GO)
		ret = true;
	return ret;
}

bool _mcc_is_client_category(struct rtw_wifi_role_t *wrole)
{
	bool ret = false;

	if (wrole->type == PHL_RTYPE_STATION || wrole->type == PHL_RTYPE_P2P_GC)
		ret = true;
	return ret;
}

struct rtw_phl_mcc_role *
_mcc_get_mrole_by_wrole(struct phl_mcc_info *minfo,
				struct rtw_wifi_role_t *wrole)
{
	struct rtw_phl_mcc_en_info *en_info = &minfo->en_info;
	struct rtw_phl_mcc_role *m_role = NULL;
	u8 midx = 0;

	for (midx = 0; midx < MCC_ROLE_NUM; midx++) {
		if (!(en_info->role_map & BIT(midx)))
			continue;
		m_role = &en_info->mcc_role[midx];
		if (m_role->wrole == wrole) {
			PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_get_mrole_by_wrole(): Get mrole in mrole_idx(%d)\n",
				midx);
			return m_role;
		}
	}
	return NULL;
}

u8
_mcc_get_mrole_idx_by_wrole(struct phl_mcc_info *minfo,
				struct rtw_wifi_role_t *wrole, u8 *idx)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_mcc_en_info *en_info = &minfo->en_info;
	struct rtw_phl_mcc_role *m_role = NULL;
	u8 midx = 0;

	for (midx = 0; midx < MCC_ROLE_NUM; midx++) {
		if (!(en_info->role_map & BIT(midx)))
			continue;
		m_role = &en_info->mcc_role[midx];
		if (m_role->wrole == wrole) {
			PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_get_mrole_idx_by_wrole(): Get mrole in mrole_idx(%d)\n",
				midx);
			*idx = midx;
			status = RTW_PHL_STATUS_SUCCESS;
			break;
		}
	}
	return status;
}

struct rtw_phl_mcc_role *
_mcc_get_mrole_by_category(struct rtw_phl_mcc_en_info *en_info,
			enum _mcc_role_cat category)
{
	struct rtw_phl_mcc_role *m_role = NULL;
	u8 midx = 0;

	for (midx = 0; midx < MCC_ROLE_NUM; midx++) {
		if (!(en_info->role_map & BIT(midx)))
			continue;
		m_role = &en_info->mcc_role[midx];
		if (MCC_ROLE_AP_CAT == category) {
			if (_mcc_is_ap_category(m_role->wrole))
				return m_role;
		} else if (MCC_ROLE_CLIENT_CAT == category) {
			if (_mcc_is_client_category(m_role->wrole))
				return m_role;
		} else {
			PHL_TRACE(COMP_PHL_MCC, _PHL_WARNING_, "_mcc_get_mrole_by_category(): Undefined category(%d)\n",
				category);
			break;
		}
	}
	return NULL;
}

enum rtw_phl_status _mcc_transfer_mode(struct phl_info_t *phl,
				struct phl_mcc_info *minfo, u8 role_map)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct rtw_wifi_role_t *wrole = NULL;
	u8 ridx = 0, ap_num = 0, client_num = 0;

	for (ridx = 0; ridx < MAX_WIFI_ROLE_NUMBER; ridx++) {
		if (!(role_map & BIT(ridx)))
			continue;
		wrole = phl_get_wrole_by_ridx(phl, ridx);
		if (wrole == NULL) {
			PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_transfer_mode(): get wrole fail, role_idx(%d)\n",
				ridx);
			goto exit;
		}
		if (_mcc_is_client_category(wrole)) {
			client_num++;
		} else if (_mcc_is_ap_category(wrole)) {
			ap_num++;
		} else {
			PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_transfer_mode(): undefined category, role->type(%d), ridx(%d), shall check code flow\n",
				wrole->type, ridx);
			goto exit;
		}
	}
	if ((client_num + ap_num > MAX_MCC_GROUP_ROLE) ||
		(client_num + ap_num < MIN_MCC_GROUP_ROLE)){
		PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_transfer_mode(): client_num(%d) + ap_num(%d) is illegal num, please check code flow\n",
			client_num, ap_num);
		goto exit;
	}
	if (ap_num == 1 && client_num == 1) {
		minfo->mcc_mode = RTW_PHL_MCC_AP_CLIENT_MODE;
	} else if (ap_num == 0 && client_num == 2) {
		minfo->mcc_mode = RTW_PHL_MCC_2CLIENTS_MODE;
	} else {
		PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_transfer_mode(): Undefined mode, please check code flow\n");
		goto exit;
	}
	_mcc_dump_mode(&minfo->mcc_mode);
	status = RTW_PHL_STATUS_SUCCESS;
exit:
	return status;
}

enum rtw_phl_status _mcc_get_role_map(struct phl_info_t *phl,
				struct hw_band_ctl_t *band_ctrl, u8 *role_map)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	void *drv = phl_to_drvpriv(phl);
	struct rtw_chan_ctx *chanctx = NULL;
	_os_list *chan_ctx_list = &band_ctrl->chan_ctx_queue.queue;

	*role_map = 0;
	_os_spinlock(drv, &band_ctrl->chan_ctx_queue.lock, _ps, NULL);
	phl_list_for_loop(chanctx, struct rtw_chan_ctx, chan_ctx_list, list) {
		*role_map |= chanctx->role_map;
	}
	_os_spinunlock(drv, &band_ctrl->chan_ctx_queue.lock, _ps, NULL);
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_get_role_map(): role_map(%d)\n",
		*role_map);
	status = RTW_PHL_STATUS_SUCCESS;
	return status;
}

void _mcc_set_unspecific_dur(struct phl_mcc_info *minfo)
{
	struct rtw_phl_mcc_en_info *en_info = &minfo->en_info;
	struct rtw_phl_mcc_role *m_role = NULL;
	u8 midx = 0;

	for (midx = 0; midx < MCC_ROLE_NUM; midx++) {
		if (!(en_info->role_map & BIT(midx)))
			continue;
		m_role = &en_info->mcc_role[midx];
		m_role->policy.duration = MCC_DUR_NONSPECIFIC;
	}
}

void _mcc_fill_default_policy(struct phl_info_t *phl,
				struct rtw_wifi_role_t *wrole,
				struct rtw_phl_mcc_policy_info *policy)
{
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_fill_default_policy(): set mcc policy by default setting\n");
	policy->c2h_rpt = RTW_MCC_RPT_ALL;
	policy->tx_null_early = 3;
	policy->dis_tx_null = _mcc_is_client_category(wrole) ? 0 : 1;
	policy->in_curr_ch = 0;
	policy->dis_sw_retry = 1;
	policy->sw_retry_count = 0;
	policy->duration = _mcc_is_client_category(wrole) ?
					DEFAULT_CLIENT_DUR : DEFAULT_AP_DUR;
	policy->rfk_by_pass = rtw_hal_check_ch_rfk(phl->hal, &wrole->chandef) ?
						false : true;
}

void _mcc_fill_mcc_role_policy_info(struct phl_info_t *phl,
		struct rtw_wifi_role_t *wrole, struct rtw_phl_mcc_role *mrole)
{
	PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, ">>> _mcc_fill_mcc_role_policy_info()\n");
	/* if get from core
	else*/
	_mcc_fill_default_policy(phl, wrole, &mrole->policy);
}

void _mcc_fill_macid_bitmap_by_role(struct phl_info_t *phl,
					struct rtw_phl_mcc_role *mrole)
{
	struct macid_ctl_t *mc = phl_to_mac_ctrl(phl);
	struct rtw_phl_mcc_macid_bitmap *used_macid = &mrole->used_macid;
	u8 i = 0, max_map_idx = 0;

	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, ">>> _mcc_fill_macid_bitmap_by_role()\n");
	for (i = 0; i < PHL_MACID_MAX_ARRAY_NUM; i++) {
		if ((mc->wifi_role_usedmap[mrole->wrole->id][i] != 0) &&
							(max_map_idx <= i)) {
			PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_fill_macid_bitmap_by_role(): macid_map[%d]=0x%08x\n",
				i, mc->wifi_role_usedmap[mrole->wrole->id][i]);
			max_map_idx = i;
		}
	}
	used_macid->bitmap = &mc->wifi_role_usedmap[mrole->wrole->id][0];
	used_macid->len = (max_map_idx + 1) * sizeof(u32);
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_fill_macid_bitmap_by_role(): bitmap->len(%d), max_map_idx(%d)\n",
		used_macid->len, max_map_idx);
}

enum rtw_phl_status _mcc_fill_mcc_role_basic_info(struct phl_info_t *phl,
		struct rtw_wifi_role_t *wrole, struct rtw_phl_mcc_role *mrole)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_stainfo_t *sta = rtw_phl_get_stainfo_self(phl, wrole);

	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, ">>> _mcc_fill_mcc_role_basic_info()\n");
	if (sta == NULL) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_fill_mcc_role_basic_info(): transfer mcc mode failed\n");
		goto exit;
	}
	mrole->wrole = wrole;
	mrole->macid = sta->macid;
#ifdef RTW_PHL_BCN
	if (_mcc_is_ap_category(wrole))
		mrole->bcn_intvl = (u16)wrole->bcn_cmn.bcn_interval;
	else
#endif
		mrole->bcn_intvl = sta->asoc_cap.bcn_interval;
	if (mrole->bcn_intvl == 0) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_fill_mcc_role_basic_info(): mrole->bcn_intvl ==0, please check code of core layer.\n");
		goto exit;
	}
	mrole->chandef = &wrole->chandef;
	mrole->group = wrole->hw_band;
	_mcc_fill_macid_bitmap_by_role(phl, mrole);
	status = RTW_PHL_STATUS_SUCCESS;
exit:
	return status;
}

enum rtw_phl_status _mcc_fill_ref_role_info(struct phl_info_t *phl,
			struct rtw_phl_mcc_en_info *en_info,
			struct rtw_wifi_role_t *wrole)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	u8 ridx = 0;
	struct rtw_phl_mcc_role *mrole = NULL;

	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, ">>> _mcc_fill_ref_role_info()\n");
	mrole = &en_info->mcc_role[REF_ROLE_IDX];
	status = _mcc_fill_mcc_role_basic_info(phl, wrole, mrole);
	if (RTW_PHL_STATUS_SUCCESS != status) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_fill_ref_role_info(): set basic info failed\n");
		goto exit;
	}
	_mcc_fill_mcc_role_policy_info(phl, wrole, mrole);
	en_info->role_map |= BIT(REF_ROLE_IDX);
	en_info->role_num++;
	status = RTW_PHL_STATUS_SUCCESS;
exit:
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_fill_ref_role_info(): status(%d), wrole id(%d), wrole->type(%d), Fill mrole(%d) Info\n",
			status, wrole->id, wrole->type, REF_ROLE_IDX);
	return status;
}

enum rtw_phl_status _mcc_fill_role_info(struct phl_info_t *phl,
			struct rtw_phl_mcc_en_info *en_info, u8 role_map,
			struct rtw_wifi_role_t *cur_role)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct rtw_wifi_role_t *wrole = NULL;
	struct rtw_phl_mcc_role *mrole = NULL;
	u8 ridx = 0, mcc_idx = 0;

	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, ">>> _mcc_fill_role_info()\n");
	if (RTW_PHL_STATUS_SUCCESS != _mcc_fill_ref_role_info(phl, en_info,
								cur_role)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_fill_role_info(): set ref role info failed\n");
		goto exit;
	}
	mcc_idx = en_info->role_num;
	role_map &= ~(BIT(cur_role->id));
	for (ridx = 0; ridx < MAX_WIFI_ROLE_NUMBER; ridx++) {
		if (!(role_map & BIT(ridx)))
			continue;
		wrole = phl_get_wrole_by_ridx(phl, ridx);
		if (wrole == NULL) {
			PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_fill_role_info(): get wrole fail, role_idx(%d)\n",
				ridx);
			goto exit;
		}
		if (mcc_idx >= MCC_ROLE_NUM) {
			PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_fill_role_info(): mcc_idx(%d) >= MCC_ROLE_NUM(%d)\n",
				mcc_idx, MCC_ROLE_NUM);
			goto exit;
		}
		PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_fill_role_info(): wrole(%d), wrole->type(%d), Fill mrole(%d) Info\n",
			ridx, wrole->type, mcc_idx);
		mrole = &en_info->mcc_role[mcc_idx];
		status = _mcc_fill_mcc_role_basic_info(phl, wrole, mrole);
		if (RTW_PHL_STATUS_SUCCESS != status) {
			PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_fill_role_info(): set basic info failed\n");
			goto exit;
		}
		_mcc_fill_mcc_role_policy_info(phl, wrole, mrole);
		en_info->role_map |= BIT(mcc_idx);
		en_info->role_num++;
		mcc_idx ++;
	}
	status = RTW_PHL_STATUS_SUCCESS;
exit:
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "<<< _mcc_fill_role_info(): status(%d), role_map(0x%x), mcc_role_map(0x%x)\n",
		status, role_map, en_info->role_map);
	return status;
}

void _mcc_fill_coex_mode(struct phl_info_t *phl, struct phl_mcc_info *minfo)
{
	/* if get from core or ....
	else*/
	minfo->coex_mode = RTW_PHL_MCC_COEX_MODE_BT_MASTER;
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_fill_coex_mode(): Set default mode(%d)\n",
		minfo->coex_mode);
}

void _mcc_fill_bt_dur(struct phl_info_t *phl, struct phl_mcc_info *minfo)
{
	minfo->bt_info.bt_dur = (u8)rtw_hal_get_btc_req_slot(phl->hal);
	minfo->bt_info.bt_seg_num = 1;
	minfo->bt_info.bt_seg[0] = minfo->bt_info.bt_dur;
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_fill_bt_dur(): minfo->bt_info.bt_dur(%d)\n",
		minfo->bt_info.bt_dur);

}

void _mcc_clean_noa(struct phl_info_t *phl, struct rtw_phl_mcc_en_info *en_info)
{
	struct phl_com_mcc_info *com_info = phl_to_com_mcc_info(phl);
	struct rtw_phl_mcc_noa param = {0};

	if (com_info == NULL) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_clean_noa(): Get mcc common info failed\n");
	} else if (com_info->ops.mcc_update_noa) {
		struct rtw_phl_mcc_role *ap_role = NULL;
		ap_role = _mcc_get_mrole_by_category(en_info, MCC_ROLE_AP_CAT);
		if (NULL == ap_role) {
			PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_clean_noa(): Get AP role fail\n");
			goto exit;
		}
		param.wrole = ap_role->wrole;
		PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_clean_noa()\n");
		com_info->ops.mcc_update_noa(com_info->ops.priv, &param);
	}
exit:
	return;
}

void _mcc_up_noa(struct phl_info_t *phl, struct phl_mcc_info *minfo)
{
	struct phl_com_mcc_info *com_info = phl_to_com_mcc_info(phl);
	struct rtw_phl_mcc_en_info *en_info = &minfo->en_info;
	struct rtw_phl_mcc_bt_info *bt = &minfo->bt_info;
	struct rtw_phl_mcc_role *role_ref = get_ref_role(en_info);
	struct rtw_phl_mcc_role *role_ano = (role_ref == &en_info->mcc_role[0])
				? &en_info->mcc_role[1] : &en_info->mcc_role[0];
	u16 d_r = role_ref->policy.duration, d_a = role_ano->policy.duration;
	struct rtw_phl_mcc_noa param = {0};
	u64 mcc_start = 0, noa_start = 0;

	PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, ">>> _mcc_up_noa()\n");
	if (com_info == NULL) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_up_noa(): Get mcc common info failed\n");
		goto exit;
	}
	if (!com_info->ops.mcc_update_noa)
		goto exit;
	mcc_start = en_info->tsf_high;
	mcc_start = mcc_start << 32;
	mcc_start |= en_info->tsf_low;
	if (_mcc_is_ap_category(role_ref->wrole)){
		/*calculate end time of GO*/
		noa_start = mcc_start + (d_r * TU);
		param.dur = en_info->mcc_intvl - d_r;
		param.wrole = role_ref->wrole;
	} else {
		u32 tsf_ref_h = 0, tsf_ref_l = 0, tsf_ano_h = 0, tsf_ano_l = 0;
		u64 tsf_ref = 0, tsf_ano = 0;
		PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_up_noa(): AP Role isn't ref role, we need to get 2 port tsf\n");
		if (RTW_HAL_STATUS_SUCCESS != rtw_hal_mcc_get_2ports_tsf(
				phl->hal, role_ref->group, role_ref->macid,
				role_ano->macid, &tsf_ref_h, &tsf_ref_l,
				&tsf_ano_h, &tsf_ano_l)) {
			PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_up_noa(): Get 2 port tsf failed\n");
			goto exit;
		}
		tsf_ref = tsf_ref_h;
		tsf_ref = tsf_ref << 32;
		tsf_ref |= tsf_ref_l;
		tsf_ano = tsf_ano_h;
		tsf_ano = tsf_ano << 32;
		tsf_ano |= tsf_ano_l;
		PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_up_noa(): tsf_ref: 0x%08X %08x, tsf_ano: 0x%08x %08x\n",
			(u32)(tsf_ref >> 32), (u32)tsf_ref,
			(u32)(tsf_ano >> 32), (u32)tsf_ano);
		/*calculate end time of GO*/
		noa_start = mcc_start + (en_info->mcc_intvl * TU);
		if (bt->add_bt_role) {
			if(bt->bt_seg_num == 1) {
				noa_start -= (bt->bt_seg[0] * TU);
			} else if (bt->bt_seg_num == 2) {
				noa_start -= (bt->bt_seg[1] * TU);
			} else {
				PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_up_noa(): error bt_seg_num(%d), please check code flow\n",
					bt->bt_seg_num);
				goto exit;
			}
		}
		noa_start = noa_start - tsf_ref + tsf_ano;
		param.dur = en_info->mcc_intvl - d_a;
		param.wrole = role_ano->wrole;
	}
	param.start_t_h = noa_start >> 32;
	param.start_t_l = (u32)noa_start;
	param.cnt = 255;
	param.interval = en_info->mcc_intvl;
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_up_noa(): IsGORef(%d), mcc_start(0x%08x %08x), NOA_start(0x%08x %08x), dur(%d), cnt(%d), interval(%d)\n",
		_mcc_is_ap_category(role_ref->wrole),
		(u32)(mcc_start >> 32), (u32)mcc_start, param.start_t_h,
		param.start_t_l, param.dur, param.cnt, param.interval);
	_mcc_dump_bt_ino(bt);
	com_info->ops.mcc_update_noa(com_info->ops.priv, &param);
exit:
	return;
}

void _mcc_adjust_dur_for_2_band_mcc_2role_bt(struct phl_mcc_info *minfo,
	struct rtw_phl_mcc_role *role_2g, struct rtw_phl_mcc_role *role_non_2g)
{
	struct rtw_phl_mcc_en_info *en_info = &minfo->en_info;
	enum rtw_phl_mcc_coex_mode *coex_mode = &minfo->coex_mode;
	u8 adjust = false;

	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_adjust_dur_for_2_band_mcc_2role_bt(): coex_mode(%d) 2G_Dur(%d), 5G_Dur(%d), BT_Dur(%d)\n",
		*coex_mode, role_2g->policy.duration,
		role_non_2g->policy.duration, minfo->bt_info.bt_dur);
	if (role_non_2g->policy.duration < minfo->bt_info.bt_dur) {
		if (*coex_mode == RTW_PHL_MCC_COEX_MODE_BT_MASTER) {
			role_non_2g->policy.duration = (u8)minfo->bt_info.bt_dur;
			role_2g->policy.duration = (u8)(en_info->mcc_intvl -
						role_non_2g->policy.duration);
			adjust = true;
			PHL_TRACE(COMP_PHL_MCC, _PHL_WARNING_, "_mcc_adjust_dur_for_2_band_mcc_2role_bt(): coex_mode == RTW_PHL_MCC_COEX_MODE_BT_MASTER, we adjust 5G duration for BT\n");
		} else if (*coex_mode == RTW_PHL_MCC_COEX_MODE_WIFI_MASTER) {
			PHL_TRACE(COMP_PHL_MCC, _PHL_WARNING_, "_mcc_adjust_dur_for_2_band_mcc_2role_bt(): coex_mode == RTW_PHL_MCC_COEX_MODE_WIFI_MASTER, we don't adjust 5G duration for BT\n");
		} else {
			PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_adjust_dur_for_2_band_mcc_2role_bt(): coex_mode(%d), Undefined mode, ignore bt duration\n",
				*coex_mode);
		}
	} else {
		PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_adjust_dur_for_2_band_mcc_2role_bt(): 5G_Dur(%d) > BT_Dur(%d), no need to adjust 5G duration for BT\n",
			role_non_2g->policy.duration, minfo->bt_info.bt_dur);
	}
	if (adjust) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_WARNING_, "_mcc_adjust_dur_for_2_band_mcc_2role_bt(): After adjust, 2G_Dur(%d), 5G_Dur(%d), BT_Dur(%d)\n",
			role_2g->policy.duration, role_non_2g->policy.duration,
			minfo->bt_info.bt_dur);
	}
}

/*
 * Wifi1_Dur BT_DUR1 Wifi2_Dur BT_DUR2
 */
void _mcc_discision_dur_for_2g_mcc_2role_bt(struct phl_mcc_info *minfo)
{
	struct rtw_phl_mcc_en_info *en_info = &minfo->en_info;
	struct rtw_phl_mcc_role *m_role1 = &en_info->mcc_role[0];
	struct rtw_phl_mcc_role *m_role2 = &en_info->mcc_role[1];
	u16 wifi_dur = 0, d1 = 0, d2 = 0;

	d1 = m_role1->policy.duration;
	d2 = m_role2->policy.duration;
	if (minfo->bt_info.bt_dur > BT_DUR_SEG_TH && BT_SEG_NUM > 1) {
		minfo->bt_info.bt_seg[0] = minfo->bt_info.bt_dur / 2;
		minfo->bt_info.bt_seg[1] = minfo->bt_info.bt_dur -
						minfo->bt_info.bt_seg[0];
		minfo->bt_info.bt_seg_num = 2;
		en_info->mcc_intvl = WORSECASE_INTVL;
		wifi_dur = en_info->mcc_intvl - minfo->bt_info.bt_dur;
		m_role1->policy.duration = (u8)(((d1 * 100 / (d1 + d2))
						* wifi_dur) / 100);
		m_role2->policy.duration = (u8)(wifi_dur -
						m_role1->policy.duration);
		PHL_TRACE(COMP_PHL_MCC, _PHL_WARNING_, "_mcc_discision_dur_for_2g_mcc_2role_bt(): Change D1(%d), D2(%d) to D1(%d), D2(%d), bt_seg[0](%d), bt_seg[1](%d)\n",
			d1, d2, m_role1->policy.duration,
			m_role2->policy.duration,
			minfo->bt_info.bt_seg[0], minfo->bt_info.bt_seg[1]);
	} else {
		/*todo*/
	}
}

bool _mcc_discision_duration_for_2role_bt(struct phl_mcc_info *minfo)
{
	struct rtw_phl_mcc_en_info *en_info = &minfo->en_info;
	struct rtw_phl_mcc_role *m_role1 = &en_info->mcc_role[0];
	struct rtw_phl_mcc_role *m_role2 = &en_info->mcc_role[1];
	bool add_extra_bt_role = false;

	if (minfo->bt_info.bt_dur == 0)
		goto exit;
	if (m_role1->chandef->band == BAND_ON_24G &&
		m_role2->chandef->band == BAND_ON_24G) {
		_mcc_discision_dur_for_2g_mcc_2role_bt(minfo);
		add_extra_bt_role = true;
		goto exit;
	}
	if (m_role1->chandef->band != BAND_ON_24G &&
		m_role2->chandef->band != BAND_ON_24G) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_discision_duration_for_2role_bt(): All 5G, Don't care BT duration\n");
		goto exit;
	}
	if (m_role1->chandef->band == BAND_ON_24G)
		_mcc_adjust_dur_for_2_band_mcc_2role_bt(minfo, m_role1, m_role2);
	else
		_mcc_adjust_dur_for_2_band_mcc_2role_bt(minfo, m_role2, m_role1);

exit:
	return add_extra_bt_role;
}

enum rtw_phl_status _mcc_calculate_start_tsf(struct phl_info_t *phl,
					struct rtw_phl_mcc_en_info *en_info
)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_mcc_role *ref_role = get_ref_role(en_info);
	u32 tsf_h = 0, tsf_l = 0;
	u64 tsf = 0, start_tsf = 0;
	u8 i = 0, max_loop = 10, calc_done = 0;

	if (RTW_HAL_STATUS_SUCCESS != rtw_hal_get_tsf(phl->hal,
				ref_role->wrole->hw_port, &tsf_h, &tsf_l)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_WARNING_, "_mcc_calculate_start_tsf(): Get tsf fail\n");
		goto exit;
	}
	tsf = tsf_h;
	tsf = tsf << 32;
	tsf |= tsf_l;
	start_tsf = tsf - _os_modular64(tsf, ref_role->bcn_intvl * TU) -
		(en_info->m_pattern.tob_r * TU);
	for (i = 0; i < max_loop; i++) {
		if (start_tsf < (tsf + (MIN_TRIGGER_MCC_TIME * TU))) {
			start_tsf += (ref_role->bcn_intvl * TU);
		} else {
			calc_done = 1;
			break;
		}
	}
	if (!calc_done) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_calculate_start_tsf(): Calcculate start tsf fail, please check code flow\n");
		goto exit;
	}
	en_info->tsf_high = start_tsf >> 32;
	en_info->tsf_low = (u32)start_tsf;
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_calculate_start_tsf(): start_tsf(0x%08x %08x), cur_tsf(0x%08x %08x), ref_role->bcn_intvl(%d), ref_role->duration(%d)\n",
		(u32)(start_tsf >> 32), (u32)start_tsf, (u32)(tsf >> 32),
		(u32)tsf, ref_role->bcn_intvl, ref_role->policy.duration);
	status = RTW_PHL_STATUS_SUCCESS;
exit:
	return status;
}

void _mcc_set_2_clients_worsecase_pattern(struct rtw_phl_mcc_pattern *m_pattern,
					u8 dur_ref)
{
	m_pattern->toa_r = CLIENTS_WORSECASE_REF_TOA;
	m_pattern->tob_r = dur_ref - m_pattern->toa_r;

	PHL_TRACE(COMP_PHL_MCC, _PHL_WARNING_, "_mcc_set_2_clients_worsecase_pattern(): tob_r(%d), toa_r(%d)\n",
		m_pattern->tob_r, m_pattern->toa_r);
}

s16 _mcc_get_offset_for_2_clients_worsecase(s16 ref_dur, s16 ano_dur,
				u16 ref_bcn_intvl, s16 toa_ref, s16 tob_ano) 
{
	return toa_ref + tob_ano + ref_dur + ano_dur - (2 * ref_bcn_intvl);
}

void _mcc_get_offset_range_for_2_clients_worsecase(s16 ref_dur, s16 ano_dur,
				u16 ref_bcn_intvl, s16 *bcn_min, s16 *bcn_max)
{
	s16 min1 = 0, min2 = 0, max1 = 0, max2 = 0;
	min1 = _mcc_get_offset_for_2_clients_worsecase(ref_dur, ano_dur,
				ref_bcn_intvl, MIN_RX_BCN_T, EARLY_RX_BCN_T);
	max1 = _mcc_get_offset_for_2_clients_worsecase(ref_dur, ano_dur,
				ref_bcn_intvl, MIN_RX_BCN_T, ano_dur -
				MIN_RX_BCN_T);
	min2 = _mcc_get_offset_for_2_clients_worsecase(ref_dur, ano_dur,
				ref_bcn_intvl, ref_dur - EARLY_RX_BCN_T,
				EARLY_RX_BCN_T);
	max2 = _mcc_get_offset_for_2_clients_worsecase(ref_dur, ano_dur,
				ref_bcn_intvl, ref_dur -EARLY_RX_BCN_T,
				ano_dur - MIN_RX_BCN_T);
	if (min1 < min2)
		*bcn_min = min1;
	else
		*bcn_min = min2;
	if (max1 > max2)
		*bcn_max = max1;
	else
		*bcn_max = max2;
}

enum rtw_phl_status _mcc_calc_2_clients_worsecase_pattern(u8 ref_dur,
				u8 ano_dur, u16 offset, u16 ref_bcn_intvl,
				struct rtw_phl_mcc_pattern *m_pattern)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	u16 mcc_intvl = ref_dur + ano_dur;
	s16 tob_r = 0, toa_r = 0, tob_a = 0, toa_a = 0;
	s16 d_r = ref_dur, d_a = ano_dur, bcns_offset = offset;
	s16 sum = 0, sum_last = 0, offset_min = 0, offset_max = 0;

	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_calculate_2_clients_worsecase_pattern(): ref_dur(%d), ano_dur(%d), bcns offset(%d), ref_bcn_intvl(%d)\n",
		ref_dur, ano_dur, offset, ref_bcn_intvl);
	if (ref_bcn_intvl != HANDLE_BCN_INTVL) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_WARNING_, "_mcc_calculate_2_clients_worsecase_pattern(): ref_bcn_intvl(%d) != HANDLE_BCN_INTVL(%d), now, we can't calculate the pattern\n",
			ref_bcn_intvl, HANDLE_BCN_INTVL);
		goto exit;
	}
	_mcc_get_offset_range_for_2_clients_worsecase(d_r, d_a, ref_bcn_intvl,
						&offset_min, &offset_max);
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_calculate_2_clients_worsecase_pattern(): we can calculate the range of bcn offset is %d~%d\n",
			offset_min, offset_max);
	if ((bcns_offset >= offset_min) && (bcns_offset <= offset_max))
		goto calc;
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_calculate_2_clients_worsecase_pattern(): transform bcn offset from %d to %d\n",
			bcns_offset, ref_bcn_intvl - bcns_offset);
	/*bcn offfset = 85, we can transform to -15*/
	bcns_offset = ref_bcn_intvl - bcns_offset;
	if (bcns_offset >= offset_min && offset_min <=offset_max) {
		goto calc;
	} else {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_calculate_2_clients_worsecase_pattern(): bcn offset out of range, we can't calculate it\n");
		goto exit;
	}
calc:
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_calculate_2_clients_worsecase_pattern(): Start calculate\n");
	for (tob_r = EARLY_RX_BCN_T; tob_r < mcc_intvl; tob_r++) {
		toa_r = d_r - tob_r;
		if (toa_r < MIN_RX_BCN_T) {
			PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_calculate_2_clients_worsecase_pattern(): Find the optimal pattern, by toa_r(%d) < MIN_RX_BCN_T(%d)\n",
				toa_r, MIN_RX_BCN_T);
			break;
		}
		tob_a = bcns_offset + 2 * ref_bcn_intvl - toa_r - mcc_intvl;
		if (tob_a < EARLY_RX_BCN_T)
			continue;
		toa_a = d_a - tob_a;
		if (toa_a < MIN_RX_BCN_T){
			PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_calculate_2_clients_worsecase_pattern(): Find the optimal pattern, by toa_a(%d) < MIN_RX_BCN_T(%d)\n",
				toa_a, MIN_RX_BCN_T);
			break;
		}
		sum = ((tob_r - toa_r) * (tob_r - toa_r)) +
			((tob_r - tob_a) * (tob_r - tob_a)) +
			((tob_r - toa_a) * (tob_r - toa_a)) +
			((toa_r - tob_a) * (toa_r - tob_a)) +
			((toa_r - toa_a) * (toa_r - toa_a)) +
			((tob_a - toa_a) * (tob_a - toa_a));
		if (sum_last !=0 && sum > sum_last) {
			PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_calculate_2_clients_worsecase_pattern(): Find the optimal pattern, by get minSum\n");
			break;
		}
		PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_calculate_2_clients_worsecase_pattern(): tob_r(%d), toa_r(%d), tob_a(%d), toa_a(%d), sum_last(%d), sum(%d)\n",
			tob_r, toa_r, tob_a, toa_a, sum_last, sum);
		sum_last = sum;

	}
	tob_r = tob_r - 1;
	toa_r = d_r - tob_r;
	tob_a = bcns_offset + 2 * ref_bcn_intvl - toa_r - mcc_intvl;
	toa_a = d_a - tob_a;
	m_pattern->tob_r = (u8)tob_r;
	m_pattern->toa_r = (u8)toa_r;
	m_pattern->tob_a = (u8)tob_a;
	m_pattern->toa_a = (u8)toa_a;
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_calculate_2_clients_worsecase_pattern(): Result, tob_r(%d), toa_r(%d), tob_a(%d), toa_a(%d)\n",
			tob_r, toa_r, tob_a, toa_a);
	status = RTW_PHL_STATUS_SUCCESS;
exit:
	if (status != RTW_PHL_STATUS_SUCCESS)
		m_pattern->calc_fail = true;
	return status;
}

/*
 * Calculate the optimal pattern for client+client MCC
 * @ref_dur: Duration of reference ch
 * @ano_dur: Duration of another ch
 * @offset: The offset between beacon of client1 and beacon of client2
 * @m_pattern: mcc pattern.
 * <tob_r> Bcn_r <toa_r>			         <tob_r> Bcn_r <toa_r>
 *			<tob_a> Bcn_a <toa_a>			     <tob_a> Bcn_a <toa_a>
 *	         <       bcns_offset       >
 */
void _mcc_calculate_2_clients_pattern(u8 ref_dur, u8 ano_dur, u16 offset,
					struct rtw_phl_mcc_pattern *m_pattern)
{
	u16 mcc_intvl = ref_dur + ano_dur;
	s16 tob_r = 0, toa_r = 0, tob_a = 0, toa_a = 0;
	s16 d_r = ref_dur, d_a = ano_dur, bcns_offset = offset;
	s16 sum = 0, sum_last = 0;

	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_calculate_2_clients_pattern(): ref_dur(%d), ano_dur(%d), bcns offset(%d)\n",
		ref_dur, ano_dur, offset);
	for (tob_r = EARLY_RX_BCN_T; tob_r < mcc_intvl; tob_r++) {
		toa_r = d_r - tob_r;
		if (toa_r < MIN_RX_BCN_T) {
			PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_calculate_2_clients_pattern(): Find the optimal pattern, by toa_r(%d) < MIN_RX_BCN_T(%d)\n",
				toa_r, MIN_RX_BCN_T);
			break;
		}
		tob_a = bcns_offset - toa_r;
		if (tob_a < EARLY_RX_BCN_T)
			continue;
		toa_a = d_a - tob_a;
		if (toa_a < MIN_RX_BCN_T){
			PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_calculate_2_clients_pattern(): Find the optimal pattern, by toa_a(%d) < MIN_RX_BCN_T(%d)\n",
				toa_a, MIN_RX_BCN_T);
			break;
		}
		sum = ((tob_r - toa_r) * (tob_r - toa_r)) +
			((tob_r - tob_a) * (tob_r - tob_a)) +
			((tob_r - toa_a) * (tob_r - toa_a)) +
			((toa_r - tob_a) * (toa_r - tob_a)) +
			((toa_r - toa_a) * (toa_r - toa_a)) +
			((tob_a - toa_a) * (tob_a - toa_a));
		if (sum_last !=0 && sum > sum_last) {
			PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_calculate_2_clients_pattern(): Find the optimal pattern, by get minSum\n");
			break;
		}
		PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_calculate_2_clients_pattern(): tob_r(%d), toa_r(%d), tob_a(%d), toa_a(%d), sum_last(%d), sum(%d)\n",
			tob_r, toa_r, tob_a, toa_a, sum_last, sum);
		sum_last = sum;
	}
	tob_r = tob_r - 1;
	toa_r = d_r - tob_r;
	tob_a = bcns_offset - toa_r;
	toa_a = d_a - tob_a;
	m_pattern->tob_r = (u8)tob_r;
	m_pattern->toa_r = (u8)toa_r;
	m_pattern->tob_a = (u8)tob_a;
	m_pattern->toa_a = (u8)toa_a;
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_calculate_2_clients_pattern(): tob_r(%d), toa_r(%d), tob_a(%d), toa_a(%d)\n",
			tob_r, toa_r, tob_a, toa_a);
}

void _mcc_fill_2_wrole_bt_pattern(struct phl_mcc_info *minfo,
	struct rtw_phl_mcc_role *role_ref, struct rtw_phl_mcc_role *role_ano)
{
	struct rtw_phl_mcc_en_info *en_info = &minfo->en_info;
	struct rtw_phl_mcc_pattern *m_pattern = &en_info->m_pattern;
	u16 dur_ref = role_ref->policy.duration;
	u16 dur_ano = role_ano->policy.duration;

	if (minfo->bt_info.bt_seg_num == 2) {
		minfo->bt_info.add_bt_role = true;
		dur_ref += minfo->bt_info.bt_seg[0];
		dur_ano += minfo->bt_info.bt_seg[1];
		if (RTW_PHL_STATUS_SUCCESS !=
				_mcc_calc_2_clients_worsecase_pattern(
						(u8)dur_ref, (u8)dur_ano,
						m_pattern->bcns_offset,
						role_ref->bcn_intvl,
						m_pattern)) {
			_mcc_set_2_clients_worsecase_pattern(m_pattern, (u8)dur_ref);
			PHL_TRACE(COMP_PHL_MCC, _PHL_WARNING_, "_mcc_fill_2_wrole_bt_pattern(): _mcc_calc_2_clients_worsecase_pattern fail, we use default worsecase pattern\n");
		}
		dur_ref = role_ref->policy.duration;
		dur_ano = role_ano->policy.duration;
		if (m_pattern->tob_r > (dur_ref - MIN_RX_BCN_T)) {
			role_ref->policy.protect_bcn = true;
			PHL_TRACE(COMP_PHL_MCC, _PHL_WARNING_, "_mcc_fill_2_wrole_bt_pattern(): bcn will in bt dur, we set protect_bcn = true for role_ref\n");
		}
		if (m_pattern->tob_a > (dur_ano - MIN_RX_BCN_T)) {
			role_ano->policy.protect_bcn = true;
			PHL_TRACE(COMP_PHL_MCC, _PHL_WARNING_, "_mcc_fill_2_wrole_bt_pattern(): bcn will in bt dur, we set protect_bcn = true for role_ano\n");
		}
	} else if (minfo->bt_info.bt_seg_num == 1){
		/*todo*/
	} else {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_fill_2_wrole_bt_pattern(): error bt_seg_num(%d)\n",
			minfo->bt_info.bt_seg_num);
	}
}

void _mcc_fill_2_clients_pattern(struct phl_mcc_info *minfo, u8 worsecase,
	struct rtw_phl_mcc_role *role_ref, struct rtw_phl_mcc_role *role_ano)
{
	struct rtw_phl_mcc_en_info *en_info = &minfo->en_info;

	if (!worsecase) {
		_mcc_calculate_2_clients_pattern(role_ref->policy.duration,
						role_ano->policy.duration,
						en_info->m_pattern.bcns_offset,
						&en_info->m_pattern);
		goto exit;
	}
	if (RTW_PHL_STATUS_SUCCESS != _mcc_calc_2_clients_worsecase_pattern(
					role_ref->policy.duration,
					role_ano->policy.duration,
					en_info->m_pattern.bcns_offset,
					role_ref->bcn_intvl,
					&en_info->m_pattern)) {
		_mcc_set_2_clients_worsecase_pattern(&en_info->m_pattern,
						role_ref->policy.duration);
		PHL_TRACE(COMP_PHL_MCC, _PHL_WARNING_, "_mcc_fill_info_for_2_clients_mode(): _mcc_calc_2_clients_worsecase_pattern fail, we use default worsecase pattern\n");
	}
exit:
	return;
}

enum rtw_phl_status _mcc_get_2_clients_bcn_offset(struct phl_info_t *phl,
			u16 *offset, struct rtw_phl_mcc_role *role_ref,
			struct rtw_phl_mcc_role *role_ano)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	u32 tsf_ref_h = 0, tsf_ref_l = 0, tsf_ano_h = 0, tsf_ano_l = 0;
	u64 tsf_ref = 0, tsf_ano = 0, mod_ref = 0, mod_ano = 0;

	if (RTW_HAL_STATUS_SUCCESS != rtw_hal_mcc_get_2ports_tsf(phl->hal,
			role_ref->group, role_ref->macid, role_ano->macid,
			&tsf_ref_h, &tsf_ref_l, &tsf_ano_h, &tsf_ano_l)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_get_2_clients_bcn_offset(): Get tsf failed\n");
		goto exit;
	}
	tsf_ref = tsf_ref_h;
	tsf_ref = tsf_ref << 32;
	tsf_ref |= tsf_ref_l;
	tsf_ano = tsf_ano_h;
	tsf_ano = tsf_ano << 32;
	tsf_ano |= tsf_ano_l;
	/*todo get tbtt offset*/
	mod_ref = _os_modular64(tsf_ref, role_ref->bcn_intvl * TU);
	mod_ano = _os_modular64(tsf_ano, role_ano->bcn_intvl * TU);
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_get_2_clients_bcn_offset(): tsf_ref: 0x%08X %08x modular(%d)\n",
		(u32)(tsf_ref >> 32), (u32)tsf_ref, (u32)mod_ref);
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_get_2_clients_bcn_offset(): tsf_ano: 0x%08x %08x modular(%d)\n",
		(u32)(tsf_ano >> 32), (u32)tsf_ano, (u32)mod_ano);
	if (mod_ref < mod_ano)
		mod_ref += role_ref->bcn_intvl * TU;
	*offset = (u16)((mod_ref - mod_ano) / TU);
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_get_2_clients_bcn_offset(): bcn offset(%d)\n",
		*offset);
	status = RTW_PHL_STATUS_SUCCESS;
exit:
	return status;
}

void _mcc_calculate_ap_client_pattern(struct phl_mcc_info *minfo,
					u16 *bcns_offet)
{
	struct rtw_phl_mcc_en_info *en_info = &minfo->en_info;
	struct rtw_phl_mcc_pattern *m_pattern = &en_info->m_pattern;
	struct rtw_phl_mcc_role *role_ref = get_ref_role(en_info);
	struct rtw_phl_mcc_role *role_ano = (role_ref == &en_info->mcc_role[0])
				? &en_info->mcc_role[1] : &en_info->mcc_role[0];

	*bcns_offet = (u8)(en_info->mcc_intvl / 2);
	m_pattern->toa_r= role_ref->policy.duration / 2;
	m_pattern->tob_r = role_ref->policy.duration - m_pattern->toa_r;
	m_pattern->tob_a = (u8)(*bcns_offet - m_pattern->toa_r);
	m_pattern->toa_a = role_ano->policy.duration - m_pattern->tob_a;
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_calculate_ap_client_pattern(): tob_r(%d), toa_r(%d), tob_a(%d), toa_a(%d)\n",
		m_pattern->tob_r, m_pattern->toa_r, m_pattern->tob_a, m_pattern->toa_a);
}

void _mcc_set_dur_for_2_clients_mode(u8 *dur1, u8 *dur2, u16 *mcc_intvl,
					bool worsecase)
{
	if (*dur1 == MCC_DUR_NONSPECIFIC) {
		*dur1 = (u8)(*mcc_intvl - (*dur2));
	} else {
		*dur2 = (u8)(*mcc_intvl - (*dur1));
	}
	if (*dur1 < MIN_CLIENT_DUR) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_WARNING_, "_mcc_set_dur_for_2_clients_mode(): Core specific unsuitable duration, we adjust the dur1(%d) and dur2(%d) to dur1(%d) and dur2(%d)\n",
			*dur1, *dur2, MIN_CLIENT_DUR, (*mcc_intvl - (*dur1)));
		*dur1 = MIN_CLIENT_DUR;
		*dur2 = (u8)(*mcc_intvl - (*dur1));
	} else if (*dur2 < MIN_CLIENT_DUR) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_WARNING_, "_mcc_set_dur_for_2_clients_mode(): Core specific unsuitable duration, we adjust the dur1(%d) and dur2(%d) to dur1(%d) and dur2(%d)\n",
			*dur1, *dur2, (*mcc_intvl - MIN_CLIENT_DUR), MIN_CLIENT_DUR);
		*dur2 = MIN_CLIENT_DUR;
		*dur1 = (u8)(*mcc_intvl - (*dur2));
	}
	if (worsecase) {
		/*need to get from core layer for worsecase??*/
		if (*dur1 > *dur2) {
			*mcc_intvl = WORSECASE_INTVL;
			*dur1 = CLIENTS_WORSECASE_LARGE_DUR;
			*dur2 = (u8)(*mcc_intvl - (*dur1));
		} else {
			*mcc_intvl = WORSECASE_INTVL;
			*dur2 = CLIENTS_WORSECASE_LARGE_DUR;
			*dur1 = (u8)(*mcc_intvl - (*dur2));
		}
	}
}

enum rtw_phl_status _mcc_fill_info_for_2_clients_mode(struct phl_info_t *phl,
						struct phl_mcc_info *minfo)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_mcc_en_info *en_info = &minfo->en_info;
	struct rtw_phl_mcc_role *role_ref = get_ref_role(en_info);
	struct rtw_phl_mcc_role *role_ano = (role_ref == &en_info->mcc_role[0])
				? &en_info->mcc_role[1] : &en_info->mcc_role[0];
	u16 bcns_offset = 0;
	bool worsecase = false;

	en_info->mcc_intvl = role_ref->bcn_intvl;
	if (RTW_PHL_STATUS_SUCCESS != _mcc_get_2_clients_bcn_offset(phl,
					&bcns_offset, role_ref, role_ano)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_fill_info_for_2_clients_mode(): Get bcn offset fail\n");
		goto exit;
	}
	en_info->m_pattern.bcns_offset = bcns_offset;
	if ((bcns_offset < MIN_BCNS_OFFSET) ||
		(bcns_offset > (role_ref->bcn_intvl - MIN_BCNS_OFFSET))) {
		worsecase = true;
		PHL_TRACE(COMP_PHL_MCC, _PHL_WARNING_, "_mcc_fill_info_for_2_clients_mode(): worsecase, bcns_offset(%d) < %d or bcns_offset > %d\n",
			bcns_offset, MIN_BCNS_OFFSET,
			(role_ref->bcn_intvl - MIN_BCNS_OFFSET));
	}
	_mcc_set_dur_for_2_clients_mode(&role_ref->policy.duration,
					&role_ano->policy.duration,
					&en_info->mcc_intvl, worsecase);
	if (_mcc_discision_duration_for_2role_bt(minfo)) {
		_mcc_fill_2_wrole_bt_pattern(minfo, role_ref, role_ano);
	} else {
		_mcc_fill_2_clients_pattern(minfo, worsecase, role_ref, role_ano);
	}
	if (RTW_PHL_STATUS_SUCCESS != _mcc_calculate_start_tsf(phl, en_info)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_fill_info_for_2_clients_mode(): calculate start tsf fail\n");
		goto exit;
	}
	status = RTW_PHL_STATUS_SUCCESS;
exit:
	return status;
}

void _mcc_set_dur_for_ap_client_mode(u8 *ap_dur, u8 *client_dur, u16 mcc_intvl)
{
	if (*ap_dur == MCC_DUR_NONSPECIFIC)
		*ap_dur = (u8)(mcc_intvl - *client_dur);
	if (*ap_dur < MIN_AP_DUR) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_WARNING_, "_mcc_set_dur_for_ap_client_mode(): ap_dur(%d) < MIN_AP_DUR(%d), set ap_dur = MIN_AP_DUR\n",
			*ap_dur, MIN_AP_DUR);
		*ap_dur = MIN_AP_DUR;
	} else if (*ap_dur > (u8)(mcc_intvl - MIN_CLIENT_DUR)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_WARNING_, "_mcc_set_dur_for_ap_client_mode(): ap_dur(%d) < MAX_AP_DUR(%d), set ap_dur = MAX_AP_DUR\n",
			*ap_dur, (mcc_intvl - MIN_CLIENT_DUR));
		*ap_dur = (u8)(mcc_intvl - MIN_CLIENT_DUR);
	}
	*client_dur = (u8)(mcc_intvl - *ap_dur);
}

enum rtw_phl_status _mcc_fill_info_for_ap_client_mode(
			struct phl_info_t *phl, struct phl_mcc_info *minfo)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_mcc_en_info *en_info = &minfo->en_info;
	struct rtw_phl_mcc_sync_tsf_info *sync_info = &en_info->sync_tsf_info;
	struct rtw_phl_mcc_role *ap_role = NULL;
	struct rtw_phl_mcc_role *client_role = NULL;
	struct rtw_phl_mcc_role *role_ref = get_ref_role(en_info);
	struct rtw_phl_mcc_role *role_ano = (role_ref == &en_info->mcc_role[0])
				? &en_info->mcc_role[1] : &en_info->mcc_role[0];

	ap_role = _mcc_get_mrole_by_category(en_info, MCC_ROLE_AP_CAT);
	client_role = _mcc_get_mrole_by_category(en_info, MCC_ROLE_CLIENT_CAT);
	if (ap_role == NULL || client_role == NULL) {
		_mcc_dump_en_info(en_info);
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_fill_info_for_ap_client_mode(): (ap_role == NULL || client_role == NULL)\n");
		goto exit;
	}
	en_info->mcc_intvl = ap_role->bcn_intvl;
	_mcc_set_dur_for_ap_client_mode(&ap_role->policy.duration,
					&client_role->policy.duration,
					en_info->mcc_intvl);
	if (_mcc_discision_duration_for_2role_bt(minfo)) {
		en_info->m_pattern.bcns_offset = AP_CLIENT_OFFSET;
		_mcc_fill_2_wrole_bt_pattern(minfo, role_ref, role_ano);
	} else {
		_mcc_calculate_ap_client_pattern(minfo,
					&en_info->m_pattern.bcns_offset);
	}
	sync_info->offset = en_info->m_pattern.bcns_offset;
	sync_info->source = client_role->macid;
	sync_info->target = ap_role->macid;
	sync_info->sync_en = true;
	if (RTW_HAL_STATUS_SUCCESS != rtw_hal_tsf_sync(phl->hal,
			client_role->wrole->hw_port, ap_role->wrole->hw_port,
			ap_role->wrole->hw_band, sync_info->offset,
			HAL_TSF_SYNC_NOW_ONCE)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_fill_info_for_ap_client_mode(): Sync tsf fail\n");
		goto exit;
	}
	if (RTW_PHL_STATUS_SUCCESS != _mcc_calculate_start_tsf(phl, en_info)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_fill_info_for_ap_client_mode(): calculate start tsf fail\n");
		goto exit;
	}
	status = RTW_PHL_STATUS_SUCCESS;
exit:
	return status;
}

enum rtw_phl_status _mcc_pkt_offload_for_client(struct phl_info_t *phl, u16 macid)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_stainfo_t *phl_sta = NULL;
	struct rtw_pkt_ofld_null_info null_info = {0};
	void *d = phl_to_drvpriv(phl);
	u32 null_token = 0;

	phl_sta = rtw_phl_get_stainfo_by_macid(phl, macid);
	if (phl_sta == NULL) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_pkt_offload_for_client(): get sta fail, macid(%d)\n",
			macid);
		goto exit;
	}
	if (NOT_USED != phl_pkt_ofld_get_id(phl, macid,
						PKT_TYPE_NULL_DATA)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_pkt_offload_for_client(): macid(%d), we had already offload NULL Pkt\n",
			macid);
		status = RTW_PHL_STATUS_SUCCESS;
		goto exit;
	}
	_os_mem_cpy(d, &(null_info.a1[0]), &(phl_sta->mac_addr[0]),
		MAC_ADDRESS_LENGTH);
	_os_mem_cpy(d,&(null_info.a2[0]), &(phl_sta->wrole->mac_addr[0]),
		MAC_ADDRESS_LENGTH);
	_os_mem_cpy(d, &(null_info.a3[0]), &(phl_sta->mac_addr[0]),
		MAC_ADDRESS_LENGTH);
	if (RTW_PHL_STATUS_SUCCESS != phl_pkt_ofld_request(phl, macid,
						PKT_TYPE_NULL_DATA, &null_token,
						__func__, &null_info)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_pkt_offload_for_client(): Pkt offload fail, macid(%d)\n",
			macid);
		goto exit;
	}
	PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_pkt_offload_for_client(): offload ok, macid(%d), null_token(%d)\n",
		macid, null_token);
	status = RTW_PHL_STATUS_SUCCESS;
exit:
	return status;
}


enum rtw_phl_status _mcc_pkt_offload(struct phl_info_t *phl,
					struct rtw_phl_mcc_en_info *info)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_mcc_role *mcc_role = NULL;
	u8 midx = 0;

	for (midx = 0; midx < MCC_ROLE_NUM; midx++) {
		if (!(info->role_map & BIT(midx)))
			continue;
		mcc_role = &info->mcc_role[midx];
		if (_mcc_is_client_category(mcc_role->wrole)) {
			if (RTW_PHL_STATUS_SUCCESS !=
				_mcc_pkt_offload_for_client(phl,
							mcc_role->macid)) {
				PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_pkt_offload_for_client(): mcc_role index(%d)\n",
					midx);
				goto exit;
			}
		}
	}
	status = RTW_PHL_STATUS_SUCCESS;
exit:
	return status;
}

enum rtw_phl_status _mcc_update_2_clients_pattern(struct phl_info_t *phl,
				struct phl_mcc_info *minfo)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_mcc_en_info *en_info = &minfo->en_info;

	if (RTW_PHL_STATUS_SUCCESS != _mcc_fill_info_for_2_clients_mode(phl,
								minfo)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_update_2_clients_pattern(): fill info fail for 2clients mode\n");
		goto error;
	}
	if (RTW_HAL_STATUS_SUCCESS != rtw_hal_mcc_set_duration(phl->hal, en_info)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_update_2_clients_pattern(): set duration fail\n");
		goto error;
	}
	status = RTW_PHL_STATUS_SUCCESS;
	goto exit;
error:
	PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_update_2_clients_pattern(): Update fail\n");
exit:
	return status;
}

enum rtw_phl_status _mcc_update_ap_client_pattern(struct phl_info_t *phl,
				struct phl_mcc_info *minfo, u16 ori_offset)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_mcc_en_info *en_info = &minfo->en_info;

	if (RTW_PHL_STATUS_SUCCESS != _mcc_fill_info_for_ap_client_mode(phl,
								minfo)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_update_ap_client_pattern(): fill info fail for ap client mode\n");
		goto error;
	}
	if (RTW_HAL_STATUS_SUCCESS != rtw_hal_mcc_set_duration(phl->hal, en_info)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_update_ap_client_pattern(): set duration fail\n");
		goto error;
	}
	if ((en_info->sync_tsf_info.sync_en) &&
		(en_info->sync_tsf_info.offset != ori_offset)) {
		if (RTW_HAL_STATUS_SUCCESS != rtw_hal_mcc_sync_enable(phl->hal,
								en_info)) {
			PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_update_ap_client_pattern(): set tsf sync fail\n");
			goto error;
		}
	}
	_mcc_up_noa(phl, minfo);
	status = RTW_PHL_STATUS_SUCCESS;
	goto exit;
error:
	PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_update_ap_client_pattern(): Update fail\n");
exit:
	return status;
}

enum rtw_phl_status _mcc_duration_change(struct phl_info_t *phl,
		struct phl_mcc_info *minfo, struct phl_mcc_info *new_minfo)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;

	if (RTW_PHL_MCC_AP_CLIENT_MODE == minfo->mcc_mode) {
		if (RTW_PHL_STATUS_SUCCESS != _mcc_update_ap_client_pattern(
					phl, new_minfo,
					minfo->en_info.sync_tsf_info.offset)) {
			PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_duration_change(): update ap_client fail\n");
			goto exit;
		}
	} else if (RTW_PHL_MCC_2CLIENTS_MODE == minfo->mcc_mode) {
		if (RTW_PHL_STATUS_SUCCESS != _mcc_update_2_clients_pattern(
							phl, new_minfo)) {
			PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_duration_change(): update ap_client fail\n");
			goto exit;
		}
	} else {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_duration_change(): Undefined mcc_mode(%d)\n",
			minfo->mcc_mode);
		goto exit;
	}
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_duration_change(): Update success\n");
	_os_mem_cpy(phl_to_drvpriv(phl), minfo, new_minfo,
			sizeof(struct phl_mcc_info));
	status = RTW_PHL_STATUS_SUCCESS;
exit:
	return status;
}

void _mcc_2_clients_tracking(struct phl_info_t *phl,
				struct phl_mcc_info *minfo
)
{
	struct rtw_phl_mcc_en_info *en_info = &minfo->en_info;
	struct rtw_phl_mcc_pattern *m_pattern = &en_info->m_pattern;
	struct rtw_phl_mcc_role *role_ref = get_ref_role(en_info);
	struct rtw_phl_mcc_role *role_ano = (role_ref == &en_info->mcc_role[0])
				? &en_info->mcc_role[1] : &en_info->mcc_role[0];
	struct phl_mcc_info new_minfo = {0};
	u16 bcns_offset = 0, diff = 0, tol = 0;/*tolerance*/
	bool negative_sign = false;

	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, ">>> _mcc_2_clients_tracking\n");
	_os_mem_cpy(phl_to_drvpriv(phl), &new_minfo, minfo,
			sizeof(struct phl_mcc_info));
	if (RTW_PHL_STATUS_SUCCESS != _mcc_get_2_clients_bcn_offset(phl,
					&bcns_offset, role_ref, role_ano)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_2_clients_tracking(): Get bcn offset fail\n");
		goto exit;
	}
	if (bcns_offset > m_pattern->bcns_offset) {
		diff = bcns_offset - m_pattern->bcns_offset;
	} else {
		diff = m_pattern->bcns_offset - bcns_offset;
		negative_sign = true;
	}
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_2_clients_tracking(): old bcns_offset(%d), new bcns_offset(%d)\n",
		m_pattern->bcns_offset, bcns_offset);
	_mcc_dump_pattern(&en_info->m_pattern);
	_mcc_dump_ref_role_info(en_info);
	if (en_info->mcc_intvl == WORSECASE_INTVL) {
		tol = CLIENTS_TRACKING_WORSECASE_TH;
		goto decision;
	}
	if (negative_sign) {
		if (m_pattern->tob_a <= EARLY_RX_BCN_T) {
			tol = CLIENTS_TRACKING_CRITICAL_POINT_TH;
		} else if (m_pattern->tob_a >= (2 * EARLY_RX_BCN_T)){
			tol = m_pattern->tob_a - ((3 * EARLY_RX_BCN_T) / 2);
		} else {
			tol = CLIENTS_TRACKING_TH;
		}
	} else {
		if (m_pattern->toa_a <= MIN_RX_BCN_T) {
			tol = CLIENTS_TRACKING_CRITICAL_POINT_TH;
		} else if (m_pattern->toa_a >= (2 * MIN_RX_BCN_T)){
			tol = m_pattern->toa_a - ((3 * MIN_RX_BCN_T) / 2);
		} else {
			tol = CLIENTS_TRACKING_TH;
		}
	}
decision:
	if (diff < tol)
		goto exit;
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "_mcc_2_clients_tracking(): Need to update new 2clients pattern, negative_sign(%d), diff(%d), tolerance(%d), mcc_intvl(%d)\n",
		negative_sign, diff, tol, en_info->mcc_intvl);
	/*get original wifi time slot*/
	_mcc_fill_mcc_role_policy_info(phl, role_ref->wrole, role_ref);
	_mcc_fill_mcc_role_policy_info(phl, role_ano->wrole, role_ano);
	if (RTW_PHL_STATUS_SUCCESS != _mcc_update_2_clients_pattern(phl,
								&new_minfo)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_2_clients_tracking(): update 2clients fail\n");
		goto exit;
	}
	PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_2_clients_tracking(): update new pattern ok\n");
	PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_2_clients_tracking(): old pattern:\n");
	_mcc_dump_pattern(&en_info->m_pattern);
	PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_2_clients_tracking(): new pattern:\n");
	_mcc_dump_pattern(&new_minfo.en_info.m_pattern);
	_os_mem_cpy(phl_to_drvpriv(phl), minfo, &new_minfo,
			sizeof(struct phl_mcc_info));
exit:
	return;
}

#ifdef RTW_WKARD_GO_BT_TS_ADJUST_VIA_NOA
enum rtw_phl_status _mcc_fill_info_for_go_bt_mode(
			struct phl_info_t *phl, struct phl_mcc_info *minfo)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_mcc_en_info *en_info = &minfo->en_info;
	struct rtw_phl_mcc_role *role_ref = get_ref_role(en_info);
	struct rtw_phl_mcc_pattern *m_pattern = &en_info->m_pattern;

	minfo->bt_info.add_bt_role = true;
	en_info->mcc_intvl = role_ref->bcn_intvl;
	role_ref->policy.duration = (u8)(en_info->mcc_intvl -
							minfo->bt_info.bt_dur);
	m_pattern->toa_r= role_ref->policy.duration / 2;
	m_pattern->tob_r = role_ref->policy.duration - m_pattern->toa_r;
	m_pattern->tob_a = (u8)(minfo->bt_info.bt_dur / 2);
	m_pattern->toa_a = (u8)(minfo->bt_info.bt_dur - m_pattern->tob_a);

	if (RTW_PHL_STATUS_SUCCESS != _mcc_calculate_start_tsf(phl, en_info)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "_mcc_fill_info_for_go_bt_mode(): calculate start tsf fail\n");
		goto exit;
	}
	status = RTW_PHL_STATUS_SUCCESS;
exit:
	return status;
}

enum rtw_phl_status rtw_phl_mcc_go_bt_coex_enable(struct phl_info_t *phl,
				struct rtw_wifi_role_t *cur_role, u16 bt_slot)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct phl_mcc_info *minfo = NULL;
	struct rtw_phl_mcc_en_info *en_info = NULL;
	struct hw_band_ctl_t *band_ctrl = get_band_ctrl(phl, cur_role->hw_band);

	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, ">>> rtw_phl_mcc_go_bt_coex_enable(): cur_role->type(%d) bt_slot(%d)\n",
		cur_role->type, bt_slot);
	if (!is_mcc_init(phl)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_go_bt_coex_enable(): mcc is not init, please check code\n");
		goto exit;
	}
	minfo = get_mcc_info(phl, cur_role->hw_band);
	en_info = &minfo->en_info;
	if (MCC_RUNING == minfo->state || MCC_FW_DIS_FAIL  == minfo->state ||
		MCC_TRIGGER_FW_DIS == minfo->state ||
		MCC_TRIGGER_FW_EN == minfo->state) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_go_bt_coex_enable(): MCC_RUNING == minfo->state || MCC_FW_DIS_FAIL  == minfo->state, please check code flow\n");
		_mcc_dump_state(&minfo->state);
		goto exit;
	}
	_os_mem_set(phl_to_drvpriv(phl), en_info, 0,
					sizeof(struct rtw_phl_mcc_en_info));
	if (RTW_PHL_STATUS_SUCCESS != _mcc_fill_role_info(phl, en_info,
						BIT(cur_role->id), cur_role)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_go_bt_coex_enable(): fill role info failed\n");
		goto exit;
	}
	minfo->bt_info.bt_dur = bt_slot;
	minfo->bt_info.bt_seg_num = 1;
	minfo->bt_info.bt_seg[0] = minfo->bt_info.bt_dur;
	if (RTW_PHL_STATUS_SUCCESS != _mcc_get_mrole_idx_by_wrole(minfo,
					cur_role, &en_info->ref_role_idx)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_go_bt_coex_enable(): fill ref_role idx failed\n");
		goto exit;
	}
	if (RTW_PHL_STATUS_SUCCESS != _mcc_fill_info_for_go_bt_mode(phl, minfo)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_go_bt_coex_enable(): fill ref_role idx failed\n");
		goto exit;
	}
	_mcc_set_state(minfo, MCC_TRIGGER_FW_EN);
	if (rtw_hal_mcc_enable(phl->hal, en_info, &minfo->bt_info) !=
						RTW_HAL_STATUS_SUCCESS) {
		_mcc_set_state(minfo, MCC_FW_EN_FAIL);
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_go_bt_coex_enable(): Enable FW mcc Fail\n");
		goto exit;
	}
	_mcc_set_state(minfo, MCC_RUNING);
	PHL_TRACE(COMP_PHL_MCC, _PHL_ALWAYS_, "rtw_phl_mcc_go_bt_coex_enable(): Enable FW mcc ok\n");
	_mcc_up_noa(phl, minfo);
	_mcc_dump_mcc_info(minfo);
	status = RTW_PHL_STATUS_SUCCESS;
exit:
	PHL_TRACE(COMP_PHL_MCC, _PHL_ALWAYS_, "<<< rtw_phl_mcc_go_bt_coex_enable():status(%d)\n",
		status);
	return status;
}

enum rtw_phl_status rtw_phl_mcc_go_bt_coex_disable(struct phl_info_t *phl,
				struct rtw_wifi_role_t *spec_role)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct phl_mcc_info *minfo = NULL;
	struct rtw_phl_mcc_en_info *en_info = NULL;
	struct rtw_phl_mcc_role *m_role = NULL;

	if (!is_mcc_init(phl)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_go_bt_coex_disable(): mcc is not init, please check code\n");
		goto exit;
	}
	minfo = get_mcc_info(phl, spec_role->hw_band);
	en_info = &minfo->en_info;
	if (MCC_RUNING != minfo->state) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_go_bt_coex_disable(): MCC_RUNING != m_info->state, please check code flow\n");
		_mcc_dump_state(&minfo->state);
		goto exit;
	}
	if (NULL == (m_role = _mcc_get_mrole_by_wrole(minfo, spec_role))) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_go_bt_coex_disable(): Can't get mrole, wrole id(%d), please check code flow\n",
			spec_role->id);
		goto exit;
	}
	_mcc_set_state(minfo, MCC_TRIGGER_FW_DIS);
	if (rtw_hal_mcc_disable(phl->hal, m_role->group, m_role->macid)
						!= RTW_HAL_STATUS_SUCCESS) {
		status = RTW_PHL_STATUS_FAILURE;
		_mcc_set_state(minfo, MCC_FW_DIS_FAIL);
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_,"rtw_phl_mcc_go_bt_coex_disable(): Disable FW mcc Fail\n");
		goto exit;
	}
	rtw_hal_sync_cur_ch(phl->hal, spec_role->hw_band, spec_role->chandef);
	_mcc_set_state(minfo, MCC_STOP);
	PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_go_bt_coex_disable(): Disable FW mcc ok\n");
	_mcc_clean_noa(phl, en_info);
	status = RTW_PHL_STATUS_SUCCESS;
exit:
	PHL_TRACE(COMP_PHL_MCC, _PHL_ALWAYS_, "<<< rtw_phl_mcc_go_bt_coex_disable(): status(%d)\n",
		status);
	return status;
}
#endif /*RTW_WKARD_GO_BT_TS_ADJUST_VIA_NOA*/

void rtw_phl_mcc_watchdog(struct phl_info_t *phl, u8 band_idx)
{
	struct phl_mcc_info *minfo = NULL;

	if (!is_mcc_init(phl)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_watchdog(): mcc is not init, please check code\n");
		goto exit;
	}
	minfo = get_mcc_info(phl, band_idx);
	if (MCC_RUNING != minfo->state)
		goto exit;

	if (RTW_PHL_MCC_2CLIENTS_MODE == minfo->mcc_mode)
		_mcc_2_clients_tracking(phl, minfo);

exit:
	return;
}

enum rtw_phl_status rtw_phl_mcc_duration_change(struct phl_info_t *phl,
				struct rtw_wifi_role_t *wrole, u8 duration)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct phl_mcc_info *minfo = NULL;
	struct phl_mcc_info new_minfo = {0};
	struct rtw_phl_mcc_role *spec_mrole = NULL;

	if (!is_mcc_init(phl)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_duration_change(): mcc is not init, please check code\n");
		goto exit;
	}
	minfo = get_mcc_info(phl, wrole->hw_band);
	if (MCC_RUNING != minfo->state) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_duration_change(): MCC_RUNING != minfo->state, , please check code flow\n");
		_mcc_dump_state(&minfo->state);
		goto exit;
	}
	_os_mem_cpy(phl_to_drvpriv(phl), &new_minfo, minfo,
			sizeof(struct phl_mcc_info));
	_mcc_set_unspecific_dur(&new_minfo);
	spec_mrole = _mcc_get_mrole_by_wrole(&new_minfo, wrole);
	if (NULL == spec_mrole) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_duration_change(): Can't get mrole by wrole(%d), please check code flow\n",
			wrole->id);
		goto exit;
	}
	spec_mrole->policy.duration = duration;
	if (RTW_PHL_STATUS_SUCCESS != _mcc_duration_change(phl, minfo,
								&new_minfo)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_duration_change(): Change fail\n");
		goto exit;
	}
	PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_duration_change(): Change success\n");
	status = RTW_PHL_STATUS_SUCCESS;
exit:
	return status;
}

enum rtw_phl_status rtw_phl_mcc_bt_duration_change(struct phl_info_t *phl,
						u8 dur, u8 band_idx)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct phl_mcc_info *minfo = NULL;
	struct rtw_phl_mcc_role *m_role1 = NULL;
	struct rtw_phl_mcc_role *m_role2 = NULL;
	struct phl_mcc_info new_minfo = {0};

	if (!is_mcc_init(phl)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_bt_duration_change(): mcc is not init, please check code\n");
		goto exit;
	}
	minfo = get_mcc_info(phl, band_idx);
	if (MCC_RUNING != minfo->state) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_bt_duration_change(): MCC_RUNING != minfo->state, , please check code flow\n");
		_mcc_dump_state(&minfo->state);
		goto exit;
	}
	_os_mem_cpy(phl_to_drvpriv(phl), &new_minfo, minfo,
			sizeof(struct phl_mcc_info));
	m_role1 = &new_minfo.en_info.mcc_role[0];
	m_role2 = &new_minfo.en_info.mcc_role[1];
	if (m_role1->chandef->band != BAND_ON_24G &&
		m_role2->chandef->band != BAND_ON_24G) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "rtw_phl_mcc_bt_duration_change(): All 5G, Don't care BT duration\n");
		goto exit;
	}
	new_minfo.bt_info.bt_dur = dur;
	/*get original wifi time slot*/
	_mcc_fill_mcc_role_policy_info(phl, m_role1->wrole, m_role1);
	_mcc_fill_mcc_role_policy_info(phl, m_role2->wrole, m_role2);
	if (RTW_PHL_STATUS_SUCCESS != _mcc_duration_change(phl, minfo,
								&new_minfo)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_bt_duration_change(): Change fail\n");
		goto exit;
	}
	PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_bt_duration_change(): Change success\n");
	status = RTW_PHL_STATUS_SUCCESS;
exit:
	return status;
}

void rtw_phl_mcc_client_link_notify_for_ap(struct phl_info_t *phl,
					struct rtw_phl_stainfo_t *sta)
{
	struct rtw_wifi_role_t *wrole = sta->wrole;
	struct phl_mcc_info *minfo = NULL;
	struct rtw_phl_mcc_role *mrole = NULL;

	if (!is_mcc_init(phl)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_client_link_notify_for_ap(): mcc is not init, please check code\n");
		goto exit;
	}
	minfo = get_mcc_info(phl, wrole->hw_band);
	if (MCC_RUNING != minfo->state) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_client_link_notify_for_ap(): MCC_RUNING != minfo->state\n");
		_mcc_dump_state(&minfo->state);
		goto exit;
	}
	if (NULL == (mrole = _mcc_get_mrole_by_wrole(minfo, wrole))) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_client_link_notify_for_ap(): Can't get mrole, wrole id(%d), please check code flow\n",
			wrole->id);
		goto exit;
	}
	_mcc_fill_macid_bitmap_by_role(phl, mrole);
	if (RTW_HAL_STATUS_SUCCESS != rtw_hal_mcc_update_macid_bitmap(
					phl->hal, mrole->group,
					mrole->macid, &mrole->used_macid)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_client_link_notify_for_ap(): Update macid map fail\n");
		goto exit;
	}
	PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_client_link_notify_for_ap(): Update macid map ok\n");
	_mcc_dump_mcc_info(minfo);
exit:
	return;
}

bool rtw_phl_mcc_inprogress(struct phl_info_t *phl, u8 band_idx)
{
	bool ret = false;
	struct phl_mcc_info *minfo = NULL;

	if (!is_mcc_init(phl)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_inprogress(): mcc is not init, please check code\n");
		goto exit;
	}
	minfo = get_mcc_info(phl, band_idx);
	if (MCC_TRIGGER_FW_EN == minfo->state || MCC_RUNING == minfo->state ||
		MCC_TRIGGER_FW_DIS == minfo->state ||
		MCC_FW_DIS_FAIL == minfo->state) {
		ret = true;
	}
exit:
	return ret;
}

/* Enable Fw MCC
 * @cur_role: the role in the current ch.
 */
enum rtw_phl_status rtw_phl_mcc_enable(struct phl_info_t *phl,
					struct rtw_wifi_role_t *cur_role)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct phl_mcc_info *minfo = NULL;
	struct rtw_phl_mcc_en_info *en_info = NULL;
	struct hw_band_ctl_t *band_ctrl = get_band_ctrl(phl, cur_role->hw_band);
	u8 role_map = 0;

	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, ">>> rtw_phl_mcc_enable(): cur_role->type(%d)\n",
		cur_role->type);
	if (!is_mcc_init(phl)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_enable(): mcc is not init, please check code\n");
		goto exit;
	}
	minfo = get_mcc_info(phl, cur_role->hw_band);
	en_info = &minfo->en_info;
	if (MCC_RUNING == minfo->state || MCC_FW_DIS_FAIL  == minfo->state) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_enable(): MCC_RUNING == minfo->state || MCC_FW_DIS_FAIL  == minfo->state, please check code flow\n");
		_mcc_dump_state(&minfo->state);
		goto exit;
	}
	if (RTW_PHL_STATUS_SUCCESS != _mcc_get_role_map(phl, band_ctrl,
							&role_map)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_enable(): Get role map failed\n");
		goto exit;
	}
	if (RTW_PHL_STATUS_SUCCESS != _mcc_transfer_mode(phl, minfo,
							role_map)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_enable(): transfer mcc mode failed\n");
		goto exit;
	}
	_os_mem_set(phl_to_drvpriv(phl), en_info, 0,
					sizeof(struct rtw_phl_mcc_en_info));
	if (RTW_PHL_STATUS_SUCCESS != _mcc_fill_role_info(phl, en_info,
							role_map, cur_role)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_enable(): fill role info failed\n");
		goto exit;
	}
	_mcc_fill_coex_mode(phl, minfo);
	_mcc_fill_bt_dur(phl, minfo);
	if (RTW_PHL_STATUS_SUCCESS != _mcc_get_mrole_idx_by_wrole(minfo,
					cur_role, &en_info->ref_role_idx)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_enable(): fill ref_role idx failed\n");
		goto exit;
	}
	if (minfo->mcc_mode == RTW_PHL_MCC_AP_CLIENT_MODE) {
		if (RTW_PHL_STATUS_SUCCESS !=
			_mcc_fill_info_for_ap_client_mode(phl, minfo)) {
			PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_enable(): fill info failed for ap_client mode\n");
			goto exit;
		}
	} else if (minfo->mcc_mode == RTW_PHL_MCC_2CLIENTS_MODE){
		if (RTW_PHL_STATUS_SUCCESS !=
			_mcc_fill_info_for_2_clients_mode(phl, minfo)) {
			PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_enable(): fill info failed for 2clients mode\n");
			goto exit;
		}
	} else {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_enable(): Undefined mcc_mode(%d)\n",
			minfo->mcc_mode);
		goto exit;
	}
	_mcc_dump_mcc_info(minfo);
	if (RTW_PHL_STATUS_SUCCESS != _mcc_pkt_offload(phl, en_info)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_enable(): pkt offload Fail\n");
		goto exit;
	}
	_mcc_set_state(minfo, MCC_TRIGGER_FW_EN);
	if (rtw_hal_mcc_enable(phl->hal, en_info, &minfo->bt_info) !=
						RTW_HAL_STATUS_SUCCESS) {
		_mcc_set_state(minfo, MCC_FW_EN_FAIL);
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_enable(): Enable FW mcc Fail\n");
		goto exit;
	}
	_mcc_set_state(minfo, MCC_RUNING);
	PHL_TRACE(COMP_PHL_MCC, _PHL_ALWAYS_, "rtw_phl_mcc_enable(): Enable FW mcc ok\n");
	if (minfo->mcc_mode == RTW_PHL_MCC_AP_CLIENT_MODE)
		_mcc_up_noa(phl, minfo);
	_mcc_dump_mcc_info(minfo);
	status = RTW_PHL_STATUS_SUCCESS;
exit:
	PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "<<< rtw_phl_mcc_enable():status(%d)\n",
		status);
	return status;
}

/*
 * Stop fw mcc
 * @ spec_role: You want to fw switch ch to the specific ch of the role when fw stop mcc
 */
enum rtw_phl_status rtw_phl_mcc_disable(struct phl_info_t *phl,
					struct rtw_wifi_role_t *spec_role)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct phl_mcc_info *minfo = NULL;
	struct rtw_phl_mcc_en_info *en_info = NULL;
	struct rtw_phl_mcc_role *m_role = NULL;

	if (!is_mcc_init(phl)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_disable(): mcc is not init, please check code\n");
		goto exit;
	}
	minfo = get_mcc_info(phl, spec_role->hw_band);
	en_info = &minfo->en_info;
	if (MCC_RUNING != minfo->state) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_disable(): MCC_RUNING != m_info->state, please check code flow\n");
		_mcc_dump_state(&minfo->state);
		goto exit;
	}
	if (NULL == (m_role = _mcc_get_mrole_by_wrole(minfo, spec_role))) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_disable(): Can't get mrole, wrole id(%d), please check code flow\n",
			spec_role->id);
		goto exit;
	}
	_mcc_set_state(minfo, MCC_TRIGGER_FW_DIS);
	if (rtw_hal_mcc_disable(phl->hal, m_role->group, m_role->macid)
						!= RTW_HAL_STATUS_SUCCESS) {
		status = RTW_PHL_STATUS_FAILURE;
		_mcc_set_state(minfo, MCC_FW_DIS_FAIL);
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_,"rtw_phl_mcc_disable(): Disable FW mcc Fail\n");
		goto exit;
	}
	rtw_hal_sync_cur_ch(phl->hal, spec_role->hw_band, spec_role->chandef);
	_mcc_set_state(minfo, MCC_STOP);
	PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_disable(): Disable FW mcc ok\n");
	if (minfo->mcc_mode == RTW_PHL_MCC_AP_CLIENT_MODE)
		_mcc_clean_noa(phl, en_info);
	status = RTW_PHL_STATUS_SUCCESS;
exit:
	return status;
}

enum rtw_phl_status rtw_phl_mcc_init_ops(struct phl_info_t *phl, struct rtw_phl_mcc_ops *ops)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct phl_com_mcc_info *com_info = NULL;

	if (!is_mcc_init(phl)) {
		PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_init_ops(): mcc is not init, please check code\n");
		goto exit;
	}
	com_info = phl_to_com_mcc_info(phl);
	com_info->ops.priv = ops->priv;
	com_info->ops.mcc_update_noa = ops->mcc_update_noa;
	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "rtw_phl_mcc_init_ops(): init ok\n");
	status = RTW_PHL_STATUS_SUCCESS;
exit:
	return status;
}

enum rtw_phl_status rtw_phl_mcc_init(struct phl_info_t *phl)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_com_t *phl_com = phl->phl_com;
	void *drv = phl_to_drvpriv(phl);
	struct mr_ctl_t *mr_ctl = phlcom_to_mr_ctrl(phl_com);
	struct hw_band_ctl_t *band_ctrl = NULL;
	u8 band_idx = 0;

	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, ">>> rtw_phl_mcc_init()\n");
	set_mcc_init_state(phl, false);
	if (mr_ctl->com_mcc == NULL) {
		mr_ctl->com_mcc = _os_mem_alloc(drv, sizeof(struct phl_com_mcc_info));
		if (mr_ctl->com_mcc != NULL) {
			PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "rtw_phl_mcc_init(): Allocate phl_com_mcc_info\n");
		} else {
			PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_init(): Allocate phl_com_mcc_info Fail\n");
			goto deinit;
		}
	}
	for (band_idx = 0; band_idx < MAX_BAND_NUM; band_idx++) {
		band_ctrl = &mr_ctl->band_ctrl[band_idx];
		if (band_ctrl->mcc_info == NULL) {
			band_ctrl->mcc_info = _os_mem_alloc(drv,
						sizeof(struct phl_mcc_info));
			if (band_ctrl->mcc_info != NULL) {
				PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "rtw_phl_mcc_init(): Allocate mcc_info for HW Band(%d)\n",
					band_idx);
			} else {
				PHL_TRACE(COMP_PHL_MCC, _PHL_ERR_, "rtw_phl_mcc_init(): Allocate mcc_info fail for HW Band(%d)\n",
					band_idx);
				goto deinit;
			}
		}
	}
	set_mcc_init_state(phl, true);
	status = RTW_PHL_STATUS_SUCCESS;
	goto exit;
deinit:
	rtw_phl_mcc_deinit(phl);
exit:
	return status;
}

void rtw_phl_mcc_deinit(struct phl_info_t *phl)
{
	struct rtw_phl_com_t *phl_com = phl->phl_com;
	void *drv = phl_to_drvpriv(phl);
	struct phl_com_mcc_info *com_info = phl_to_com_mcc_info(phl);
	struct mr_ctl_t *mr_ctl = phlcom_to_mr_ctrl(phl_com);
	struct hw_band_ctl_t *band_ctrl = NULL;
	u8 band_idx = 0;

	PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, ">>> rtw_phl_mcc_deinit()\n");
	if (com_info != NULL) {
		_os_mem_free(drv, com_info, sizeof(struct phl_com_mcc_info));
		PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "rtw_phl_mcc_deinit(): Free phl_com_mcc_info\n");
	}
	for (band_idx = 0; band_idx < MAX_BAND_NUM; band_idx++) {
		band_ctrl = &mr_ctl->band_ctrl[band_idx];
		if (band_ctrl->mcc_info != NULL) {
			_os_mem_free(drv, band_ctrl->mcc_info,
						sizeof(struct phl_mcc_info));
			PHL_TRACE(COMP_PHL_MCC, _PHL_INFO_, "rtw_phl_mcc_deinit(): Free phl_mcc_info, HwBand(%d)\n",
				band_idx);
		}
	}
	set_mcc_init_state(phl, false);
}



#endif /* CONFIG_MCC_SUPPORT */
