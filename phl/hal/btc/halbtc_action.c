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
#define _HAL_BTC_ACTION_C_
#include "../hal_headers_le.h"
#include "hal_btc.h"
#include "halbtc_fw.h"
#include "halbtc_fwdef.h"
#include "halbtc_action.h"
#include "halbtc_def.h"

#ifdef CONFIG_BTCOEX

#define _set_cx_ctrl(btc, val) rtw_hal_mac_set_coex_ctrl(btc->hal, val)

/* tdma policy template */
struct fbtc_tdma t_def[] = {
	{ CXTDMA_OFF,    CXFLC_OFF, CXTPS_OFF, 0, 0, 0, 0, 0}, /*CXTD_OFF*/
	{ CXTDMA_OFF,    CXFLC_OFF, CXTPS_OFF, 0, 0, 1, 0, 0}, /*CXTD_B2*/
	{ CXTDMA_OFF,    CXFLC_OFF, CXTPS_OFF, 0, 0, 3, 0, 0}, /*CXTD_OFF_EXT*/
	{ CXTDMA_FIX,    CXFLC_OFF, CXTPS_OFF, 0, 0, 0, 0, 0}, /* CXTD_FIX */
	{ CXTDMA_FIX,  CXFLC_NULLP,  CXTPS_ON, 0, 5, 0, 0, 0}, /* CXTD_PFIX */
	{ CXTDMA_AUTO,   CXFLC_OFF, CXTPS_OFF, 0, 0, 0, 0, 0}, /* CXTD_AUTO */
	{ CXTDMA_AUTO, CXFLC_NULLP,  CXTPS_ON, 0, 5, 0, 0, 0}, /* CXTD_PAUTO */
	{CXTDMA_AUTO2,   CXFLC_OFF, CXTPS_OFF, 0, 0, 0, 0, 0}, /* CXTD_AUTO2 */
	{CXTDMA_AUTO2, CXFLC_NULLP,  CXTPS_ON, 0, 5, 0, 0, 0} /* CXTD_PAUTO2 */
};

/* slot policy template */
struct fbtc_slot s_def[] = {
	{100, 0x55555555, SLOT_MIX}, /* CXST_OFF */
	{  5, 0x5a5a5a5a, SLOT_ISO}, /* CXST_B2W */
	{ 70, 0x5a5a5a5a, SLOT_ISO}, /* CXST_W1 */
	{ 70, 0x5a5a5aaa, SLOT_ISO}, /* CXST_W2 */
	{ 15, 0x5a5a5a5a, SLOT_ISO}, /* CXST_W2B */
	{100, 0x55555555, SLOT_MIX}, /* CXST_B1 */
	{  7, 0xaaaaaaaa, SLOT_ISO}, /* CXST_B2 */
	{  5, 0x55555555, SLOT_MIX}, /* CXST_B3 */
	{ 50, 0x55555555, SLOT_MIX}, /* CXST_B4 */
	{ 20, 0x5a5a5a5a, SLOT_ISO}, /* CXST_LK */
	{250, 0x55555555, SLOT_MIX}, /* CXST_BLK */
	{ 20, 0x5a5a5a5a, SLOT_ISO}, /* CXST_E2G */
	{ 20, 0xffffffff, SLOT_MIX}, /* CXST_E5G */
	{ 20, 0x55555555, SLOT_MIX}, /* CXST_EBT */
	{ 20, 0x5a5a5a5a, SLOT_ISO}, /* CXST_NOA */
	{ 20, 0xffffffff, SLOT_MIX}  /* CXST_FREE */
};

const u32 cxtbl[] = {
	0xffffffff, /* 0 */
	0xaaaaaaaa, /* 1 */
	0x55555555, /* 2 */
	0x66555555, /* 3 */
	0x66556655, /* 4 */
	0x5a5a5a5a, /* 5 */
	0x5a5a5aaa, /* 6 */
	0x5aaa5aaa, /* 7 */
	0x6a5a5a5a, /* 8 */
	0x6a5a5aaa, /* 9 */
	0x6a5a6a5a, /* 10 */
	0x6a5a6aaa, /* 11 */
	0x6afa5afa, /* 12 */
	0xaaaa5aaa, /* 13 */
	0xaaffffaa, /* 14 */
	0xaa5555aa, /* 15 */
	0xfafafafa, /* 16 */
	0xffffddff, /* 17 */
	0xdaffdaff, /* 18 */
	0xfafadafa  /* 19 */
};

void _set_bt_ignore_wlan_act(struct btc_t *btc, u8 enable)
{
	u8 buf = 0;

	PHL_INFO("[BTC], %s(): set bt %s wlan_act\n", __func__,
		 (enable? "ignore" : "do not ignore"));

	buf = enable;
	hal_btc_fw_set_bt(btc, SET_BT_IGNORE_WLAN_ACT, 1, &buf);
}

void _set_wl_tx_power(struct btc_t *btc, u32 level)
{
	struct btc_wl_info *wl = &btc->cx.wl;

	if (wl->rf_para.tx_pwr_freerun == level)
		return;

	wl->rf_para.tx_pwr_freerun = level;
	btc->dm.rf_trx_para.wl_tx_power = level;

	PHL_INFO("[BTC], %s(): level = %d\n", __func__, level);

	/* Todo: set wl tx power */
	btc->chip->ops->wl_tx_power(btc, level);
}

void _set_wl_rx_gain(struct btc_t *btc, u32 level)
{
	struct btc_wl_info *wl = &btc->cx.wl;

	if (wl->rf_para.rx_gain_freerun == level)
		return;

	wl->rf_para.rx_gain_freerun = level;
	btc->dm.rf_trx_para.wl_rx_gain = level;

	PHL_INFO("[BTC], %s(): level = %d\n", __func__, level);

	/* Todo: set wl ra gain */
	btc->chip->ops->wl_rx_gain(btc, level);
}

void _set_bt_tx_power(struct btc_t *btc, u32 level)
{
	struct btc_bt_info *bt = &btc->cx.bt;
	u8 buf = 0;

	if (bt->rf_para.tx_pwr_freerun == level)
		return;

	bt->rf_para.tx_pwr_freerun = level;
	btc->dm.rf_trx_para.bt_tx_power = level;

	PHL_INFO("[BTC], %s(): level = %d\n", __func__, level);

	buf = (0x100 - level) & bMASKB0;
	hal_btc_fw_set_bt(btc, SET_BT_TX_PWR, 1, &buf);
}

void _set_bt_rx_gain(struct btc_t *btc, u32 level)
{
	struct btc_bt_info *bt = &btc->cx.bt;
	u8 buf = 0;

	if (bt->rf_para.rx_gain_freerun == level ||
	    level > BTC_BT_RX_NORMAL_LVL)
		return;

	bt->rf_para.rx_gain_freerun = level;
	btc->dm.rf_trx_para.bt_rx_gain = level;

	PHL_INFO("[BTC], %s(): level = %d\n", __func__, level);

	buf = level & bMASKB0;
	hal_btc_fw_set_bt(btc, SET_BT_LNA_CONSTRAIN, 1, &buf);
}

void _set_rf_trx_para(struct btc_t *btc)
{
	struct btc_dm *dm = &btc->dm;
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_rf_trx_para para;
	u8 level_id = 0;

	if (!dm->freerun)
		dm->freerun_level = 0;

	level_id = (u8)dm->freerun_level;

	if (level_id >= btc->chip->rf_para_dlink_num ||
	    level_id >= btc->chip->rf_para_ulink_num) {
	    PHL_INFO("[BTC], %s(): invalid level_id: %d\n", __func__, level_id);
	    return;
	}

	if (wl->status.map.traffic_dir & BIT(TRAFFIC_UL))
		para = btc->chip->rf_para_ulink[level_id];
	else
		para = btc->chip->rf_para_dlink[level_id];

	_set_wl_tx_power(btc, para.wl_tx_power);
	_set_wl_rx_gain(btc, para.wl_rx_gain);
	_set_bt_tx_power(btc, para.bt_tx_power);
	_set_bt_rx_gain(btc, para.bt_rx_gain);
}

void _update_btc_state_map(struct btc_t *btc)
{
	struct btc_cx *cx = &btc->cx;
	struct btc_wl_info *wl = &cx->wl;
	struct btc_bt_info *bt = &cx->bt;
	struct btc_bt_link_info *bt_linfo = &bt->link_info;

	if (wl->status.map.connecting || wl->status.map._4way ||
	    wl->status.map.roaming) {
		cx->state_map = BTC_WLINKING;
	} else if (wl->status.map.busy) { /* only busy */
		if (bt_linfo->status.map.inq_pag)
			cx->state_map = BTC_WBUSY_BSCAN;
		else
			cx->state_map = BTC_WBUSY_BNOSCAN;
	} else if (wl->status.map.scan) { /* wl scan */
		if (bt_linfo->status.map.inq_pag)
			cx->state_map = BTC_WSCAN_BSCAN;
		else
			cx->state_map = BTC_WSCAN_BNOSCAN;
	} else { /* wl idle */
		cx->state_map = BTC_WIDLE;
	}
}

static void _set_bt_golden_rx_range(struct btc_t *btc, u8 p_id, u8 level)
{
	struct btc_bt_link_info *bt_linfo = &btc->cx.bt.link_info;
	u8 buf[2] = {0}, pos = 0, i;

	if (p_id > BTC_BT_PAN)
		return;

	for (i = 0; i< BTC_PROFILE_MAX; i++) {
		if ((p_id >> i) & 0x1) {
			pos = i;
			break;
		}
	}

	if (bt_linfo->golden_rx_shift[pos] == level)
		return;

	bt_linfo->golden_rx_shift[pos] = level;

	PHL_INFO("[BTC], %s(): p_id=%d, level=%d\n",__func__, p_id, level);

	buf[0] = level;
	buf[1] = pos;
	hal_btc_fw_set_bt(btc, SET_BT_GOLDEN_RX_RANGE, sizeof(buf), buf);
}

void _set_bt_wl_ch_info(struct btc_t *btc)
{
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_bt_info *bt = &btc->cx.bt;
	struct btc_bt_link_info *b = &bt->link_info;
	struct btc_wl_role_info *wl_rinfo = &wl->role_info;
	/*struct btc_module *module = &btc->mdinfo;*/
	u8 en = 0, i, ch = 0, bw = 0;

	if (btc->ctrl.manual || wl->status.map.scan)
		return;

	/* TBD if include module->ant.type == BTC_ANT_SHARED */
	if (wl->status.map.rf_off || bt->whql_test ||
	    wl_rinfo->link_mode == BTC_WLINK_NOLINK ||
	    wl_rinfo->link_mode == BTC_WLINK_5G ||
	    wl_rinfo->connect_cnt > BTC_TDMA_WLROLE_MAX) {
		en = false;
	} else if (wl_rinfo->link_mode == BTC_WLINK_2G_MCC ||
		   wl_rinfo->link_mode == BTC_WLINK_2G_SCC) {
		en = true;
		/* get p2p channel */
		for (i = 0; i <= MAX_WIFI_ROLE_NUMBER; i++) {
			if (wl_rinfo->active_role[i].role == PHL_RTYPE_P2P_GO ||
			    wl_rinfo->active_role[i].role == PHL_RTYPE_P2P_GO) {
				ch = wl_rinfo->active_role[i].ch;
				bw = wl_rinfo->active_role[i].bw;
				break;
			}
		}
	} else {
		en = true;
		/* get 2g channel  */
		for (i = 0; i <= MAX_WIFI_ROLE_NUMBER; i++) {
			if (wl_rinfo->active_role[i].connected &&
			    wl_rinfo->active_role[i].band == BAND_ON_24G) {
				ch = wl_rinfo->active_role[i].ch;
				bw = wl_rinfo->active_role[i].bw;
				break;
			}
		}
	}

	/* default AFH channel sapn = center-ch +- 6MHz  */
	switch (bw) {
	case CHANNEL_WIDTH_20:
#if BTC_NON_SHARED_ANT_FREERUN
		bw = 48;
#else
		bw = 20 + btc->chip->afh_guard_ch * 2;
#endif
		break;
	case CHANNEL_WIDTH_40:
		bw = 40 + btc->chip->afh_guard_ch * 2;
		break;
	case CHANNEL_WIDTH_5:
		bw = 5 + btc->chip->afh_guard_ch * 2;
		break;
	case CHANNEL_WIDTH_10:
		bw = 10 + btc->chip->afh_guard_ch * 2;
		break;
	default:
		bw = 0;
		en = false; /* turn off AFH info if BW > 40 */
		break;
	}

	/* ch = (u8)_read_wl_rf_reg(btc, RF_PATH_A, 0x18, 0xff); */

	if (wl->afh_info.en == en &&
	    wl->afh_info.ch == ch &&
	    wl->afh_info.bw == bw) {
	    	if (b->profile_cnt.last == b->profile_cnt.now) {
			/* PHL_INFO("[BTC], %s(): return because no change!\n",
				 __func__); */
			return;
	    	}
	}

	wl->afh_info.en = en;
	wl->afh_info.ch = ch;
	wl->afh_info.bw = bw;
	hal_btc_fw_set_bt(btc, SET_BT_WL_CH_INFO, 3, (u8*)&wl->afh_info);

	PHL_INFO("[BTC], %s(): en=%d, ch=%d, bw=%d\n", __func__, en, ch, bw);
	btc->cx.cnt_wl[BTC_WCNT_CH_UPDATE]++;
}

static bool _check_freerun(struct btc_t *btc)
{
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_bt_info *bt = &btc->cx.bt;
	struct btc_wl_role_info *wl_rinfo = &wl->role_info;
	struct btc_bt_link_info *bt_linfo = &bt->link_info;
	struct btc_bt_hid_desc *hid = &bt_linfo->hid_desc;

	if (btc->mdinfo.ant.type == BTC_ANT_SHARED)
		return false;

	if (wl_rinfo->connect_cnt > BTC_TDMA_WLROLE_MAX)
		return true;

	if (hid->pair_cnt > BTC_TDMA_BTHID_MAX)
		return true;

	/* Todo: check isolation/WL-RSSI/BT-RSSI */
	if (btc->mdinfo.ant.isolation >= BTC_FREERUN_ANTISO_MIN)
		return true;

#if BTC_NON_SHARED_ANT_FREERUN
	if (btc->mdinfo.ant.type == BTC_ANT_DEDICATED)
		return true;
#endif

	return false;
}

static void _set_policy(struct btc_t *btc, u16 policy_type, const char* action)
{
	struct fbtc_tdma *t = &btc->dm.tdma;
	struct fbtc_slot *s = btc->dm.slot;
	u8 type;
	u32 tbl_w1, tbl_b1, tbl_b4;
	bool mode = NM_EXEC;

	if (btc->mdinfo.ant.type == BTC_ANT_SHARED) {
		tbl_w1 = cxtbl[8];
		tbl_b1 = cxtbl[3];
		tbl_b4 = cxtbl[3];
	} else {
		tbl_w1 = cxtbl[16]; /* 19 for HID exist */
		tbl_b1 = cxtbl[17];
		tbl_b4 = cxtbl[17];
	}

	type = (u8)((policy_type & bMASKB1) >> 8);

	switch(type) {
	case BTC_CXP_USERDEF0:
		mode = FC_EXEC;
		_tdma_cpy(t, &t_def[CXTD_OFF]);
		_slot_cpy(&s[CXST_OFF], &s_def[CXST_OFF]);
		_slot_set_tbl(CXST_OFF, cxtbl[2]);
		break;
	case BTC_CXP_OFF: /* TDMA off */
		_write_scbd(btc, BTC_WSCB_TDMA, false);
		_tdma_cpy(t, &t_def[CXTD_OFF]);
		_slot_cpy(&s[CXST_OFF], &s_def[CXST_OFF]);

		switch (policy_type) {
		case BTC_CXP_OFF_BT:
			_slot_set_tbl(CXST_OFF, cxtbl[2]);
			break;
		case BTC_CXP_OFF_WL:
			_slot_set_tbl(CXST_OFF, cxtbl[1]);
			break;
		case BTC_CXP_OFF_EQ0:
			_slot_set_tbl(CXST_OFF, cxtbl[0]);
			break;
		case BTC_CXP_OFF_EQ1:
			_slot_set_tbl(CXST_OFF, cxtbl[16]);
			break;
		case BTC_CXP_OFF_EQ2:
			_slot_set_tbl(CXST_OFF, cxtbl[17]);
			break;
		case BTC_CXP_OFF_EQ3:
			_slot_set_tbl(CXST_OFF, cxtbl[18]);
			break;
		case BTC_CXP_OFF_BWB0:
			_slot_set_tbl(CXST_OFF, cxtbl[5]);
			break;
		case BTC_CXP_OFF_BWB1:
			_slot_set_tbl(CXST_OFF, cxtbl[8]);
			break;
		}
		break;
	case BTC_CXP_OFFB: /* TDMA off + beacon protect */
		_write_scbd(btc, BTC_WSCB_TDMA, false);
		_tdma_cpy(t, &t_def[CXTD_OFF_B2]);
		_slot_cpy(&s[CXST_OFF], &s_def[CXST_OFF]);
		switch (policy_type) {
		case BTC_CXP_OFFB_BWB0:
			_slot_set_tbl(CXST_OFF, cxtbl[8]);
			break;
		}
		break;
	case BTC_CXP_OFFE: /* TDMA off + beacon protect + Ext_control */
		_write_scbd(btc, BTC_WSCB_TDMA, true);
		_tdma_cpy(t, &t_def[CXTD_OFF_EXT]);
		switch (policy_type) {
		case BTC_CXP_OFFE_DEF:
			_slot_cpy(&s[CXST_E2G], &s_def[CXST_E2G]);
			_slot_cpy(&s[CXST_E5G], &s_def[CXST_E5G]);
			_slot_cpy(&s[CXST_EBT], &s_def[CXST_EBT]);
			_slot_cpy(&s[CXST_NOA], &s_def[CXST_NOA]);
			break;
		}
		break;
	case BTC_CXP_FIX: /* TDMA Fix-Slot */
		_write_scbd(btc, BTC_WSCB_TDMA, true);
		_tdma_cpy(t, &t_def[CXTD_FIX]);
		switch (policy_type) {
		case BTC_CXP_FIX_TD3030: /* W1:B1 = 30:30 */
			_slot_set(CXST_W1, 30, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 30, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_FIX_TD5050: /* W1:B1 = 50:50 */
			_slot_set(CXST_W1, 50, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 50, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_FIX_TD2030: /* W1:B1 = 20:30 */
			_slot_set(CXST_W1, 20, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 30, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_FIX_TD4010: /* W1:B1 = 40:10 */
			_slot_set(CXST_W1, 40, tbl_w1, SLOT_ISO);
			/*_slot_set(CXST_W1, 40, cxtbl[9], SLOT_ISO);*/
			_slot_set(CXST_B1, 10, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_FIX_TD7010: /* W1:B1 = 70:10 */
			_slot_set(CXST_W1, 70, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 10, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_FIX_TD2060: /* W1:B1 = 20:60 */
			_slot_set(CXST_W1, 20, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 60, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_FIX_TD3060: /* W1:B1 = 30:60 */
			_slot_set(CXST_W1, 30, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 60, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_FIX_TD2080: /* W1:B1 = 20:80 */
			_slot_set(CXST_W1, 20, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 80, tbl_b1, SLOT_MIX);
			break;
		}
		break;
	case BTC_CXP_PFIX: /* PS-TDMA Fix-Slot */
		_write_scbd(btc, BTC_WSCB_TDMA, true);
		_tdma_cpy(t, &t_def[CXTD_PFIX]);
		if (btc->cx.wl.role_info.role_map.role.ap)
			_tdma_set_flctrl(CXFLC_QOSNULL);

		switch (policy_type) {
		case BTC_CXP_PFIX_TD3030: /* W1:B1 = 30:30 */
			_slot_set(CXST_W1, 30, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 30, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_PFIX_TD5050: /* W1:B1 = 50:50 */
			_slot_set(CXST_W1, 50, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 50, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_PFIX_TD2030: /* W1:B1 = 20:30 */
			_slot_set(CXST_W1, 20, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 30, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_PFIX_TD2060: /* W1:B1 = 20:60 */
			_slot_set(CXST_W1, 20, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 60, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_PFIX_TD3070: /* W1:B1 = 30:60 */
			_slot_set(CXST_W1, 30, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 60, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_PFIX_TD2080: /* W1:B1 = 20:80 */
			_slot_set(CXST_W1, 20, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 80, tbl_b1, SLOT_MIX);
			break;
		}
		break;
	case BTC_CXP_AUTO: /* TDMA Auto-Slot */
		_write_scbd(btc, BTC_WSCB_TDMA, true);
		_tdma_cpy(t, &t_def[CXTD_AUTO]);
		switch (policy_type) {
		case BTC_CXP_AUTO_TD50100: /* W1:B1 = 50:100 */
			_slot_set(CXST_W1,  50, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 100, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_AUTO_TD60150: /* W1:B1 = 60:150 */
			_slot_set(CXST_W1,  60, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 150, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_AUTO_TD20150:  /* W1:B1 = 20:150 */
			_slot_set(CXST_W1,  20, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 150, tbl_b1, SLOT_MIX);
			break;
		}
		break;
	case BTC_CXP_PAUTO: /* PS-TDMA Auto-Slot */
		_write_scbd(btc, BTC_WSCB_TDMA, true);
		_tdma_cpy(t, &t_def[CXTD_PAUTO]);
		switch (policy_type) {
		case BTC_CXP_PAUTO_TD50100: /* W1:B1 = 50:100 */
			_slot_set(CXST_W1,  50, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 100, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_PAUTO_TD60150: /* W1:B1 = 60:150 */
			_slot_set(CXST_W1,  60, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 150, tbl_b1, SLOT_MIX);
			break;
		case BTC_CXP_PAUTO_TD20150:  /* W1:B1 = 20:150 */
			_slot_set(CXST_W1,  20, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 150, tbl_b1, SLOT_MIX);
			break;
		}
		break;
	case BTC_CXP_AUTO2: /* TDMA Auto-Slot2 */
		_write_scbd(btc, BTC_WSCB_TDMA, true);
		_tdma_cpy(t, &t_def[CXTD_AUTO2]);
		switch (policy_type) {
		case BTC_CXP_AUTO2_TD3050: /* W1:B4 = 30:50 */
			_slot_set(CXST_W1,  30, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 150, tbl_b1, SLOT_MIX);
			_slot_set(CXST_B4,  50, tbl_b4, SLOT_MIX);
			break;
		case BTC_CXP_AUTO2_TD3070: /* W1:B4 = 30:70 */
			_slot_set(CXST_W1,  30, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 150, tbl_b1, SLOT_MIX);
			_slot_set(CXST_B4,  70, tbl_b4, SLOT_MIX);
			break;
		case BTC_CXP_AUTO2_TD5050: /* W1:B4 = 50:50 */
			_slot_set(CXST_W1,  50, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 150, tbl_b1, SLOT_MIX);
			_slot_set(CXST_B4,  50, tbl_b4, SLOT_MIX);
			break;
		case BTC_CXP_AUTO2_TD6060: /* W1:B4 = 60:60 */
			_slot_set(CXST_W1,  60, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 150, tbl_b1, SLOT_MIX);
			_slot_set(CXST_B4,  60, tbl_b4, SLOT_MIX);
			break;
		case BTC_CXP_AUTO2_TD2080: /* W1:B4 = 20:80 */
			_slot_set(CXST_W1,  20, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 150, tbl_b1, SLOT_MIX);
			_slot_set(CXST_B4,  80, tbl_b4, SLOT_MIX);
			break;
		}
		break;
	case BTC_CXP_PAUTO2: /* PS-TDMA Auto-Slot2 */
		_write_scbd(btc, BTC_WSCB_TDMA, true);
		_tdma_cpy(t, &t_def[CXTD_PAUTO2]);
		switch (policy_type) {
		case BTC_CXP_PAUTO2_TD3050: /* W1:B4 = 30:50 */
			_slot_set(CXST_W1,  30, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 150, tbl_b1, SLOT_MIX);
			_slot_set(CXST_B4,  50, tbl_b4, SLOT_MIX);
			break;
		case BTC_CXP_PAUTO2_TD3070: /* W1:B4 = 30:70 */
			_slot_set(CXST_W1,  30, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 150, tbl_b1, SLOT_MIX);
			_slot_set(CXST_B4,  70, tbl_b4, SLOT_MIX);
			break;
		case BTC_CXP_PAUTO2_TD5050: /* W1:B4 = 50:50 */
			_slot_set(CXST_W1,  50, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 150, tbl_b1, SLOT_MIX);
			_slot_set(CXST_B4,  50, tbl_b4, SLOT_MIX);
			break;
		case BTC_CXP_PAUTO2_TD6060: /* W1:B4 = 60:60 */
			_slot_set(CXST_W1,  60, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 150, tbl_b1, SLOT_MIX);
			_slot_set(CXST_B4,  60, tbl_b4, SLOT_MIX);
			break;
		case BTC_CXP_PAUTO2_TD2080: /* W1:B4 = 20:80 */
			_slot_set(CXST_W1,  20, tbl_w1, SLOT_ISO);
			_slot_set(CXST_B1, 150, tbl_b1, SLOT_MIX);
			_slot_set(CXST_B4,  80, tbl_b4, SLOT_MIX);
			break;
		}
		break;
	}

	if (run_rsn("_cmd_set_coex"))
		mode = FC_EXEC;

	if ((t->rxflctrl == CXFLC_NULLP) || (t->rxflctrl == CXFLC_QOSNULL))
		btc->hal->btc_ctrl.lps = 1;
	else
		btc->hal->btc_ctrl.lps = 0;

	_update_poicy(btc, mode, policy_type, action);
}

static u8 _get_wl_role_idx(struct btc_t *btc, u8 role)
{
	struct btc_wl_role_info *wl_rinfo = &btc->cx.wl.role_info;
	u8 i, pid = 0;

	for (i = 0; i < MAX_WIFI_ROLE_NUMBER; i++) {
		if (wl_rinfo->active_role[i].role == role) {
			pid = i;
			break;
		}
	}

	return pid;
}

void _set_gnt_wl(struct btc_t *btc, u8 phy_map, u8 state)
{
	struct rtw_hal_com_t *h = btc->hal;
	struct btc_dm *dm = &btc->dm;
	struct btc_gnt_ctrl *g = dm->gnt;
	u8 i;

	RTW_INFO("%s TODO NEO\n", __func__);
#if 0 // NEO
	if (phy_map > BTC_PHY_ALL)
		return;

	for (i = 0; i < BTC_PHY_MAX; i++) {
		if (!(phy_map & BIT(i)))
			continue;

		switch (state) {
		case BTC_GNT_HW:
			g[i].gnt_wl_sw_en = 0;
			g[i].gnt_wl = 0;
			break;
		case BTC_GNT_SW_LO:
			g[i].gnt_wl_sw_en = 1;
			g[i].gnt_wl = 0;
			break;
		case BTC_GNT_SW_HI:
			g[i].gnt_wl_sw_en = 1;
			g[i].gnt_wl = 1;
			break;
		}
	}

	rtw_hal_mac_set_grant(h, (u8*)g);
#endif // if 0 NEO
}

void _set_gnt_bt(struct btc_t *btc, u8 phy_map, u8 state)
{
	struct rtw_hal_com_t *h = btc->hal;
	struct btc_dm *dm = &btc->dm;
	struct btc_gnt_ctrl *g = dm->gnt;
	u8 i;

	RTW_INFO("%s NEO TODO\n", __func__);
#if 0 // NEO
	if (phy_map > BTC_PHY_ALL)
		return;

	for (i = 0; i < BTC_PHY_MAX; i++) {
		if (!(phy_map & BIT(i)))
			continue;

		switch (state) {
		case BTC_GNT_HW:
			g[i].gnt_bt_sw_en = 0;
			g[i].gnt_bt = 0;
			break;
		case BTC_GNT_SW_LO:
			g[i].gnt_bt_sw_en = 1;
			g[i].gnt_bt = 0;
			break;
		case BTC_GNT_SW_HI:
			g[i].gnt_bt_sw_en = 1;
			g[i].gnt_bt = 1;
			break;
		}
	}

	rtw_hal_mac_set_grant(h, (u8*)g);
#endif // if 0 NEO
}

static void _set_bt_plut(struct btc_t *btc, u8 phy_map, u8 tx_val, u8 rx_val)
{
	RTW_INFO("%s NEO TODO\n", __func__);
#if 0 // NEO
	if (phy_map & BTC_PHY_0)
		rtw_hal_mac_set_polluted(btc->hal, HW_PHY_0, tx_val, rx_val);

	if (!btc->hal->dbcc_en)
		return;

	if (phy_map & BTC_PHY_1)
		rtw_hal_mac_set_polluted(btc->hal, HW_PHY_1, tx_val, rx_val);
#endif // if 0 NEO
}

static void _set_ant(struct btc_t *btc, bool force_exec, u8 phy_map, u8 type)
{
	struct btc_dm *dm = &btc->dm;
	struct btc_cx *cx = &btc->cx;
	struct btc_wl_info *wl = &cx->wl;
	struct btc_bt_info *bt = &cx->bt;
	struct btc_wl_dbcc_info *wl_dinfo = &wl->dbcc_info;
	u8 gnt_wl_ctrl, gnt_bt_ctrl, plt_ctrl, i, b2g = 0;
	u32 ant_path_type;

	ant_path_type = ((phy_map << 8) + type);

	if (run_rsn("_cmd_set_coex"))
		force_exec = FC_EXEC;

	if (!force_exec && (ant_path_type == dm->set_ant_path)) {
		/*PHL_INFO("[BTC], %s(): return by no change!!\n", __func__);*/
		return;
	} else if (bt->rfk_info.map.run) {
		PHL_INFO("[BTC], %s(): return by bt rfk!!\n", __func__);
		return;
	} else if (!run_rsn("_ntfy_wl_rfk") &&
		   wl->rfk_info.state != BTC_WRFK_STOP) {
		PHL_INFO("[BTC], %s(): return by wl rfk!!\n", __func__);
		return;
	}

	dm->set_ant_path = ant_path_type;

	PHL_INFO("[BTC], %s(): path=0x%x, set_type=%s\n", __func__,
		 phy_map, id_to_str(BTC_STR_ANTPATH, dm->set_ant_path & 0xff));

	switch (type){
	case BTC_ANT_WPOWERON:
		_set_cx_ctrl(btc, BTC_CTRL_BY_BT);
		break;
	case BTC_ANT_WINIT:
		if (bt->enable.now) {
			_set_gnt_wl(btc, phy_map, BTC_GNT_SW_LO);
			_set_gnt_bt(btc, phy_map, BTC_GNT_SW_HI);
		} else {
			_set_gnt_wl(btc, phy_map, BTC_GNT_SW_HI);
			_set_gnt_bt(btc, phy_map, BTC_GNT_SW_LO);
		}
		_set_cx_ctrl(btc, BTC_CTRL_BY_WL);
		_set_bt_plut(btc, BTC_PHY_ALL, BTC_PLT_BT, BTC_PLT_BT);
		break;
	case BTC_ANT_WONLY:
		_set_gnt_wl(btc, phy_map, BTC_GNT_SW_HI);
		_set_gnt_bt(btc, phy_map, BTC_GNT_SW_LO);
		_set_cx_ctrl(btc, BTC_CTRL_BY_WL);
		_set_bt_plut(btc, BTC_PHY_ALL, BTC_PLT_NONE, BTC_PLT_NONE);
		break;
	case BTC_ANT_WOFF:
		_set_cx_ctrl(btc, BTC_CTRL_BY_BT);
		_set_bt_plut(btc, BTC_PHY_ALL, BTC_PLT_NONE, BTC_PLT_NONE);
		break;
	case BTC_ANT_W2G:
		_set_cx_ctrl(btc, BTC_CTRL_BY_WL);
		if (btc->hal->dbcc_en) {
			for (i = 0; i < HW_PHY_MAX; i++) {
				b2g = (wl_dinfo->real_band[i] == BAND_ON_24G);

				gnt_wl_ctrl = (b2g? BTC_GNT_HW : BTC_GNT_SW_HI);
				_set_gnt_wl(btc, BIT(i), gnt_wl_ctrl);

				gnt_bt_ctrl = (b2g? BTC_GNT_HW : BTC_GNT_SW_HI);
				/* BT should control by GNT_BT if WL_2G at S0 */
				if (i == 1 &&
				    wl_dinfo->real_band[0] == BAND_ON_24G &&
				    wl_dinfo->real_band[1] == BAND_ON_5G)
				    gnt_bt_ctrl = BTC_GNT_HW;
				_set_gnt_bt(btc, BIT(i), gnt_bt_ctrl);

				plt_ctrl = (b2g? BTC_PLT_BT : BTC_PLT_NONE);
				_set_bt_plut(btc, BIT(i), plt_ctrl, plt_ctrl);
			}
		} else {
			_set_gnt_wl(btc, phy_map, BTC_GNT_HW);
			_set_gnt_bt(btc, phy_map, BTC_GNT_HW);
			_set_bt_plut(btc, BTC_PHY_ALL, BTC_PLT_BT, BTC_PLT_BT);
		}
		break;
	case BTC_ANT_W5G:
		_set_cx_ctrl(btc, BTC_CTRL_BY_WL);
		_set_gnt_wl(btc, phy_map, BTC_GNT_SW_HI);
		_set_gnt_bt(btc, phy_map, BTC_GNT_HW);
		//_set_gnt_bt(btc, phy_map, BTC_GNT_SW_HI);
		_set_bt_plut(btc, BTC_PHY_ALL, BTC_PLT_NONE, BTC_PLT_NONE);
		break;
	case BTC_ANT_W25G:
		_set_cx_ctrl(btc, BTC_CTRL_BY_WL);
		_set_gnt_wl(btc, phy_map, BTC_GNT_HW);
		_set_gnt_bt(btc, phy_map, BTC_GNT_HW);
		_set_bt_plut(btc, BTC_PHY_ALL, BTC_PLT_GNT_WL, BTC_PLT_GNT_WL);
		break;
	case BTC_ANT_FREERUN:
		_set_cx_ctrl(btc, BTC_CTRL_BY_WL);
		_set_gnt_wl(btc, phy_map, BTC_GNT_SW_HI);
		_set_gnt_bt(btc, phy_map, BTC_GNT_SW_HI);
		_set_bt_plut(btc, BTC_PHY_ALL, BTC_PLT_NONE, BTC_PLT_NONE);
		break;
	case BTC_ANT_WRFK:
		_set_cx_ctrl(btc, BTC_CTRL_BY_WL);
		_set_gnt_wl(btc, phy_map, BTC_GNT_SW_HI);
		_set_gnt_bt(btc, phy_map, BTC_GNT_SW_LO);
		_set_bt_plut(btc, phy_map, BTC_PLT_NONE, BTC_PLT_NONE);
		break;
	case BTC_ANT_BRFK:
		_set_cx_ctrl(btc, BTC_CTRL_BY_BT);
		_set_gnt_wl(btc, phy_map, BTC_GNT_SW_LO);
		_set_gnt_bt(btc, phy_map, BTC_GNT_SW_HI);
		_set_bt_plut(btc, phy_map, BTC_PLT_NONE, BTC_PLT_NONE);
		break;
	}
}

void _action_wl_only(struct btc_t *btc)
{
	PHL_INFO("[BTC], %s !!\n", __func__);

	_set_ant(btc, FC_EXEC, BTC_PHY_ALL, BTC_ANT_WONLY);
	_set_policy(btc, BTC_CXP_OFF_BT, __func__);
}

void _action_wl_init(struct btc_t *btc)
{
	PHL_INFO("[BTC], %s !!\n", __func__);

	_set_ant(btc, FC_EXEC, BTC_PHY_ALL, BTC_ANT_WINIT);
	_set_policy(btc, BTC_CXP_OFF_BT, __func__);
}

void _action_wl_off(struct btc_t *btc)
{
	PHL_INFO("[BTC], %s !!\n", __func__);

	_set_ant(btc, FC_EXEC, BTC_PHY_ALL, BTC_ANT_WOFF);
	_set_policy(btc, BTC_CXP_OFF_BT, __func__);
}

void _action_freerun(struct btc_t *btc)
{
	PHL_INFO("[BTC], %s !!\n", __func__);

	_set_ant(btc, FC_EXEC, BTC_PHY_ALL, BTC_ANT_FREERUN);
	_set_policy(btc, BTC_CXP_OFF_BT, __func__);

#if 0   /* Todo: Adjust freerun level by algorithm here!! */
	btc->dm.freerun_level = 0;
#endif

	btc->dm.freerun = true;
}

void _action_bt_whql_test(struct btc_t *btc)
{
	PHL_INFO("[BTC], %s !!\n", __func__);

	_set_ant(btc, FC_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, BTC_CXP_OFF_BT, __func__);
}

void _action_bt_rfk(struct btc_t *btc)
{
	PHL_INFO("[BTC], %s !!\n", __func__);

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_BRFK);
	_set_policy(btc, BTC_CXP_OFF_BT, __func__);
}

void _action_bt_off(struct btc_t *btc)
{
	PHL_INFO("[BTC], %s !!\n", __func__);

	_set_ant(btc, FC_EXEC, BTC_PHY_ALL, BTC_ANT_WONLY);
	_set_policy(btc, BTC_CXP_OFF_BT, __func__);
}

static void _action_bt_idle(struct btc_t *btc)
{
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);

	if (btc->mdinfo.ant.type == BTC_ANT_SHARED) { /* shared-antenna */
		switch(btc->cx.state_map) {
		case BTC_WBUSY_BNOSCAN: /*wl-busy + bt idle*/
			_set_policy(btc, BTC_CXP_FIX_TD4010, __func__);
			break;
		case BTC_WBUSY_BSCAN: /*wl-busy + bt-inq */
			_set_policy(btc, BTC_CXP_PFIX_TD5050, __func__);
			break;
		case BTC_WSCAN_BNOSCAN: /* wl-scan + bt-idle */
			_set_policy(btc, BTC_CXP_FIX_TD4010, __func__);
			break;
		case BTC_WSCAN_BSCAN: /* wl-scan + bt-inq */
			_set_policy(btc, BTC_CXP_FIX_TD5050, __func__);
			break;
		case BTC_WLINKING: /* wl-connecting + bt-inq or bt-idle */
			_set_policy(btc, BTC_CXP_FIX_TD7010, __func__);
			break;
		case BTC_WIDLE:  /* wl-dile + bt-idle */
			_set_policy(btc, BTC_CXP_OFF_BWB1, __func__);
			break;
		}
	} else { /* dedicated-antenna */
		_set_policy(btc, BTC_CXP_OFF_EQ0, __func__);
	}
}

static void _action_bt_hfp(struct btc_t *btc)
{
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_bt_info *bt = &btc->cx.bt;
	struct btc_bt_link_info *bt_linfo = &bt->link_info;

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);

	if (btc->mdinfo.ant.type == BTC_ANT_SHARED) { /* shared-antenna */
		if (wl->status.map.busy && bt_linfo->multi_link.now)
			_set_policy(btc, BTC_CXP_PFIX_TD3030, __func__);
		else
			_set_policy(btc, BTC_CXP_OFF_BWB0, __func__);
	} else { /* dedicated-antenna */
		_set_policy(btc, BTC_CXP_OFF_EQ2, __func__);
	}
}

static void _action_bt_hid(struct btc_t *btc)
{
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);

	if (btc->mdinfo.ant.type == BTC_ANT_SHARED) /* shared-antenna */
		_set_policy(btc, BTC_CXP_OFF_BWB1, __func__);
	else /* dedicated-antenna */
		_set_policy(btc, BTC_CXP_OFF_EQ3, __func__);
}

static void _action_bt_a2dp(struct btc_t *btc)
{
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);

	switch(btc->cx.state_map) {
	case BTC_WBUSY_BNOSCAN: /* wl-busy + bt-A2DP */
		_set_policy(btc, BTC_CXP_PAUTO_TD60150, __func__);
		break;
	case BTC_WBUSY_BSCAN: /* wl-busy + bt-inq + bt-A2DP */
		_set_policy(btc, BTC_CXP_PAUTO2_TD3050, __func__);
		break;
	case BTC_WSCAN_BNOSCAN: /* wl-scan + bt-A2DP */
		_set_policy(btc, BTC_CXP_AUTO_TD50100, __func__);
		break;
	case BTC_WSCAN_BSCAN: /* wl-scan + bt-inq + bt-A2DP */
		_set_policy(btc, BTC_CXP_AUTO2_TD3050, __func__);
		break;
	case BTC_WLINKING: /* wl-connecting + bt-A2DP */
		_set_policy(btc, BTC_CXP_AUTO_TD60150, __func__);
		break;
	case BTC_WIDLE:  /* wl-dile + bt-A2DP */
		_set_policy(btc, BTC_CXP_AUTO_TD20150, __func__);
		break;
	}
}

static void _action_bt_a2dpsink(struct btc_t *btc)
{
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);

	switch(btc->cx.state_map) {
	case BTC_WBUSY_BNOSCAN: /* wl-busy + bt-A2dp_Sink */
		_set_policy(btc, BTC_CXP_PFIX_TD2030, __func__);
		break;
	case BTC_WBUSY_BSCAN: /* wl-busy + bt-inq + bt-A2dp_Sink */
		_set_policy(btc, BTC_CXP_PFIX_TD2060, __func__);
		break;
	case BTC_WSCAN_BNOSCAN: /* wl-scan + bt-A2dp_Sink */
		_set_policy(btc, BTC_CXP_FIX_TD2030, __func__);
		break;
	case BTC_WSCAN_BSCAN: /* wl-scan + bt-inq + bt-A2dp_Sink */
		_set_policy(btc, BTC_CXP_FIX_TD2060, __func__);
		break;
	case BTC_WLINKING: /* wl-connecting + bt-A2dp_Sink */
		_set_policy(btc, BTC_CXP_FIX_TD3030, __func__);
		break;
	case BTC_WIDLE: /* wl-dile + bt-A2dp_Sink */
		_set_policy(btc, BTC_CXP_FIX_TD2080, __func__);
		break;
	}
}

static void _action_bt_pan(struct btc_t *btc)
{
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);

	switch(btc->cx.state_map) {
	case BTC_WBUSY_BNOSCAN: /* wl-busy + bt-PAN */
		_set_policy(btc, BTC_CXP_PFIX_TD5050, __func__);
		break;
	case BTC_WBUSY_BSCAN: /* wl-busy + bt-inq + bt-PAN */
		_set_policy(btc, BTC_CXP_PFIX_TD3070, __func__);
		break;
	case BTC_WSCAN_BNOSCAN: /* wl-scan + bt-PAN */
		_set_policy(btc, BTC_CXP_FIX_TD3030, __func__);
		break;
	case BTC_WSCAN_BSCAN: /* wl-scan + bt-inq + bt-PAN */
		_set_policy(btc, BTC_CXP_FIX_TD3060, __func__);
		break;
	case BTC_WLINKING: /* wl-connecting + bt-PAN */
		_set_policy(btc, BTC_CXP_FIX_TD4010, __func__);
		break;
	case BTC_WIDLE: /* wl-dile + bt-pan */
		_set_policy(btc, BTC_CXP_PFIX_TD2080, __func__);
		break;
	}
}

static void _action_bt_a2dp_hid(struct btc_t *btc)
{
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);

	switch(btc->cx.state_map) {
	case BTC_WBUSY_BNOSCAN: /* wl-busy + bt-A2DP+HID */
		_set_policy(btc, BTC_CXP_PAUTO_TD60150, __func__);
		break;
	case BTC_WBUSY_BSCAN: /* wl-busy + bt-inq + bt-A2DP+HID */
		_set_policy(btc, BTC_CXP_PAUTO2_TD3050, __func__);
		break;
	case BTC_WSCAN_BNOSCAN: /* wl-scan + bt-A2DP+HID */
		_set_policy(btc, BTC_CXP_AUTO_TD50100, __func__);
		break;
	case BTC_WSCAN_BSCAN: /* wl-scan + bt-inq + bt-A2DP+HID */
		_set_policy(btc, BTC_CXP_AUTO2_TD3050, __func__);
		break;
	case BTC_WLINKING: /* wl-connecting + bt-A2DP+HID */
		_set_policy(btc, BTC_CXP_AUTO_TD60150, __func__);
		break;
	case BTC_WIDLE:  /* wl-dile + bt-A2DP */
		_set_policy(btc, BTC_CXP_AUTO_TD20150, __func__);
		break;
	}
}

static void _action_bt_a2dp_pan(struct btc_t *btc)
{
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);

	switch(btc->cx.state_map) {
	case BTC_WBUSY_BNOSCAN: /* wl-busy + bt-A2DP+PAN */
		_set_policy(btc, BTC_CXP_PAUTO2_TD5050, __func__);
		break;
	case BTC_WBUSY_BSCAN: /* wl-busy + bt-inq + bt-A2DP+PAN */
		_set_policy(btc, BTC_CXP_PAUTO2_TD3070, __func__);
		break;
	case BTC_WSCAN_BNOSCAN: /* wl-scan + bt-A2DP+PAN */
		_set_policy(btc, BTC_CXP_AUTO2_TD5050, __func__);
		break;
	case BTC_WSCAN_BSCAN: /* wl-scan + bt-inq + bt-A2DP+PAN */
		_set_policy(btc, BTC_CXP_AUTO2_TD3070, __func__);
		break;
	case BTC_WLINKING: /* wl-connecting + bt-A2DP+PAN */
		_set_policy(btc, BTC_CXP_AUTO2_TD6060, __func__);
		break;
	case BTC_WIDLE:  /* wl-dile + bt-A2DP+PAN */
		_set_policy(btc, BTC_CXP_PAUTO2_TD2080, __func__);
		break;
	}
}

static void _action_bt_pan_hid(struct btc_t *btc)
{
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);

	switch(btc->cx.state_map) {
	case BTC_WBUSY_BNOSCAN: /* wl-busy + bt-PAN+HID */
		_set_policy(btc, BTC_CXP_PFIX_TD3030, __func__);
		break;
	case BTC_WBUSY_BSCAN: /* wl-busy + bt-inq + bt-PAN+HID */
		_set_policy(btc, BTC_CXP_PFIX_TD3070, __func__);
		break;
	case BTC_WSCAN_BNOSCAN: /* wl-scan + bt-PAN+HID */
		_set_policy(btc, BTC_CXP_FIX_TD3030, __func__);
		break;
	case BTC_WSCAN_BSCAN: /* wl-scan + bt-inq + bt-PAN+HID */
		_set_policy(btc, BTC_CXP_FIX_TD3060, __func__);
		break;
	case BTC_WLINKING: /* wl-connecting + bt-PAN+HID */
		_set_policy(btc, BTC_CXP_FIX_TD4010, __func__);
		break;
	case BTC_WIDLE: /* wl-dile + bt-PAN+HID */
		_set_policy(btc, BTC_CXP_PFIX_TD2080, __func__);
		break;
	}
}

static void _action_bt_a2dp_pan_hid(struct btc_t *btc)
{
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);

	switch(btc->cx.state_map) {
	case BTC_WBUSY_BNOSCAN: /* wl-busy + bt-A2DP+PAN+HID */
		_set_policy(btc, BTC_CXP_PAUTO2_TD5050, __func__);
		break;
	case BTC_WBUSY_BSCAN: /* wl-busy + bt-inq + bt-A2DP+PAN+HID */
		_set_policy(btc, BTC_CXP_PAUTO2_TD3070, __func__);
		break;
	case BTC_WSCAN_BNOSCAN: /* wl-scan + bt-A2DP+PAN+HID */
		_set_policy(btc, BTC_CXP_AUTO2_TD5050, __func__);
		break;
	case BTC_WSCAN_BSCAN: /* wl-scan + bt-inq + bt-A2DP+PAN+HID */
		_set_policy(btc, BTC_CXP_AUTO2_TD3070, __func__);
		break;
	case BTC_WLINKING: /* wl-connecting + bt-A2DP+PAN+HID */
		_set_policy(btc, BTC_CXP_AUTO2_TD6060, __func__);
		break;
	case BTC_WIDLE:  /* wl-dile + bt-A2DP+PAN+HID */
		_set_policy(btc, BTC_CXP_PAUTO2_TD2080, __func__);
		break;
	}
}

void _action_wl_5g(struct btc_t *btc)
{
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W5G);
	_set_policy(btc, BTC_CXP_OFF_EQ0, __func__);
}

void _action_wl_other(struct btc_t *btc)
{
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);

	if (btc->mdinfo.ant.type == BTC_ANT_SHARED)
		_set_policy(btc, BTC_CXP_OFFB_BWB0, __func__);
	else
		_set_policy(btc, BTC_CXP_OFF_EQ0, __func__);
}

void _action_wl_idle(struct btc_t *btc)
{
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);

	if (btc->mdinfo.ant.type == BTC_ANT_SHARED)
		_set_policy(btc, BTC_CXP_OFFB_BWB0, __func__);
	else
		_set_policy(btc, BTC_CXP_OFF_EQ0, __func__);
}

void _action_wl_nc(struct btc_t *btc)
{
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, BTC_CXP_OFF_BT, __func__);
}

void _action_wl_rfk(struct btc_t *btc)
{
	struct btc_wl_rfk_info rfk = btc->cx.wl.rfk_info;

	if (rfk.state != BTC_WRFK_START)
		return;

	PHL_INFO("[BTC], %s(): band = %d\n", __func__, rfk.band);

	_set_ant(btc, FC_EXEC, BTC_PHY_ALL, BTC_ANT_WRFK);
	_set_policy(btc, BTC_CXP_OFF_WL, __func__);

#if 0
	if (rfk.band == BAND_ON_24G) {
		_set_ant(btc, FC_EXEC, BTC_PHY_ALL, BTC_ANT_WRFK);
		_set_policy(btc, BTC_CXP_OFF_WL, __func__);
	} else {
		_set_ant(btc, FC_EXEC, BTC_PHY_ALL, BTC_ANT_FREERUN);
		_set_policy(btc, BTC_CXP_OFF_EQ0, __func__);
	}
#endif
}

static void _set_action_by_bt(struct btc_t *btc)
{
	struct btc_bt_info *bt = &btc->cx.bt;
	struct btc_bt_link_info *bt_linfo = &bt->link_info;
	struct btc_bt_hid_desc hid = bt_linfo->hid_desc;
	struct btc_bt_a2dp_desc a2dp = bt_linfo->a2dp_desc;
	struct btc_bt_pan_desc pan = bt_linfo->pan_desc;
	u8 profile_map = 0;

	if (bt_linfo->hfp_desc.exist)
		profile_map |= BTC_BT_HFP;

	if (bt_linfo->hid_desc.exist)
		profile_map |= BTC_BT_HID;

	if (bt_linfo->a2dp_desc.exist)
		profile_map |= BTC_BT_A2DP;

	if (bt_linfo->pan_desc.exist)
		profile_map |= BTC_BT_PAN;

	switch (profile_map) {
	case 0:
		if (_check_freerun(btc))
			_action_freerun(btc);
		else if (a2dp.active || pan.active)
			_action_bt_pan(btc);
		else
			_action_bt_idle(btc);
		break;
	case BTC_BT_HFP:
		_action_bt_hfp(btc);
		break;
	case BTC_BT_HFP | BTC_BT_HID:
	case BTC_BT_HID:
		if (_check_freerun(btc))
			_action_freerun(btc);
		else
			_action_bt_hid(btc);
		break;
	case BTC_BT_A2DP:
		if (_check_freerun(btc))
			_action_freerun(btc);
		else if (a2dp.sink)
			_action_bt_a2dpsink(btc);
		else if (bt_linfo->multi_link.now && !hid.pair_cnt)
			 _action_bt_a2dp_pan(btc);
		else
			_action_bt_a2dp(btc);
		break;
	case BTC_BT_PAN:
		_action_bt_pan(btc);
		break;
	case BTC_BT_A2DP | BTC_BT_HFP:
	case BTC_BT_A2DP | BTC_BT_HID:
	case BTC_BT_A2DP | BTC_BT_HFP | BTC_BT_HID:
		if (_check_freerun(btc))
			_action_freerun(btc);
		else
			_action_bt_a2dp_hid(btc);
		break;
	case BTC_BT_A2DP | BTC_BT_PAN:
		_action_bt_a2dp_pan(btc);
		break;
	case BTC_BT_PAN | BTC_BT_HFP:
	case BTC_BT_PAN | BTC_BT_HID:
	case BTC_BT_PAN | BTC_BT_HFP | BTC_BT_HID:
		_action_bt_pan_hid(btc);
		break;
	case BTC_BT_A2DP | BTC_BT_PAN | BTC_BT_HID:
	case BTC_BT_A2DP | BTC_BT_PAN | BTC_BT_HFP:
	default:
		_action_bt_a2dp_pan_hid(btc);
		break;
	}
}

void _action_wl_2g_sta(struct btc_t *btc)
{
	_set_action_by_bt(btc);
}

void _action_wl_scan(struct btc_t *btc)
{
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_wl_dbcc_info *wl_dinfo = &wl->dbcc_info;

	if (btc->hal->dbcc_en) {
		if (wl_dinfo->real_band[HW_PHY_0] != BAND_ON_24G &&
		    wl_dinfo->real_band[HW_PHY_1] != BAND_ON_24G)
			_action_wl_5g(btc);
		else
			_set_action_by_bt(btc);
	} else {
		if (wl->scan_info.band[HW_PHY_0] != BAND_ON_24G)
			_action_wl_5g(btc);
		else
			_set_action_by_bt(btc);
	}
}

void _action_wl_25g_mcc(struct btc_t *btc)
{
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W25G);

	if (btc->mdinfo.ant.type == BTC_ANT_SHARED)/* shared-antenna */
		_set_policy(btc, BTC_CXP_OFFE_DEF, __func__);
	else/* dedicated-antenna */
		_set_policy(btc, BTC_CXP_OFF_EQ0, __func__);
}

void _action_wl_2g_mcc(struct btc_t *btc)
{
	struct btc_wl_role_info *wl_rinfo = &btc->cx.wl.role_info;
	u8 pid = 0, p2p_role, noa;

	/* Check P2P Go/GC, NOA */
	if (wl_rinfo->role_map.role.p2p_go) {
		pid = _get_wl_role_idx(btc, PHL_RTYPE_P2P_GO);
		p2p_role = PHL_RTYPE_P2P_GO;
	} else {
		pid = _get_wl_role_idx(btc, PHL_RTYPE_P2P_GC);
		p2p_role = PHL_RTYPE_P2P_GC;
	}

	noa = wl_rinfo->active_role[pid].noa;

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);

	if (btc->mdinfo.ant.type == BTC_ANT_SHARED) { /* shared-antenna */
		if (noa) /* STA + GO-NOA or STA + GC-NOA */
			_set_policy(btc, BTC_CXP_OFFE_DEF, __func__);
		else /* STA + GO-No-NOA or  STA + GC-No-NOA */
			_set_policy(btc, BTC_CXP_FIX_TD3030, __func__);
		/* Todo: request GO-NOA duration by BT status/profile */
	} else { /* dedicated-antenna */
		_set_policy(btc, BTC_CXP_OFF_EQ0, __func__);
	}
}

void _action_wl_2g_scc(struct btc_t *btc)
{
	struct btc_wl_role_info *wl_rinfo = &btc->cx.wl.role_info;
	u8 pid, p2p_role, noa;

	/* Check P2P Go/GC, NOA */
	if (wl_rinfo->role_map.role.p2p_gc) {
		pid = _get_wl_role_idx(btc, PHL_RTYPE_P2P_GO);
		p2p_role = PHL_RTYPE_P2P_GO;
	} else {
		pid = _get_wl_role_idx(btc, PHL_RTYPE_P2P_GC);
		p2p_role = PHL_RTYPE_P2P_GC;
	}

	noa = wl_rinfo->active_role[pid].noa;

	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);

	if (btc->mdinfo.ant.type == BTC_ANT_SHARED) { /* shared-antenna */
		if (noa) /* STA + GO-NOA or STA + GC-NOA */
			_set_policy(btc, BTC_CXP_OFFE_DEF, __func__);
		else if (p2p_role == PHL_RTYPE_P2P_GO && !noa)/*STA+GO-No-NOA */
			_set_policy(btc, BTC_CXP_FIX_TD3030, __func__);
		/* Todo: request NOA duration by BT status/profile */
		else /* STA + GC-No-NOA */
			_set_action_by_bt(btc);
	} else { /* dedicated-antenna */
		_set_policy(btc, BTC_CXP_OFF_EQ0, __func__);
	}
}

void _action_wl_2g_ap(struct btc_t *btc)
{
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_wl_role_info *wl_rinfo = &wl->role_info;
	u8 pid;

	pid = _get_wl_role_idx(btc, PHL_RTYPE_AP);
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);

	if (btc->mdinfo.ant.type == BTC_ANT_SHARED) { /* shared-antenna */
		/* if client issue Null/Null-P
	 	 * WL/BT slot follow Null/Null-P slot
		 */
		if (wl_rinfo->active_role[pid].client_ps)
			_set_policy(btc, BTC_CXP_OFFE_DEF, __func__);
		else if (wl->status.map.busy)
			_set_policy(btc, BTC_CXP_PFIX_TD3070, __func__);
		else
			_set_policy(btc, BTC_CXP_FIX_TD3030, __func__);
	} else {/* dedicated-antenna */
		_set_policy(btc, BTC_CXP_OFF_EQ0, __func__);
	}
}

void _action_wl_2g_go(struct btc_t *btc)
{
	struct btc_wl_role_info *wl_rinfo = &btc->cx.wl.role_info;
	u8 pid;

	pid = _get_wl_role_idx(btc, PHL_RTYPE_P2P_GO);
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);

	if (btc->mdinfo.ant.type == BTC_ANT_SHARED) { /* shared-antenna */
		if (!wl_rinfo->active_role[pid].noa)/* Check if NOA */
			_set_policy(btc, BTC_CXP_FIX_TD3030, __func__);
			/* Todo: request NOA duration by BT status/profile */
		else
			_set_policy(btc, BTC_CXP_OFFE_DEF, __func__);
	} else { /* dedicated-antenna */
		_set_policy(btc, BTC_CXP_OFF_EQ0, __func__);
	}
}

void _action_wl_2g_gc(struct btc_t *btc)
{
	struct btc_wl_role_info *wl_rinfo = &btc->cx.wl.role_info;
	u8 pid;

	pid = _get_wl_role_idx(btc, PHL_RTYPE_P2P_GC);
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);

	/* Check GC, GC-NOA */
	if (btc->mdinfo.ant.type == BTC_ANT_SHARED) { /* shared-antenna */
		if (!wl_rinfo->active_role[pid].noa)
			_set_action_by_bt(btc);
		else /* Todo: if GC-Noa check noa-duration */
			_set_policy(btc, BTC_CXP_OFFE_DEF, __func__);
	} else {/* dedicated-antenna */
		_set_policy(btc, BTC_CXP_OFF_EQ0, __func__);
	}
}

void _action_wl_2g_nan(struct btc_t *btc)
{
	_set_ant(btc, NM_EXEC, BTC_PHY_ALL, BTC_ANT_W2G);
	_set_policy(btc, BTC_CXP_OFFE_DEF, __func__);
}

#endif
