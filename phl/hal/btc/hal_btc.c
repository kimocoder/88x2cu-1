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
#define _HAL_BTC_C_
#include "../hal_headers_le.h"
#include "hal_btc.h"
#include "halbtc_fw.h"
#include "halbtc_def.h"
#include "halbtc_action.h"

#ifdef CONFIG_BTCOEX

/* [31:24] -> main-version
 * [23:16] -> sub-version (increase version for each WQC testing)
 * [15:8]  -> Hot-Fix-version (Non-Main-Branch only change this)
 * [7:0]   -> branch ID, ex: 0x00-> Main-Branch
 * Modify bt_8852a.c chip_8852a member: btcx_desired, wlcx_desired if required
 * btcx_desired: BT FW coex version -> Increase main-version if update.
 * wlcx_desired: WL FW coex version -> Increase sub-version if update
 */
const u32 coex_ver = 0x02000700;

static struct btc_ops _btc_ops = {
	_send_fw_cmd,
	_ntfy_power_on,
	_ntfy_power_off,
	_ntfy_init_coex,
	_ntfy_scan_start,
	_ntfy_scan_finish,
	_ntfy_switch_band,
	_ntfy_specific_packet,
	_ntfy_show_coex_info,
	_ntfy_role_info,
	_ntfy_control,
	_ntfy_radio_state,
	_ntfy_customerize,
	_ntfy_wl_rfk,
	_ntfy_wl_sta,
	_ntfy_fwinfo,
	_ntfy_timer
};

#define _update_dbcc_band(phy_idx) \
	btc->cx.wl.dbcc_info.real_band[phy_idx] =\
	(btc->cx.wl.scan_info.phy_map & BIT(phy_idx) ?\
	btc->cx.wl.dbcc_info.scan_band[phy_idx] :\
	btc->cx.wl.dbcc_info.op_band[phy_idx])

/******************************************************************************
 *
 * coex internal functions
 *
 *****************************************************************************/
#define PTMR_PERIOD 100 /* ms */

static void _ptmr_stop(struct btc_t *btc)
{
	PHL_INFO("[BTC], %s(): stop ptmr!!\n", __func__);

	btc->ptmr_stop = true;
	_os_cancel_timer(halcom_to_drvpriv(btc->hal), &btc->ptmr);
}

static void _ptmr_start(struct btc_t *btc)
{
	btc->ptmr_stop = false;
	_os_set_timer(halcom_to_drvpriv(btc->hal), &btc->ptmr, PTMR_PERIOD);
}

static void _ptmr_cb(void *ctx)
{
	struct btc_t *btc = NULL;

	if (!ctx)
		return;
	else
		btc = (struct btc_t *)ctx;

	if (!btc->ptmr_init || btc->ptmr_stop)
		return;

	/* PHL_INFO("[BTC], Timer \n"); */

	hal_btc_send_hub_msg(btc, NULL, 0, BTC_HMSG_TMR_EN);
	_os_set_timer(halcom_to_drvpriv(btc->hal), &btc->ptmr, PTMR_PERIOD);
}

static void _ptmr_init(struct btc_t *btc)
{
	_os_init_timer(halcom_to_drvpriv(btc->hal), &btc->ptmr,
					_ptmr_cb, btc, NULL);
	btc->ptmr_init = true;
	btc->ptmr_stop = true;
}

static void _ptmr_deinit(struct btc_t *btc)
{
	btc->ptmr_stop = true;

	if (btc->ptmr_init) {
		_ptmr_stop(btc);
		PHL_INFO("[BTC], %s(): deinit ptmr!!\n", __func__);
		_os_release_timer(halcom_to_drvpriv(btc->hal), &btc->ptmr);
		btc->ptmr_init = false;
	}
}

void _set_btc_timer(struct btc_t *btc, u16 tmr_id, u16 val)
{
	if (tmr_id < BTC_TIMER_MAX) {
		if (val < 0xffff)
			btc->timer[tmr_id] = val + 1;
		else
			btc->timer[tmr_id] = 0xffff;
	}
}

static void
_send_fw_cmd(struct btc_t *btc, u8 h2c_class, u8 h2c_func, u8 *param, u16 len)
{
	struct rtw_hal_com_t *h = btc->hal;
	struct btc_wl_info *wl = &btc->cx.wl;
	struct rtw_g6_h2c_hdr hdr = {0};

	hdr.h2c_class = h2c_class;
	hdr.h2c_func = h2c_func;
	hdr.type = H2CB_TYPE_DATA;
	hdr.content_len = len;
	hdr.done_ack = 1;

	if (!wl->status.map.init_ok) {
		PHL_INFO("[BTC], %s(): return by btc not init !! \n", __func__);
		btc->fwinfo.cnt_h2c_fail++;
		return;
	}

#if 0
	PHL_INFO("[BTC], %s(): send H2C class/func = 0x%x/0x%x\n",
		 __func__, h2c_class, h2c_func);
	/*debug_dump_data(param, len, "[BTC], fw cmd data : ");*/
#endif

	btc->fwinfo.cnt_h2c++;

	if (rtw_hal_mac_send_h2c(h, &hdr, (u32 *)param) != 0)
		btc->fwinfo.cnt_h2c_fail++;
}

u32 _read_cx_reg(struct btc_t *btc, u32 offset)
{
	u32 val = 0;
	RTW_INFO("%s NEO TODO\n", __func__);
#if 0 // NEO
	rtw_hal_mac_coex_reg_read(btc->hal, offset, &val);
#endif // if 0 NEO
	return val;
}

u8 _read_cx_ctrl(struct btc_t *btc)
{
	u32 val = 0;
	RTW_INFO("%s NEO TODO\n", __func__);
#if 0 // NEO
	rtw_hal_mac_get_coex_ctrl(btc->hal, &val);
#endif // if 0 NEO

	return ((u8)val);
}

u32 _read_scbd(struct btc_t *btc)
{
	const struct btc_chip *chip = btc->chip;
	u32 scbd_val = 0;

	RTW_INFO("%s TODO NEO\n", __func__);
#if 0 // NEO
	if (!chip->scbd)
		return 0;

	rtw_hal_mac_get_scoreboard(btc->hal, &scbd_val);

	PHL_INFO("[BTC], read scbd : 0x%08x \n", scbd_val);
	btc->cx.cnt_bt[BTC_BCNT_SCBDUPDATE]++;
#endif // if 0 NEO
	return (scbd_val);
}

void _write_scbd(struct btc_t *btc, u32 val, bool state)
{
	const struct btc_chip *chip = btc->chip;
	struct btc_wl_info *wl = &btc->cx.wl;
	u32 scbd_val = 0;

	RTW_INFO("%s TODO NEO\n", __func__);
#if 0 //NEO
	if (!chip->scbd)
		return;

	/* only use bit23~0 */
	scbd_val = (state ? (wl->scbd | val) : (wl->scbd & (~val)));

	if (scbd_val != wl->scbd) {
		rtw_hal_mac_set_scoreboard(btc->hal, &scbd_val);
		PHL_INFO("[BTC], write scbd : 0x%08x \n", scbd_val);
		wl->scbd = scbd_val;

		btc->cx.cnt_wl[BTC_WCNT_SCBDUPDATE]++;
	}
#endif // if 0 NEO
}

static u8
_update_rssi_state(struct btc_t *btc, u8 pre_state, u8 rssi, u8 thresh)
{
	u8 next_state, tol = btc->chip->rssi_tol;

	if (pre_state == BTC_RSSI_ST_LOW ||
	    pre_state == BTC_RSSI_ST_STAY_LOW) {
		if (rssi >= (thresh + tol))
			next_state = BTC_RSSI_ST_HIGH;
		else
			next_state = BTC_RSSI_ST_STAY_LOW;
	} else {
		if (rssi < thresh)
			next_state = BTC_RSSI_ST_LOW;
		else
			next_state = BTC_RSSI_ST_STAY_HIGH;
	}

	return next_state;
}

void _write_bt_reg(struct btc_t *btc, u8 reg_type, u16 addr, u32 val)
{
	u8 buf[4] = {0};

	/* set write address */
	buf[0] = reg_type;
	buf[1] = addr & bMASKB0;
	buf[2] = (addr & bMASKB1) >> 8;
	hal_btc_fw_set_bt(btc, SET_BT_WREG_ADDR, 3, buf);

	/* set write value */
	buf[0] = val & bMASKB0;
	buf[1] = (val & bMASKB1) >> 8;
	buf[2] = (val & bMASKB2) >> 16;
	buf[3] = (val & bMASKB3) >> 23;
	hal_btc_fw_set_bt(btc, SET_BT_WREG_VAL, 4, buf);
}

void _read_bt_reg(struct btc_t *btc, u8 reg_type, u16 addr)
{
	/* this function is only for API call.
	 * If BTC should use hal_btc_fw_set_monreg to read bt reg.
	 */
	u8 buf[3] = {0};

	/* set write address */
	buf[0] = reg_type;
	buf[1] = addr & bMASKB0;
	buf[2] = (addr & bMASKB1) >> 8;
	hal_btc_fw_set_bt(btc, SET_BT_RREG_ADDR, sizeof(buf), buf);

	/* To do wait FW event -> BTF_EVNT_BT_REG*/
}

static void _set_wl_rx_agc_by_btc(struct btc_t *btc)
{
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_wl_role_info *wl_rinfo = &wl->role_info;
	struct btc_wl_dbcc_info *wl_dinfo = &wl->dbcc_info;
	bool is_btg = false;

	if (btc->ctrl.manual)
		return;

	/* control by WL_FW if MCC dual_band  */
	if (wl_rinfo->link_mode == BTC_WLINK_25G_MCC)
		return;

	/* notify halbb ignore GNT_BT or not for WL BB Rx-AGC control */
	if (btc->mdinfo.ant.type == BTC_ANT_DEDICATED)
		is_btg = false;
	else if (wl_rinfo->link_mode == BTC_WLINK_5G) /* always 0 if 5G */
		is_btg = false;
	else if (wl_rinfo->link_mode == BTC_WLINK_25G_DBCC &&
		 wl_dinfo->real_band[1] != BAND_ON_24G)
		is_btg = false;
	else
		is_btg = true;

	if (!run_rsn("_ntfy_init_coex") && is_btg == btc->dm.wl_btg_rx)
		return;

	btc->dm.wl_btg_rx = is_btg;
	/* rtw_hal_bb_ctrl_btg control the following:
	 * The Gnt-Mux setting in BB for gnt_sw_control, gnt_val
	 * if btc->dm.wl_btg_rx = true (gnt_sw_control = 1)
	 * Lte_rx:    0x10980[17]=1, 0x10980[29]=0
	 * Gnt_wl:    0x10980[18]=1, 0x10980[28]=1
	 * Gnt_bt_tx: 0x10980[19]=1, 0x10980[27]=0
	 * Gnt_bt:    0x10980[20]=1, 0x10980[26]=0
	 * if if btc->dm.wl_btg_rx = false (gnt_sw_control = 0)
	 * Lte_rx:    0x10980[17]=0, 0x10980[29]=0
	 * Gnt_wl:    0x10980[18]=0, 0x10980[28]=1
	 * Gnt_bt_tx: 0x10980[19]=0, 0x10980[27]=0
	 * Gnt_bt:    0x10980[20]=0, 0x10980[26]=0
	 */

	rtw_hal_bb_ctrl_btg(btc->hal, is_btg);
}

static void _set_wl_tx_limit(struct btc_t *btc)
{
	struct btc_cx *cx = &btc->cx;
	struct btc_dm *dm = &btc->dm;
	struct btc_wl_info *wl = &cx->wl;
	struct btc_bt_info *bt = &cx->bt;
	struct btc_bt_link_info *b = &bt->link_info;
	struct btc_bt_hfp_desc *hfp = &b->hfp_desc;
	struct btc_bt_hid_desc *hid = &b->hid_desc;
	struct btc_wl_role_info *wl_rinfo = &wl->role_info;
	struct btc_wl_link_info *plink = NULL;
	u8 mode = wl_rinfo->link_mode, i;
	u8 tx_retry = 0;
	u32 tx_time = 0;
	u16 enable = 0;
	bool reenable = false;

	if (btc->ctrl.manual)
		return;

	if (btc->dm.freerun || btc->ctrl.igno_bt || b->profile_cnt.now == 0 ||
	    mode == BTC_WLINK_5G || mode == BTC_WLINK_NOLINK) {
		enable = 0;
		tx_time = BTC_MAX_TX_TIME_DEF;
		tx_retry = BTC_MAX_TX_RETRY_DEF;
	} else if ((hfp->exist && hid->exist) || hid->pair_cnt > 1) {
		enable = 1;
		tx_time = BTC_MAX_TX_TIME_L2;
		tx_retry = BTC_MAX_TX_RETRY_L1;
	} else if (hfp->exist || hid->exist) {
		enable = 1;
		tx_time = BTC_MAX_TX_TIME_L3;
		tx_retry = BTC_MAX_TX_RETRY_L1;
	} else {
		enable = 0;
		tx_time = BTC_MAX_TX_TIME_DEF;
		tx_retry = BTC_MAX_TX_RETRY_DEF;
	}

	if (dm->wl_tx_limit.enable == enable &&
	    dm->wl_tx_limit.tx_time == tx_time &&
	    dm->wl_tx_limit.tx_retry == tx_retry)
		return;

	if (!dm->wl_tx_limit.enable && enable)
		reenable = true;

	dm->wl_tx_limit.enable = enable;
	dm->wl_tx_limit.tx_time = tx_time;
	dm->wl_tx_limit.tx_retry = tx_retry;

	for (i = 0; i < MAX_WIFI_ROLE_NUMBER; i++) {
		plink = &wl->link_info[i];

		if (!plink->connected)
			continue;

		/* backup the original tx time before tx-limit on */
		if (reenable) {
			rtw_hal_mac_get_tx_time(btc->hal,
						(u8)plink->mac_id,
						&plink->tx_time);
			rtw_hal_mac_get_tx_retry_limit(btc->hal,
						       (u8)plink->mac_id,
						       &plink->tx_retry);
		}

		/* restore the original tx time if no tx-limit */
		if (!enable) {
			rtw_hal_mac_set_tx_time(btc->hal, 1, 1,
						(u8)plink->mac_id,
						plink->tx_time);
			rtw_hal_mac_set_tx_retry_limit(btc->hal, 1, 1,
						       (u8)plink->mac_id,
						       plink->tx_retry);
		} else {
			rtw_hal_mac_set_tx_time(btc->hal, 1, 0,
						(u8)plink->mac_id, tx_time);
			rtw_hal_mac_set_tx_retry_limit(btc->hal, 1, 0,
						       (u8)plink->mac_id,
						       tx_retry);
		}
	}
}

void _set_bt_psd_report(struct btc_t *btc, u8 start_idx, u8 rpt_type)
{
	u8 buf[2] = {0};

	PHL_INFO("[BTC], %s(): set bt psd\n", __func__);

	buf[0] = start_idx;
	buf[1] = rpt_type;
	hal_btc_fw_set_bt(btc, SET_BT_PSD_REPORT, 2, buf);
}

static void _set_bt_info_report(struct btc_t *btc, u8 trigger)
{
	u8 buf = 0;

	PHL_INFO("[BTC], %s(): query bt info\n", __func__);

	buf = trigger;
	hal_btc_fw_set_bt(btc, SET_BT_INFO_REPORT, 1, &buf);
}

static void _reset_btc_var(struct btc_t *btc, u8 type)
{
	struct rtw_hal_com_t *h = btc->hal;
	struct btc_cx *cx = &btc->cx;
	struct btc_wl_info *wl = &cx->wl;
	struct btc_bt_info *bt = &cx->bt;
	struct btc_bt_link_info *bt_linfo = &bt->link_info;
	struct btc_wl_link_info *wl_linfo = wl->link_info;
	u8 i;

	PHL_INFO("[BTC], %s !! \n", __func__);

	/* Reset Coex variable */
	if (type & BTC_RESET_CX)
		hal_mem_set(h, cx, 0, sizeof(struct btc_cx));
	else if (type & BTC_RESET_BTINFO) /* only for BT enable */
		hal_mem_set(h, bt, 0, sizeof(struct btc_bt_info));

	if (type & BTC_RESET_CTRL) {
		hal_mem_set(h, &btc->ctrl, 0, sizeof(struct btc_ctrl));
		btc->ctrl.trace_step = FCXDEF_STEP;
	}

	/* Init Coex variables that are not zero */
	if (type & BTC_RESET_DM) {
		hal_mem_set(h, &btc->dm, 0, sizeof(struct btc_dm));
		hal_mem_set(h, bt_linfo->rssi_state, 0, BTC_BT_RSSI_THMAX);

		for (i = 0; i < MAX_WIFI_ROLE_NUMBER; i++)
			hal_mem_set(h, wl_linfo[i].rssi_state, 0,
				    BTC_WL_RSSI_THMAX);

		/* set the slot_now table to original */
		_tdma_cpy(&btc->dm.tdma_now, &t_def[CXTD_OFF]);
		_tdma_cpy(&btc->dm.tdma, &t_def[CXTD_OFF]);
		_slots_cpy(btc->dm.slot_now, s_def);
		_slots_cpy(btc->dm.slot, s_def);
		btc->policy_len = 0;

		btc->dm.coex_info_map = BTC_COEX_INFO_ALL;
		btc->dm.wl_tx_limit.tx_time = BTC_MAX_TX_TIME_DEF;
		btc->dm.wl_tx_limit.tx_retry = BTC_MAX_TX_RETRY_DEF;
	}

	if (type & BTC_RESET_MDINFO)
		hal_mem_set(h, &btc->mdinfo, 0, sizeof(struct btc_module));
}

static bool _check_wl_rfk_request(struct btc_t *btc)
{
	struct btc_cx *cx = &btc->cx;
	struct btc_bt_info *bt = &cx->bt;

	_update_bt_scbd(btc, true);

	cx->cnt_wl[BTC_WCNT_RFK_REQ]++;

	if ((bt->rfk_info.map.run || bt->rfk_info.map.req) &&
	    (!bt->rfk_info.map.timeout)) {
		cx->cnt_wl[BTC_WCNT_RFK_REJECT]++;
		return false;
	} else {
		cx->cnt_wl[BTC_WCNT_RFK_GO]++;
		return true;
	}
}

void _set_init_info(struct btc_t *btc)
{
	struct btc_dm *dm = &btc->dm;
	struct btc_wl_info *wl = &btc->cx.wl;

	dm->init_info.wl_only = (u8)dm->wl_only;
	dm->init_info.bt_only = (u8)dm->bt_only;
	dm->init_info.wl_init_ok = (u8)wl->status.map.init_ok;
	dm->init_info.dbcc_en = btc->hal->dbcc_en;
	dm->init_info.cx_other = btc->cx.other.type;
	dm->init_info.wl_guard_ch = btc->chip->afh_guard_ch;
	dm->init_info.module = btc->mdinfo;
}

static void _update_wl_info(struct btc_t *btc)
{
	RTW_ERR("%s NEO TODO\n", __func__);
#if 0
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_wl_link_info *wl_linfo = wl->link_info;
	struct btc_wl_role_info *wl_rinfo = &wl->role_info;
	struct btc_wl_dbcc_info *wl_dinfo = &wl->dbcc_info;
	struct rtw_hal_com_t *h = btc->hal;
	u8 i, cnt_connect = 0, cnt_connecting = 0, cnt_active = 0;
	u8 cnt_2g = 0, cnt_5g = 0, phy;
	u32 wl_2g_ch[2] = {0}, wl_5g_ch[2] = {0};
	bool b2g = false, b5g = false;

	hal_mem_set(h, wl_rinfo, 0, sizeof(struct btc_wl_role_info));

	for (i = 0; i < MAX_WIFI_ROLE_NUMBER; i++) {
		/* check if role active? */
		if (!wl_linfo[i].active)
			continue;

		cnt_active++;
		wl_rinfo->active_role[cnt_active-1].role = wl_linfo[i].role;
		wl_rinfo->active_role[cnt_active-1].pid = wl_linfo[i].pid;
		wl_rinfo->active_role[cnt_active-1].phy = wl_linfo[i].phy;
		wl_rinfo->active_role[cnt_active-1].band = wl_linfo[i].band;
		wl_rinfo->active_role[cnt_active-1].noa = (u8)wl_linfo[i].noa;
		wl_rinfo->active_role[cnt_active-1].connected = 0;

		wl->port_id[wl_linfo[i].role] = wl_linfo[i].pid;

		phy = wl_linfo[i].phy;

		/* check dbcc role */
		if (h->dbcc_en && phy < HW_PHY_MAX) {
			wl_dinfo->role[phy] = wl_linfo[i].role;
			wl_dinfo->op_band[phy] = wl_linfo[i].band;
			_update_dbcc_band(phy);
			hal_btc_fw_set_drv_info(btc, CXDRVINFO_DBCC);
		}

		/* check if role connect? */
		if (wl_linfo[i].connected == MLME_NO_LINK)
			continue;
		else if (wl_linfo[i].connected == MLME_LINKING)
			cnt_connecting++;
		else
			cnt_connect++;

		wl_rinfo->role_map.val |= BIT(wl_linfo[i].role);
		wl_rinfo->active_role[cnt_active-1].ch = wl_linfo[i].ch;
		wl_rinfo->active_role[cnt_active-1].bw = wl_linfo[i].bw;
		wl_rinfo->active_role[cnt_active-1].connected = 1;

		/* only care 2 roles + BT coex */
		if (wl_linfo[i].band != BAND_ON_24G) {
			if (cnt_5g <= ARRAY_SIZE(wl_5g_ch) - 1)
				wl_5g_ch[cnt_5g] = wl_linfo[i].ch;
			cnt_5g++;
			b5g = true;
		} else {
			if (cnt_2g <= ARRAY_SIZE(wl_2g_ch) - 1)
				wl_2g_ch[cnt_2g] = wl_linfo[i].ch;
			cnt_2g++;
			b2g = true;
		}
	}

	wl_rinfo->connect_cnt = cnt_connect;

	/* wl->status.map.connecting = !!(cnt_connecting); */

	/* Be careful to change the following sequence!! */
	if (cnt_connect == 0) {
		wl_rinfo->link_mode = BTC_WLINK_NOLINK;
	} else if (!b2g && b5g) {
		wl_rinfo->link_mode = BTC_WLINK_5G;
	} else if (wl_rinfo->role_map.role.nan) {
		wl_rinfo->link_mode = BTC_WLINK_2G_NAN;
	} else if (cnt_connect > BTC_TDMA_WLROLE_MAX) {
		wl_rinfo->link_mode = BTC_WLINK_OTHER;
	} else  if (b2g && b5g && cnt_connect == 2) {
		if (h->dbcc_en) {
			switch(wl_dinfo->role[HW_PHY_0]) {
			case PHL_RTYPE_STATION:
				wl_rinfo->link_mode = BTC_WLINK_2G_STA;
				break;
			case PHL_RTYPE_P2P_GO:
				wl_rinfo->link_mode = BTC_WLINK_2G_GO;
				break;
			case PHL_RTYPE_P2P_GC:
				wl_rinfo->link_mode = BTC_WLINK_2G_GC;
				break;
			case PHL_RTYPE_AP:
				wl_rinfo->link_mode = BTC_WLINK_2G_AP;
				break;
			default:
				wl_rinfo->link_mode = BTC_WLINK_OTHER;
				break;
			}
		} else {
			wl_rinfo->link_mode = BTC_WLINK_25G_MCC;
		}
	} else if (!b5g && cnt_connect == 2) { /* cnt_connect = 2 */
		if (wl_rinfo->role_map.role.station &&
		    (wl_rinfo->role_map.role.p2p_go ||
		    wl_rinfo->role_map.role.p2p_gc)) {
			if (wl_2g_ch[0] == wl_2g_ch[1])
				wl_rinfo->link_mode = BTC_WLINK_2G_SCC;
			else
				wl_rinfo->link_mode= BTC_WLINK_2G_MCC;
		}
	} else if (!b5g && cnt_connect == 1) { /* cnt_connect = 1 */
		if (wl_rinfo->role_map.role.station)
			wl_rinfo->link_mode = BTC_WLINK_2G_STA;
		else if (wl_rinfo->role_map.role.ap)
			wl_rinfo->link_mode = BTC_WLINK_2G_AP;
		else if (wl_rinfo->role_map.role.p2p_go)
			wl_rinfo->link_mode = BTC_WLINK_2G_GO;
		else if (wl_rinfo->role_map.role.p2p_gc)
			wl_rinfo->link_mode = BTC_WLINK_2G_GC;
		else
			wl_rinfo->link_mode = BTC_WLINK_OTHER;
	}

	hal_btc_fw_set_drv_info(btc, CXDRVINFO_ROLE);
#endif
}

void _run_coex(struct btc_t *btc, const char *reason)
{
	RTW_ERR("%s TODO NEO\n", __func__);
#if 0
	struct btc_dm *dm = &btc->dm;
	struct btc_cx *cx = &btc->cx;
	struct btc_wl_info *wl = &cx->wl;
	struct btc_wl_role_info *wl_rinfo = &wl->role_info;
	u8 mode = wl_rinfo->link_mode;

	PHL_INFO("[BTC], %s(): reason = %s, mode=%d\n", __func__, reason, mode);
	_rsn_cpy(dm->run_reason, (char*)reason);

#if BTC_CX_FW_OFFLOAD
	if (wl->rfk_info.state != BTC_WRFK_STOP && mode != BTC_WLINK_5G) {
		_action_wl_rfk(btc);
	} else  if (wl->rfk_info.state == BTC_WRFK_STOP) {
		PHL_INFO("[BTC], %s(): offload to WL_FW\n", __func__);
		hal_btc_fw_set_drv_info(btc, CXDRVINFO_SMAP);
		hal_btc_fw_set_drv_info(btc, CXDRVINFO_RUN);
	}

	goto exit;
#else

	_update_btc_state_map(btc);

	/* Be careful to change the following function sequence!! */
	if (btc->ctrl.manual) {
		PHL_INFO("[BTC], %s(): return for Manual CTRL!!\n", __func__);
		return;
	}

	if (btc->ctrl.igno_bt &&
	    (run_rsn("_update_bt_info") || run_rsn("_update_bt_scbd"))) {
		PHL_INFO("[BTC], %s(): return for Stop Coex DM!!\n", __func__);
		return;
	}

	if (!wl->status.map.init_ok) {
		PHL_INFO("[BTC], %s(): return for WL init fail!!\n", __func__);
		return;
	}

	dm->cnt_dm[BTC_DCNT_RUN]++;

	if (btc->ctrl.always_freerun) {
		_action_freerun(btc);
		btc->ctrl.igno_bt = true;
		goto exit;
	}

	if (dm->wl_only) {
		_action_wl_only(btc);
		btc->ctrl.igno_bt = true;
		goto exit;
	}

	if (wl->status.map.rf_off || dm->bt_only) {
		_action_wl_off(btc);
		btc->ctrl.igno_bt = true;
		goto exit;
	}

	btc->ctrl.igno_bt = false;
	dm->freerun = false;

	if (run_rsn("_ntfy_init_coex")) {
		_action_wl_init(btc);
		goto exit;
	}

	if (!cx->bt.enable.now && !cx->other.type) {
		_action_bt_off(btc);
		goto exit;
	}

	if (cx->bt.whql_test) {
		_action_bt_whql_test(btc);
		goto exit;
	}

	if (wl->rfk_info.state != BTC_WRFK_STOP) {
		_action_wl_rfk(btc);
		goto exit;
	}

	if (run_rsn("_ntfy_scan_start") || run_rsn("_ntfy_switch_band") ||
	    cx->state_map == BTC_WLINKING) {
		_action_wl_scan(btc);
		goto exit;
	}

	switch (mode) {
	case BTC_WLINK_NOLINK:
		_action_wl_nc(btc);
		break;
	case BTC_WLINK_2G_STA:
		_action_wl_2g_sta(btc);
		break;
	case BTC_WLINK_2G_AP:
		_action_wl_2g_ap(btc);
		break;
	case BTC_WLINK_2G_GO:
		_action_wl_2g_go(btc);
		break;
	case BTC_WLINK_2G_GC:
		_action_wl_2g_gc(btc);
		break;
	case BTC_WLINK_2G_SCC:
		_action_wl_2g_scc(btc);
		break;
	case BTC_WLINK_2G_MCC:
		_action_wl_2g_mcc(btc);
		break;
	case BTC_WLINK_25G_MCC:
		_action_wl_25g_mcc(btc);
		break;
	case BTC_WLINK_5G:
		_action_wl_5g(btc);
		break;
	case BTC_WLINK_2G_NAN:
		_action_wl_2g_nan(btc);
		break;
	default:
		_action_wl_other(btc);
		break;
	}
#endif
exit:
	_set_bt_wl_ch_info(btc);
	_set_wl_rx_agc_by_btc(btc);
	_set_wl_tx_limit(btc);
	_set_rf_trx_para(btc);
#endif // if 0 NEO
}

static void _update_offload_runinfo(struct btc_t *btc, u8 *buf, u32 len)
{
#if BTC_CX_FW_OFFLOAD
	u32 val;
	struct btc_cxr_result *r = NULL;

	if (!buf || buf[0] >= BTC_CXR_MAX)
		return;

	switch(buf[0]) {
	case BTC_CXR_WSCBD:
		val = (buf[4] << 24) + (buf[3] << 16) + (buf[2] << 8) + buf[1];
		if (val & BIT(31)) /* if val & BIT(31) --> write scbd bit false */
			_write_scbd(btc, val & 0x7fffffff, false);
		else
			_write_scbd(btc, val, true);
		break;
	case BTC_CXR_RESULT:
		r = (struct btc_cxr_result*) &buf[1];

		btc->dm.freerun = r->dm.freerun;
		btc->dm.wl_ps_ctrl = r->dm.wl_ps_ctrl;
		btc->dm.leak_ap = r->dm.leak_ap;
		btc->ctrl.igno_bt = (u16)r->dm.igno_bt;
		btc->dm.noisy_level = r->dm.noisy_level;
		btc->dm.set_ant_path = r->dm.set_ant_path;

		btc->dm.rf_trx_para = r->rf_trx_para;
		btc->cx.state_map = r->cx_state_map;
		btc->policy_type = (u16)r->policy_type;
		btc->dm.cnt_dm[BTC_DCNT_RUN] = r->run_cnt;

		hal_mem_cpy(btc->hal, btc->dm.run_reason, r->run_reason,
			    BTC_RSN_MAXLEN);
		hal_mem_cpy(btc->hal, btc->dm.run_action, r->run_action,
			    BTC_ACT_MAXLEN);
		break;
	}
#endif
}

void _update_bt_scbd(struct btc_t *btc, bool only_update)
{
	struct btc_cx *cx = &btc->cx;
	struct btc_bt_info *bt = &cx->bt;
	u32 val;
	bool status_change = false;

	if (!btc->chip->scbd)
		return;

	PHL_INFO("[BTC], %s !! \n", __func__);

	val = _read_scbd(btc);

	if (val == 0xffffffff) {
		PHL_INFO("[BTC], %s return by invalid scbd value\n", __func__);
		return;
	}

	if (!(val & BTC_BSCB_ON) ||
	    btc->dm.cnt_dm[BTC_DCNT_BTCNT_FREEZE] >= BTC_CHK_HANG_MAX)
		bt->enable.now = 0;
	else
		bt->enable.now = 1;

	if (bt->enable.now != bt->enable.last)
		status_change = true;

	/* reset bt info if bt re-enable */
	if (bt->enable.now && !bt->enable.last) {
		_reset_btc_var(btc, BTC_RESET_BTINFO);
		cx->cnt_bt[BTC_BCNT_REENABLE]++;
		bt->enable.now = 1;
	}

	bt->enable.last = bt->enable.now;

	bt->scbd = val;
	bt->mbx_avl = !!(val & BTC_BSCB_ACT);

        if (bt->whql_test != (u32)(!!(val & BTC_BSCB_WHQL)))
		status_change = true;
	bt->whql_test = !!(val & BTC_BSCB_WHQL);

	bt->btg_type = (val & BTC_BSCB_BT_S1 ? BTC_BT_BTG: BTC_BT_ALONE);

	bt->link_info.a2dp_desc.active = !!(val & BTC_BSCB_A2DP_ACT);

	/* if rfk run 1->0 */
	if (bt->rfk_info.map.run && !(val & BTC_BSCB_RFK_RUN))
		status_change = true;
	bt->rfk_info.map.run  = !!(val & BTC_BSCB_RFK_RUN);

	bt->rfk_info.map.req = !!(val & BTC_BSCB_RFK_REQ);

	bt->run_patch_code = !!(val & BTC_BSCB_PATCH_CODE);

#if !BTC_CX_FW_OFFLOAD
	if (!only_update && status_change)
		_run_coex(btc, __func__);
#endif
}

/******************************************************************************
 *
 * coexistence operations for external notifications
 *
 *****************************************************************************/
static void _ntfy_power_on(struct btc_t *btc)
{
	/* no action for power on, beacuse power-on move halmac API
	* the _ntfy_power_on = _ntfy_init_coex
	*/
}

static void _ntfy_power_off(struct btc_t *btc)
{
	PHL_INFO("[BTC], %s !! \n", __func__);
	btc->dm.cnt_notify[BTC_NCNT_POWER_OFF]++;

	btc->cx.wl.status.map.rf_off = 1;

	_write_scbd(btc, BTC_WSCB_ALL, false);
	_run_coex(btc, __func__);

	hal_btc_fw_en_rpt(btc, RPT_EN_ALL, 0);
}

static void _ntfy_init_coex(struct btc_t *btc, u8 mode)
{
	RTW_ERR("%s TODO NEO\n", __func__);
#if 0 // NEO TODO
	struct btc_dm *dm = &btc->dm;
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_chip_ops *ops = btc->chip->ops;

	PHL_INFO("[BTC], %s(): mode=%d\n", __func__, mode);
	dm->cnt_notify[BTC_NCNT_INIT_COEX]++;
	dm->wl_only = (mode == BTC_MODE_WL? 1 : 0);
	dm->bt_only = (mode == BTC_MODE_BT? 1 : 0);

	if (!wl->status.map.init_ok) {
		PHL_INFO("[BTC], %s(): return for WL init fail!!\n", __func__);
		dm->error.map.init = true;
		return;
	}

	if (ops && ops->init_cfg)
		ops->init_cfg(btc);

	/* Setup RF front end type from EFuse RFE type*/
	if (ops && ops->set_rfe)
		ops->set_rfe(btc);

	_write_scbd(btc, BTC_WSCB_ACTIVE | BTC_WSCB_ON | BTC_WSCB_BTLOG, true);
	_update_bt_scbd(btc, true);

	/* check PTA control owner to avoid BT coex issue */
	if (_read_cx_ctrl(btc) == BTC_CTRL_BY_WL) {
		PHL_INFO("[BTC], %s(): PTA owner warning!!\n", __func__);
		dm->error.map.pta_owner = true;
	}

	_set_init_info(btc);
	_set_wl_tx_power(btc, 0xff); /* original tx power, no Tx power adjust */
	hal_btc_fw_set_slots(btc, CXST_MAX, dm->slot);
	hal_btc_fw_set_monreg(btc);
	hal_btc_fw_set_drv_info(btc, CXDRVINFO_INIT);
	hal_btc_fw_set_drv_info(btc, CXDRVINFO_CTRL);

	_run_coex(btc, __func__);
	_ptmr_start(btc);

#if BTC_GPIO_DBG_EN
	hal_btc_fw_set_gpio_dbg(btc, CXDGPIO_EN_MAP,
				BIT(BTC_DBG_GNT_WL) | BIT(BTC_DBG_GNT_BT));
#endif
#endif // if 0
}

static void _ntfy_scan_start(struct btc_t *btc, u8 phy_idx, u8 band)
{
	struct btc_wl_info *wl = &btc->cx.wl;

	RTW_ERR("%s NEO TODO\n", __func__);

#if 0 // NEO TODO

	PHL_INFO("[BTC], %s(): phy_idx=%d, band=%d\n", __func__, phy_idx, band);
	btc->dm.cnt_notify[BTC_NCNT_SCAN_START]++;

	wl->status.map.scan = true;
	wl->scan_info.band[phy_idx] = band;
	wl->scan_info.phy_map |= BIT(phy_idx);
	hal_btc_fw_set_drv_info(btc, CXDRVINFO_SCAN);

	if (btc->hal->dbcc_en) {
		wl->dbcc_info.scan_band[phy_idx] = band;
		_update_dbcc_band(phy_idx);
		hal_btc_fw_set_drv_info(btc, CXDRVINFO_DBCC);
	}

	_run_coex(btc, __func__);
#endif // if 0 NEO
}

static void _ntfy_scan_finish(struct btc_t *btc, u8 phy_idx)
{
	RTW_ERR("%s NEO TODO\n", __func__);
#if 0 // NEO TODO
	struct btc_wl_info *wl = &btc->cx.wl;

	PHL_INFO("[BTC], %s(): phy_idx=%d\n", __func__, phy_idx);
	btc->dm.cnt_notify[BTC_NCNT_SCAN_FINISH]++;

	wl->status.map.scan = false;
	wl->scan_info.phy_map &= ~BIT(phy_idx);
	hal_btc_fw_set_drv_info(btc, CXDRVINFO_SCAN);

	if (btc->hal->dbcc_en) {
		_update_dbcc_band(phy_idx);
		hal_btc_fw_set_drv_info(btc, CXDRVINFO_DBCC);
	}

	_run_coex(btc, __func__);
#endif // if 0 NEO
}

static void _ntfy_switch_band(struct btc_t *btc, u8 phy_idx, u8 band)
{
	RTW_ERR("%s NEO TODO\n", __func__);
#if 0 // NEO TODO
	struct btc_wl_info *wl = &btc->cx.wl;

	PHL_INFO("[BTC], %s(): phy_idx=%d, band=%d\n", __func__, phy_idx, band);
	btc->dm.cnt_notify[BTC_NCNT_SWITCH_BAND]++;

	wl->scan_info.band[phy_idx] = band;
	wl->scan_info.phy_map |= BIT(phy_idx);
	hal_btc_fw_set_drv_info(btc, CXDRVINFO_SCAN);

	if (btc->hal->dbcc_en) {
		wl->dbcc_info.scan_band[phy_idx] = band;
		_update_dbcc_band(phy_idx);
		hal_btc_fw_set_drv_info(btc, CXDRVINFO_DBCC);
	}

	_run_coex(btc, __func__);
#endif // if 0 NEO
}

static void _ntfy_specific_packet(struct btc_t *btc, u8 pkt_type)
{
	struct btc_cx *cx = &btc->cx;
	struct btc_wl_info *wl = &cx->wl;
	u32 cnt;

	switch (pkt_type) {
	case PACKET_DHCP:
	case PACKET_EAPOL:
		cnt = ++cx->cnt_wl[BTC_WCNT_DHCP];
		PHL_INFO("[BTC], %s(): DHCP cnt=%d \n", __func__, cnt);
		_set_btc_timer(btc, BTC_TIMER_WL_SPECPKT, BTC_SPECPKT_MAXT);
		break;
	case PACKET_ARP:
		cnt = ++cx->cnt_wl[BTC_WCNT_ARP];
		PHL_INFO("[BTC], %s(): ARP cnt=%d\n", __func__, cnt);
		return;
	}

	wl->status.map._4way = true;
	btc->dm.cnt_notify[BTC_NCNT_SPECIAL_PACKET]++;

	_run_coex(btc, __func__);
}

static void _update_bt_psd(struct btc_t *btc, u8 *buf, u32 len)
{
	PHL_INFO("[BTC], %s():\n", __func__);
}

static void _update_bt_info(struct btc_t *btc, u8 *buf, u32 len)
{
	struct rtw_hal_com_t *h = btc->hal;
	struct btc_cx *cx = &btc->cx;
	struct btc_bt_info *bt = &cx->bt;
	struct btc_bt_link_info *b = &bt->link_info;
	struct btc_bt_hfp_desc *hfp = &b->hfp_desc;
	struct btc_bt_hid_desc *hid = &b->hid_desc;
	struct btc_bt_a2dp_desc *a2dp = &b->a2dp_desc;
	struct btc_bt_pan_desc *pan = &b->pan_desc;
	union btc_btinfo btinfo;

	if (buf[BTC_BTINFO_L1] != 6)
		return;

	/* return if bt info match last bt-info  */
	if (!hal_mem_cmp(h, bt->raw_info, buf, BTC_BTINFO_MAX)) {
		PHL_INFO("[BTC], %s return by bt-info duplicate!!\n", __func__);
		cx->cnt_bt[BTC_BCNT_INFOSAME]++;
		return;
	}

	hal_mem_cpy(h, bt->raw_info, buf, BTC_BTINFO_MAX);

	PHL_INFO("[BTC], %s: bt_info[2]=0x%02x\n", __func__, bt->raw_info[2]);

	/* reset to mo-connect before update */
	b->status.val = BTC_BLINK_NOCONNECT;
	b->profile_cnt.last = b->profile_cnt.now;
	b->relink.last = b->relink.now;
	b->multi_link.last = b->multi_link.now;
	bt->inq_pag.last = bt->inq_pag.now;
	b->profile_cnt.now = 0;
	hid->type = 0;

	/* ======= parse raw info low-Byte2 ======= */
	btinfo.val = bt->raw_info[BTC_BTINFO_L2];
	b->status.map.connect = btinfo.lb2.connect;
	b->status.map.sco_busy = btinfo.lb2.sco_busy;
	b->status.map.acl_busy = btinfo.lb2.acl_busy;
	b->status.map.inq_pag = btinfo.lb2.inq_pag;
	bt->inq_pag.now = btinfo.lb2.inq_pag;
	cx->cnt_bt[BTC_BCNT_INQPAG] += !!(bt->inq_pag.now && !bt->inq_pag.last);

	hfp->exist = btinfo.lb2.hfp;
	b->profile_cnt.now += (u8)hfp->exist;
	hid->exist = btinfo.lb2.hid;
	b->profile_cnt.now += (u8)hid->exist;
	a2dp->exist = btinfo.lb2.a2dp;
	b->profile_cnt.now += (u8)a2dp->exist;
	pan->exist = btinfo.lb2.pan;
	b->profile_cnt.now += (u8)pan->exist;

	/* ======= parse raw info low-Byte3 ======= */
	btinfo.val = bt->raw_info[BTC_BTINFO_L3];
	cx->cnt_bt[BTC_BCNT_RETRY] = btinfo.lb3.retry;
	b->cqddr = btinfo.lb3.cqddr;
	cx->cnt_bt[BTC_BCNT_INQ] += !!(btinfo.lb3.inq && !bt->inq);
	bt->inq = btinfo.lb3.inq;
	cx->cnt_bt[BTC_BCNT_PAGE] += !!(btinfo.lb3.pag && !bt->pag);
	bt->pag = btinfo.lb3.pag;

	b->status.map.mesh_busy = btinfo.lb3.mesh_busy;
	/* ======= parse raw info high-Byte0 ======= */
	btinfo.val = bt->raw_info[BTC_BTINFO_H0];
	/* raw val is dBm unit, translate from -100~ 0dBm to 0~100%*/
	b->rssi = btc->chip->ops->bt_rssi(btc, btinfo.hb0.rssi);

	/* ======= parse raw info high-Byte1 ======= */
	btinfo.val = bt->raw_info[BTC_BTINFO_H1];
	b->status.map.ble_connect = btinfo.hb1.ble_connect;
	if (btinfo.hb1.ble_connect)
		hid->type |= (hid->exist? BTC_HID_BLE : BTC_HID_RCU);

	cx->cnt_bt[BTC_BCNT_REINIT] += !!(btinfo.hb1.reinit && !bt->reinit);
	bt->reinit = btinfo.hb1.reinit;
	cx->cnt_bt[BTC_BCNT_RELINK] += !!(btinfo.hb1.relink && !b->relink.now);
	b->relink.now = btinfo.hb1.relink;
	cx->cnt_bt[BTC_BCNT_IGNOWL] += !!(btinfo.hb1.igno_wl && !bt->igno_wl);
	bt->igno_wl = btinfo.hb1.igno_wl;

#if !BTC_CX_FW_OFFLOAD
	if (bt->igno_wl && !cx->wl.status.map.rf_off)
		_set_bt_ignore_wlan_act(btc, false);
#endif

	hid->type |= (btinfo.hb1.voice? BTC_HID_RCU_VOICE : 0);
	bt->ble_scan_en = btinfo.hb1.ble_scan;

	cx->cnt_bt[BTC_BCNT_ROLESW] += !!(btinfo.hb1.role_sw && !b->role_sw);
	b->role_sw = btinfo.hb1.role_sw;

	b->multi_link.now = btinfo.hb1.multi_link;

	/* ======= parse raw info high-Byte2 ======= */
	btinfo.val = bt->raw_info[BTC_BTINFO_H2];
	pan->active = !!btinfo.hb2.pan_active;

	cx->cnt_bt[BTC_BCNT_AFH] += !!(btinfo.hb2.afh_update && !b->afh_update);
	b->afh_update = btinfo.hb2.afh_update;
	a2dp->active = btinfo.hb2.a2dp_active;
	b->slave_role = btinfo.hb2.slave;
	hid->slot_info = btinfo.hb2.hid_slot;
	hid->pair_cnt = btinfo.hb2.hid_cnt;
	hid->type |= (hid->slot_info == BTC_HID_218? BTC_HID_218 : BTC_HID_418);

	/* ======= parse raw info high-Byte3 ======= */
	btinfo.val = bt->raw_info[BTC_BTINFO_H3];
	a2dp->bitpool = btinfo.hb3.a2dp_bitpool;
	a2dp->sink = btinfo.hb3.a2dp_sink;

#if !BTC_CX_FW_OFFLOAD
	if (b->profile_cnt.now || b->status.map.ble_connect)
		hal_btc_fw_en_rpt(btc, RPT_EN_BT_AFH_MAP, 1);
	else
		hal_btc_fw_en_rpt(btc, RPT_EN_BT_AFH_MAP, 0);

	if (a2dp->exist && a2dp->flush_time == 0)
		hal_btc_fw_en_rpt(btc, RPT_EN_BT_DEVICE_INFO, 1);
	else
		hal_btc_fw_en_rpt(btc, RPT_EN_BT_DEVICE_INFO, 0);

	_run_coex(btc, __func__);
#endif
}

static void
_ntfy_show_coex_info(struct btc_t *btc, void (*out)(const char *), u8 type)
{
	RTW_ERR("%s NEO TODO\n", __func__);
#if 0 // NEO
	if (!out)
		return;

	PHL_INFO("[BTC], %s(): type=%d\n", __func__, type);

	btc->dm.cnt_notify[BTC_NCNT_SHOW_COEX_INFO]++;

	_disp_cx_info(btc, out);
	_disp_wl_info(btc, out);
	_disp_bt_info(btc, out);
	_disp_dm_info(btc, out);

	if (type != 0)
		hal_btc_fw_dm_msg(btc, &btc->fwinfo, out);

	_disp_mreg(btc, &btc->fwinfo, out);
	_disp_summary(btc, &btc->fwinfo, out);
#endif // if 0 NEO
}

static void _ntfy_role_info(struct btc_t *btc, u8 rid,
			    struct btc_wl_link_info *info,
			    enum role_state reason)
{
	struct rtw_hal_com_t *h = btc->hal;
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_wl_link_info *wlinfo = NULL;

	PHL_INFO("[BTC], %s(), role_id=%d, reason=%d\n", __func__, rid, reason);

	if (rid >= MAX_WIFI_ROLE_NUMBER)
		return;

	btc->dm.cnt_notify[BTC_NCNT_ROLE_INFO]++;

	wlinfo = &wl->link_info[rid];

	hal_mem_cpy(h, wlinfo, info, sizeof(struct btc_wl_link_info));

	/* update wifi_link_info_ext variable */
	_update_wl_info(btc);

	if (reason == PHL_ROLE_MSTS_STA_CONN_START)
		wl->status.map.connecting = 1;
	else
		wl->status.map.connecting = 0;

	_run_coex(btc, __func__);
}

static void _ntfy_control(struct btc_t *btc, u8 type, u16 len, u8 *buf)
{
}

static void _ntfy_radio_state(struct btc_t *btc, bool rf_on)
{
	PHL_INFO("[BTC], %s !! \n", __func__);
	btc->dm.cnt_notify[BTC_NCNT_RADIO_STATE]++;

	btc->cx.wl.status.map.rf_off = (rf_on? 0 : 1);

	_run_coex(btc, __func__);

	if (rf_on) {
		hal_btc_fw_en_rpt(btc, RPT_EN_MREG, 1);
		_write_scbd(btc, BTC_WSCB_ACTIVE, true);
	} else {
		hal_btc_fw_en_rpt(btc, RPT_EN_ALL, 0);
		_write_scbd(btc, BTC_WSCB_ACTIVE, false);
	}
}

static void _ntfy_customerize(struct btc_t *btc, u8 type, u16 len, u8 *buf)
{
	struct btc_bt_info *bt = &btc->cx.bt;

	if (!buf)
		return;

	PHL_INFO("[BTC], %s !! \n", __func__);
	btc->dm.cnt_notify[BTC_NCNT_CUSTOMERIZE]++;

	switch (type) {
	case PHL_BTC_CNTFY_BTINFO:
		if (len != 1)
			return;
		buf[0] = bt->raw_info[BTC_BTINFO_L2];
		break;
	}
}

static u8 _ntfy_wl_rfk(struct btc_t *btc, u8 phy_path, u8 type, u8 state)
{
	struct btc_cx *cx = &btc->cx;
	struct btc_wl_info *wl = &cx->wl;
	bool result = BTC_WRFK_REJECT;

	RTW_ERR("%s TODO NEO \n", __func__);
#if 0 // NEO TODO
	wl->rfk_info.type = type;
	wl->rfk_info.path_map = phy_path & BTC_RFK_PATH_MAP;
	wl->rfk_info.phy_map = (phy_path & BTC_RFK_PHY_MAP) >> 4;
	wl->rfk_info.band = (phy_path & BTC_RFK_BAND_MAP) >> 6;
	state &= (BIT(0) | BIT(1));

	PHL_INFO("[BTC], %s()_start: phy=0x%x, path=0x%x, type=%d, state=%d\n",
		 __func__, wl->rfk_info.phy_map, wl->rfk_info.path_map,
		 type, state);

	switch (state) {
	case BTC_WRFK_START:
		result = _check_wl_rfk_request(btc);
		wl->rfk_info.state = (result? BTC_WRFK_START : BTC_WRFK_STOP);
#if 0
		hal_btc_fw_set_drv_info(btc, CXDRVINFO_RFK);
#endif
		_write_scbd(btc, BTC_WSCB_WLRFK, result);

		btc->dm.cnt_notify[BTC_NCNT_WL_RFK]++;
		break;
	case BTC_WRFK_ONESHOT_START:
	case BTC_WRFK_ONESHOT_STOP:
		if (wl->rfk_info.state == BTC_WRFK_STOP) {
			result = BTC_WRFK_REJECT;
		} else {
			result = BTC_WRFK_ALLOW;
			wl->rfk_info.state = state;
		}
		break;
	case BTC_WRFK_STOP:
		result = BTC_WRFK_ALLOW;
		wl->rfk_info.state = BTC_WRFK_STOP;
#if 0
		hal_btc_fw_set_drv_info(btc, CXDRVINFO_RFK);
#endif
		_write_scbd(btc, BTC_WSCB_WLRFK, false);
		break;
	}

	if (result == BTC_WRFK_ALLOW)
		_run_coex(btc, __func__);

	if (wl->rfk_info.state == BTC_WRFK_START) /* wait 100ms*3 */
		_set_btc_timer(btc, BTC_TIMER_WL_RFKTO, BTC_WRFK_MAXT);

	PHL_INFO("[BTC], %s()_finish: rfk_cnt=%d, result=%d\n",
		 __func__, btc->dm.cnt_notify[BTC_NCNT_WL_RFK], result);

#endif
	return result;
}

static void _ntfy_wl_sta(struct btc_t *btc, u8 ntfy_num,
			 struct btc_wl_stat_info stat_info[], u8 reason)
{
	struct rtw_hal_com_t *h = btc->hal;
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_wl_stat_info *w = NULL;
	struct btc_traffic *t;
	struct btc_wl_link_info *link_info = NULL;
	u8 i, j, busy = 0, dir = 0;
	u8 busy_all = 0, dir_all = 0;
	bool is_sta_change = false;

	btc->dm.cnt_notify[BTC_NCNT_WL_STA]++;

	for (i = 0; i < ntfy_num; i++) {
		w = &stat_info[i];
		t = &w->stat.traffic;

		link_info = &wl->link_info[w->pid];

		if (link_info->connected == MLME_NO_LINK)
			continue;

		hal_mem_cpy(h, (void *)&link_info->stat.traffic, (void *)t,
			    sizeof(struct btc_traffic));

		link_info->stat.rssi = w->stat.rssi;
		/* check if rssi across wl_rssi_thres boundary */
		for (j = 0; j < BTC_WL_RSSI_THMAX; j++) {
			link_info->rssi_state[j] =
				_update_rssi_state(btc,
						   link_info->rssi_state[j],
						   link_info->stat.rssi,
						   btc->chip->wl_rssi_thres[j]);

			if (BTC_RSSI_CHANGE(link_info->rssi_state[j]))
				is_sta_change = true;
		}

		/* set busy once idle->busy immediately */
		if ((t->tx_lvl != RTW_TFC_IDLE) || (t->rx_lvl != RTW_TFC_IDLE)) {
			busy = 1;
			link_info->idle_cnt = 0;

			if (t->tx_lvl > t->rx_lvl)
				dir = TRAFFIC_UL;
			else
				dir = TRAFFIC_DL;
		} else {/*set idle once busy -> idle after BTC_BUSY2IDLE_THRES*/
			link_info->idle_cnt++;
			if (link_info->idle_cnt >= BTC_BUSY2IDLE_THRES) {
				busy = 0;
				dir = TRAFFIC_DL;
			} else {
				busy = link_info->busy;
				dir = link_info->dir;
			}
		}

		if (link_info->busy != busy || link_info->dir != dir) {
			is_sta_change = true;
			link_info->busy = busy;
			link_info->dir = dir;
		}

		busy_all |= link_info->busy;
		dir_all |= BIT(link_info->dir);
	}

	if (is_sta_change) {
		wl->status.map.busy = (u32)busy_all;
		wl->status.map.traffic_dir = (u32)dir_all;
		_run_coex(btc, __func__);
	}
}

static void _ntfy_fwinfo(struct btc_t *btc, u8 *buf, u32 len, u8 cls, u8 func)
{
	struct btf_fwinfo *pfwinfo = &btc->fwinfo;

	if (!buf || !len)
		return;

	btc->dm.cnt_notify[BTC_NCNT_FWINFO]++;
	pfwinfo->cnt_c2h++;

	if (cls == BTFC_FW_EVENT) {
		switch (func) {
		case BTF_EVNT_RPT:
		case BTF_EVNT_BUF_OVERFLOW:
			pfwinfo->event[func]++;
			hal_btc_fw_event(btc, func, buf, len);
			break;
		case BTF_EVNT_BT_INFO:
			btc->cx.cnt_bt[BTC_BCNT_INFOUPDATE]++;
			_update_bt_info(btc, buf, len);
			break;
		case BTF_EVNT_BT_SCBD:
			_update_bt_scbd(btc, false);
			break;
		case BTF_EVNT_BT_PSD:
			_update_bt_psd(btc, buf, len);
			break;
		case BTF_EVNT_BT_REG:
			PHL_INFO("[BTC], %s(), len=%d, buf[0]=%x, buf[1]=%x, buf[2]=%x, buf[3]=%x ",
				 __func__, len, buf[0], buf[1], buf[2], buf[3]);
			btc->dbg.rb_done = true;
			btc->dbg.rb_val = ((buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | (buf[0]));
			//hal_btc_send_hub_msg(btc, buf, 4, BTC_HMSG_BT_REG_READBACK);
			break;
		case BTF_EVNT_CX_RUNINFO:
			btc->dm.cnt_dm[BTC_DCNT_CX_RUNINFO]++;
			_update_offload_runinfo(btc, buf, len);
			break;
		}
	}
}

static void _ntfy_timer(struct btc_t *btc, u32 tmr_map)
{
	struct btc_dm *dm = &btc->dm;
	struct btc_cx *cx = &btc->cx;
	struct btc_wl_info *wl = &cx->wl;

	RTW_ERR("%s NEO TODO \n", __func__);
#if 0 // NEO TODO
	PHL_INFO("[BTC], %s(): tmr_map = 0x%x\n", __func__, tmr_map);
	dm->cnt_notify[BTC_NCNT_TIMER]++;

	if (tmr_map & BIT(BTC_TIMER_WL_RFKTO)) {
		if (wl->rfk_info.state != BTC_WRFK_STOP) {
			cx->cnt_wl[BTC_WCNT_RFK_TIMEOUT]++;
			dm->error.map.wl_rfk_timeout = true;
			wl->rfk_info.state = BTC_WRFK_STOP;
			_write_scbd(btc, BTC_WSCB_WLRFK, false);
			_run_coex(btc, __func__);
		}
	}

	if (tmr_map & BIT(BTC_TIMER_WL_SPECPKT)) {
		wl->status.map._4way = false;
		_run_coex(btc, __func__);
	}
#endif // if 0 NEO
}

/******************************************************************************
 *
 * coexistence extern functions
 *
 *****************************************************************************/
/*
 * btc related sw initialization
 */
bool hal_btc_init(struct btc_t *btc)
{
	RTW_ERR("%s TODO NEO\n", __func__);
	return false;
#if 0 // NEO 
	switch (btc->hal->chip_id) {
	case CHIP_WIFI6_8852A:
		PHL_INFO("[BTC], %s(): Init 8852A!!\n", __func__);
		btc->chip = &chip_8852a;
		break;
	default:
		PHL_ERR("[BTC], %s(): no matched IC!!\n", __func__);
		btc->cx.wl.status.map.init_ok = false;
		return false;
	}

	_reset_btc_var(btc, BTC_RESET_ALL);
	_ptmr_init(btc);

	_rsn_cpy(btc->dm.run_reason, "None");
	_act_cpy(btc->dm.run_action, "None");

	btc->hal->btc_vc.btc_ver = coex_ver;
	btc->ops = &_btc_ops;
	btc->mlen = BTC_MSG_MAXLEN;
	btc->ctrl.igno_bt = true;
	btc->cx.wl.status.map.init_ok = true;

	return true;
#endif // if 0 NEO
}

void hal_btc_deinit(struct btc_t *btc)
{
	_ptmr_deinit(btc);
}

#endif

