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
#define _HAL_BTC_FW_C_
#include "../hal_headers_le.h"
#include "hal_btc.h"
#include "halbtc_fw.h"
#include "halbtc_def.h"

#ifdef CONFIG_BTCOEX


void _chk_err_state(struct btc_t *btc, u8 type, u32 cnt)
{
	RTW_ERR("%s TODO NEO\n", __func__);
#if 0
	struct btc_cx *cx = &btc->cx;
	struct btc_dm *dm = &btc->dm;
	struct btc_bt_info *bt = &cx->bt;

	switch (type) {
	case BTC_DCNT_RPT_FREEZE:
		if (dm->cnt_dm[BTC_DCNT_RPT] == cnt &&
		    btc->fwinfo.rpt_en_map)
			dm->cnt_dm[BTC_DCNT_RPT_FREEZE]++;
		else
			dm->cnt_dm[BTC_DCNT_RPT_FREEZE] = 0;

		if (dm->cnt_dm[BTC_DCNT_RPT_FREEZE] >= BTC_CHK_HANG_MAX)
			dm->error.map.wl_fw_hang = true;
		else
			dm->error.map.wl_fw_hang = false;

		dm->cnt_dm[BTC_DCNT_RPT] = cnt;
		break;
	case BTC_DCNT_CYCLE_FREEZE:
		if (dm->cnt_dm[BTC_DCNT_CYCLE] == cnt &&
		    dm->tdma_now.type != CXTDMA_OFF)
			dm->cnt_dm[BTC_DCNT_CYCLE_FREEZE]++;
		else
			dm->cnt_dm[BTC_DCNT_CYCLE_FREEZE] = 0;

		if (dm->cnt_dm[BTC_DCNT_CYCLE_FREEZE] >= BTC_CHK_HANG_MAX)
			dm->error.map.cycle_hang = true;
		else
			dm->error.map.cycle_hang = false;

		dm->cnt_dm[BTC_DCNT_CYCLE] = cnt;
		break;
	case BTC_DCNT_W1_FREEZE:
		if (dm->cnt_dm[BTC_DCNT_W1] == cnt &&
		    dm->tdma_now.type != CXTDMA_OFF)
			dm->cnt_dm[BTC_DCNT_W1_FREEZE]++;
		else
			dm->cnt_dm[BTC_DCNT_W1_FREEZE] = 0;

		if (dm->cnt_dm[BTC_DCNT_W1_FREEZE] >= BTC_CHK_HANG_MAX)
			dm->error.map.w1_hang = true;
		else
			dm->error.map.w1_hang = false;

		dm->cnt_dm[BTC_DCNT_W1] = cnt;
		break;
	case BTC_DCNT_B1_FREEZE:
		if (dm->cnt_dm[BTC_DCNT_B1] == cnt &&
		    dm->tdma_now.type != CXTDMA_OFF)
			dm->cnt_dm[BTC_DCNT_B1_FREEZE]++;
		else
			dm->cnt_dm[BTC_DCNT_B1_FREEZE] = 0;

		if (dm->cnt_dm[BTC_DCNT_B1_FREEZE] >= BTC_CHK_HANG_MAX)
			dm->error.map.b1_hang = true;
		else
			dm->error.map.b1_hang = false;

		dm->cnt_dm[BTC_DCNT_B1] = cnt;
		break;
	case BTC_DCNT_TDMA_NONSYNC:
		if (cnt != 0) /* if tdma not sync between drv/fw  */
			dm->cnt_dm[BTC_DCNT_TDMA_NONSYNC]++;
		else
			dm->cnt_dm[BTC_DCNT_TDMA_NONSYNC] = 0;

		if (dm->cnt_dm[BTC_DCNT_TDMA_NONSYNC] >= BTC_CHK_HANG_MAX)
			dm->error.map.tdma_no_sync = true;
		else
			dm->error.map.tdma_no_sync = false;
		break;
	case BTC_DCNT_SLOT_NONSYNC:
		if (cnt != 0) /* if slot not sync between drv/fw  */
			dm->cnt_dm[BTC_DCNT_SLOT_NONSYNC]++;
		else
			dm->cnt_dm[BTC_DCNT_SLOT_NONSYNC] = 0;

		if (dm->cnt_dm[BTC_DCNT_SLOT_NONSYNC] >= BTC_CHK_HANG_MAX)
			dm->error.map.tdma_no_sync = true;
		else
			dm->error.map.tdma_no_sync = false;
		break;
	case BTC_DCNT_BTCNT_FREEZE:
		cnt = cx->cnt_bt[BTC_BCNT_HIPRI_RX] +
		      cx->cnt_bt[BTC_BCNT_HIPRI_TX] +
		      cx->cnt_bt[BTC_BCNT_LOPRI_RX] +
		      cx->cnt_bt[BTC_BCNT_LOPRI_TX];

		if (cnt == 0)
			dm->cnt_dm[BTC_DCNT_BTCNT_FREEZE]++;
		else
			dm->cnt_dm[BTC_DCNT_BTCNT_FREEZE] = 0;

		if ((dm->cnt_dm[BTC_DCNT_BTCNT_FREEZE] >= BTC_CHK_HANG_MAX &&
		     bt->enable.now) || (!dm->cnt_dm[BTC_DCNT_BTCNT_FREEZE] &&
		     !bt->enable.now))
			_update_bt_scbd(btc, false);
		break;
	}
#endif // NEO if 0
}

#if 0 // NEO TODO

void _disp_summary(struct btc_t *btc, struct btf_fwinfo *pfwinfo,
			   void (*out)(const char *))
{
	struct btc_rpt_cmn_info *pcinfo = NULL;
	struct fbtc_rpt_ctrl *prptctrl = NULL;
	struct btc_cx *cx = &btc->cx;
	struct btc_dm *dm = &btc->dm;
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_bt_info *bt = &cx->bt;
	char *buf = btc->mbuf;
	size_t len = btc->mlen;
	u32 cnt_sum = 0, *cnt = btc->dm.cnt_notify;
	u8 i;

	if (!(dm->coex_info_map & BTC_COEX_INFO_SUMMARY))
		return;

	_os_snprintf(buf, len, "%s", "\n\r========== [Statistics] ==========");
	out(buf);

	pcinfo = &pfwinfo->rpt_ctrl.cinfo;
	if (pcinfo->valid) {
		prptctrl = &pfwinfo->rpt_ctrl.finfo;

		_os_snprintf(buf, len, "\n\r %-15s : h2c_cnt=%d(fail:%d, fw_recv:%d), c2h_cnt=%d(fw_send:%d), ",
			     "[summary]",
			     pfwinfo->cnt_h2c, pfwinfo->cnt_h2c_fail, prptctrl->h2c_cnt,
			     pfwinfo->cnt_c2h, prptctrl->c2h_cnt);
		out(buf);

		_os_snprintf(buf, len, "rpt_cnt=%d(fw_send:%d), rpt_map= 0x%x, dm_error_map:0x%x",
			     pfwinfo->event[BTF_EVNT_RPT], prptctrl->rpt_cnt,
			     prptctrl->rpt_enable, dm->error.val);
		out(buf);

		btc->fwinfo.rpt_en_map = prptctrl->rpt_enable;
		wl->ver_info.fw_coex = prptctrl->wl_fw_coex_ver;
		wl->ver_info.fw = prptctrl->wl_fw_ver;
		dm->wl_fw_cx_offload = !!(prptctrl->wl_fw_cx_offload);

		_chk_err_state(btc, BTC_DCNT_RPT_FREEZE,
			       pfwinfo->event[BTF_EVNT_RPT]);

		if (dm->error.map.wl_fw_hang) {
			_os_snprintf(buf, len, " (WL FW Hang!!!)");
			out(buf);
		}

		_os_snprintf(buf, len, "\n\r %-15s : send_ok:%d, send_fail:%d, recv:%d",
			     "[mailbox]", prptctrl->mb_send_ok_cnt,
			     prptctrl->mb_send_fail_cnt, prptctrl->mb_recv_cnt);
		out(buf);
		_os_snprintf(buf, len, "(A2DP_empty:%d, A2DP_flowstop:%d, A2DP_full:%d)",
			     prptctrl->mb_a2dp_empty_cnt,
			     prptctrl->mb_a2dp_flct_cnt,
			     prptctrl->mb_a2dp_full_cnt);
		out(buf);

		_os_snprintf(buf, len, "\n\r %-15s : wl_rfk[req:%d/go:%d/reject:%d/timeout:%d]",
		     "[RFK]", cx->cnt_wl[BTC_WCNT_RFK_REQ],
		     cx->cnt_wl[BTC_WCNT_RFK_GO],
		     cx->cnt_wl[BTC_WCNT_RFK_REJECT],
		     cx->cnt_wl[BTC_WCNT_RFK_TIMEOUT]);
		out(buf);

		_os_snprintf(buf, len, ", bt_rfk[req:%d/go:%d/reject:%d/timeout:%d/fail:%d]",
			     prptctrl->bt_rfk_cnt[BTC_BCNT_RFK_REQ],
			     prptctrl->bt_rfk_cnt[BTC_BCNT_RFK_GO],
			     prptctrl->bt_rfk_cnt[BTC_BCNT_RFK_REJECT],
			     prptctrl->bt_rfk_cnt[BTC_BCNT_RFK_TIMEOUT],
			     prptctrl->bt_rfk_cnt[BTC_BCNT_RFK_FAIL]);
		out(buf);

		if (prptctrl->bt_rfk_cnt[BTC_BCNT_RFK_TIMEOUT] > 0)
			bt->rfk_info.map.timeout = 1;
		else
			bt->rfk_info.map.timeout = 0;

		dm->error.map.wl_rfk_timeout = bt->rfk_info.map.timeout;
	} else {
		_os_snprintf(buf, len, "\n\r %-15s : h2c_cnt=%d(fail:%d), c2h_cnt=%d, rpt_cnt=%d",
			     "[summary]", pfwinfo->cnt_h2c,
			     pfwinfo->cnt_h2c_fail, pfwinfo->cnt_c2h,
			     pfwinfo->event[BTF_EVNT_RPT]);
		out(buf);
		_os_snprintf(buf, len, " (WL FW report invalid!!)");
		out(buf);
	}

	for (i = 0; i < BTC_NCNT_MAX; i++)
		cnt_sum += dm->cnt_notify[i];

	_os_snprintf(buf, len, "\n\r %-15s : total=%d, show_coex_info=%d, power_on=%d, init_coex=%d, ",
		     "[notify_cnt]", cnt_sum, cnt[BTC_NCNT_SHOW_COEX_INFO],
		     cnt[BTC_NCNT_POWER_ON], cnt[BTC_NCNT_INIT_COEX]);
	out(buf);

	_os_snprintf(buf, len, "power_off=%d, radio_state=%d, role_info=%d, wl_rfk=%d, wl_sta=%d",
		     cnt[BTC_NCNT_POWER_OFF], cnt[BTC_NCNT_RADIO_STATE],
		     cnt[BTC_NCNT_ROLE_INFO], cnt[BTC_NCNT_WL_RFK],
		     cnt[BTC_NCNT_WL_STA]);
	out(buf);

	_os_snprintf(buf, len, "\n\r %-15s : scan_start=%d, scan_finish=%d, switch_band=%d, special_pkt=%d, ",
		     "[notify_cnt]", cnt[BTC_NCNT_SCAN_START],
		     cnt[BTC_NCNT_SCAN_FINISH], cnt[BTC_NCNT_SWITCH_BAND],
		     cnt[BTC_NCNT_SPECIAL_PACKET]);
	out(buf);

	_os_snprintf(buf, len, "timer=%d, control=%d, customerize=%d",
		     cnt[BTC_NCNT_TIMER], cnt[BTC_NCNT_CONTROL],
		     cnt[BTC_NCNT_CUSTOMERIZE]);
	out(buf);
}

void _disp_mreg(struct btc_t *btc, struct btf_fwinfo *pfwinfo,
		       void (*out)(const char *))
{
	struct btc_rpt_cmn_info *pcinfo = NULL;
	struct fbtc_mreg_val *pmreg = NULL;
	struct fbtc_gpio_dbg *gdbg = NULL;
	struct rtw_hal_com_t *h = btc->hal;
	struct btc_cx *cx = &btc->cx;
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_bt_info *bt = &btc->cx.bt;
	struct btc_gnt_ctrl gnt[2] = {0};
	char *buf = btc->mbuf;
	size_t len = btc->mlen;
	u8 i = 0, type = 0, cnt = 0;
	u32 val, offset;

	if (!(btc->dm.coex_info_map & BTC_COEX_INFO_MREG))
		return;

	_os_snprintf(buf, len, "%s", "\n\r========== [HW Status] ==========");
	out(buf);

	_os_snprintf(buf, len, "\n\r %-15s : WL->BT:0x%08x(cnt:%d), BT->WL:0x%08x(cnt:%d)",
		     "[scoreboard]", wl->scbd, cx->cnt_wl[BTC_WCNT_SCBDUPDATE],
		     bt->scbd, cx->cnt_bt[BTC_BCNT_SCBDUPDATE]);
	out(buf);

	rtw_hal_mac_get_grant(h, (u8*)gnt);

	_os_snprintf(buf, len, "\n\r %-15s : pta_owner:%s, phy-0[gnt_wl:%s-%d/gnt_bt:%s-%d], ",
		     "[gnt_status]",
		     (_read_cx_ctrl(btc) == BTC_CTRL_BY_WL? "WL" : "BT"),
		     (gnt[0].gnt_wl_sw_en? "SW" : "HW"), gnt[0].gnt_wl,
		     (gnt[0].gnt_bt_sw_en? "SW" : "HW"), gnt[0].gnt_bt);
	out(buf);

	_os_snprintf(buf, len, "phy-1[gnt_wl:%s-%d/gnt_bt:%s-%d]",
		     (gnt[1].gnt_wl_sw_en? "SW" : "HW"), gnt[1].gnt_wl,
		     (gnt[1].gnt_bt_sw_en? "SW" : "HW"), gnt[1].gnt_bt);
	out(buf);

	pcinfo = &pfwinfo->rpt_fbtc_mregval.cinfo;
	if (!pcinfo->valid)
		return;

	pmreg = &pfwinfo->rpt_fbtc_mregval.finfo;

	for (i = 0; i < pmreg->reg_num; i++) {
		type = (u8)btc->chip->mon_reg[i].type;
		offset = btc->chip->mon_reg[i].offset;
		val = pmreg->mreg_val[i];

		if (cnt % 6 == 0)
			_os_snprintf(buf, len, "\n\r %-15s : %s_0x%x=0x%x",
				     "[reg]",
				     id_to_str(BTC_STR_REG, (u32)type),
				     offset, val);
	 	else
			_os_snprintf(buf, len, ", %s_0x%x=0x%x",
				     id_to_str(BTC_STR_REG, (u32)type),
				     offset, val);
		out(buf);
		cnt++;
	}

	pcinfo = &pfwinfo->rpt_fbtc_gpio_dbg.cinfo;
	if (!pcinfo->valid)
		return;

	gdbg = &pfwinfo->rpt_fbtc_gpio_dbg.finfo;
	if (!gdbg->en_map)
		return;

	_os_snprintf(buf, len, "\n\r %-15s : enable_map:0x%08x",
		     "[gpio_dbg]", gdbg->en_map);
	out(buf);

	for (i = 0; i < BTC_DBG_MAX1; i++) {
		if (!(gdbg->en_map & BIT(i)))
			continue;
		_os_snprintf(buf, len, ", %s->GPIO%d",
			     id_to_str(BTC_STR_GDBG, (u32)i),
			     gdbg->gpio_map[i]);
		out(buf);
	}
}

void _disp_dm_info(struct btc_t *btc, void (*out)(const char *))
{
	struct btc_module *module = &btc->mdinfo;
	struct btc_dm *dm = &btc->dm;
	size_t len = btc->mlen;
	char *buf = btc->mbuf;

	if (!(dm->coex_info_map & BTC_COEX_INFO_DM))
		return;

	_os_snprintf(buf, len, "\n\r========== [Mechanism Status %s] ==========",
		     (btc->ctrl.manual? "(Manual)":"(Auto)"));
	out(buf);

	_os_snprintf(buf, len, "\n\r %-15s : type:%s, reason:%s(), action:%s(), ant_path:%s, run_cnt:%d",
		     "[status]",
		     (module->ant.type == BTC_ANT_SHARED ?
		      "shared" : "dedicated"),
		     dm->run_reason, dm->run_action,
		     id_to_str(BTC_STR_ANTPATH, dm->set_ant_path & 0xff),
		     dm->cnt_dm[BTC_DCNT_RUN]);
	out(buf);

	_os_snprintf(buf, len, "\n\r %-15s : wl_only:%d, bt_only:%d, igno_bt:%d, free_run:%d, wl_ps_ctrl:%d, wl_mimo_ps:%d, ",
		     "[dm_flag]", dm->wl_only, dm->bt_only, btc->ctrl.igno_bt,
		     dm->freerun, dm->wl_ps_ctrl, dm->wl_mimo_ps);
	out(buf);

	_os_snprintf(buf, len, "leak_ap:%d, noisy_level:%d, fw_offload:%s%s",
		     dm->leak_ap, dm->noisy_level,
		     (BTC_CX_FW_OFFLOAD? "Y" : "N"),
		     (dm->wl_fw_cx_offload == BTC_CX_FW_OFFLOAD?
		     "" : "(Mis-Match!!)"));
	out(buf);

	if (dm->rf_trx_para.wl_tx_power == 0xff)
		_os_snprintf(buf, len, "\n\r %-15s : wl_tx_pwr:original, ",
			     "[trx_ctrl]");

	else
		_os_snprintf(buf, len, "\n\r %-15s : wl_tx_pwr:%d, ",
			     "[trx_ctrl]", dm->rf_trx_para.wl_tx_power);

	out(buf);

	_os_snprintf(buf, len, "wl_rx_lvl:%d, bt_tx_pwr_dec:%d, bt_rx_lna:%d, ",
		     dm->rf_trx_para.wl_rx_gain, dm->rf_trx_para.bt_tx_power,
		     dm->rf_trx_para.bt_rx_gain);
	out(buf);

	_os_snprintf(buf, len, "wl_btg_rx:%d, wl_tx_limit[en:%d/max_time:%dus/max_retry:%d]",
		     dm->wl_btg_rx, dm->wl_tx_limit.enable,
		     dm->wl_tx_limit.tx_time, dm->wl_tx_limit.tx_retry);
	out(buf);
}

void _disp_cx_info(struct btc_t *btc, void (*out)(const char *))
{
	struct btc_dm *dm = &btc->dm;
	struct btc_bt_info *bt = &btc->cx.bt;
	struct btc_wl_info *wl = &btc->cx.wl;
	struct rtw_hal_com_t *h = btc->hal;
	size_t len = btc->mlen;
	char *buf = btc->mbuf;
	u32 ver_main = 0, ver_sub = 0, ver_hotfix = 0, id_branch = 0;

	if (!(dm->coex_info_map & BTC_COEX_INFO_CX))
		return;

	_os_snprintf(buf, len, "\n\r========== [BTC COEX INFO (%s)] ==========",
		     id_to_str(BTC_STR_CHIPID, (u32)btc->chip->chip_id));
	out(buf);

	ver_main = (coex_ver & bMASKB3) >> 24;
	ver_sub = (coex_ver & bMASKB2) >> 16;
	ver_hotfix = (coex_ver & bMASKB1) >> 8;
	id_branch = coex_ver & bMASKB0;

	_os_snprintf(buf, len, "\n\r %-15s : Coex:%d.%d.%d(branch:%s), ",
		     "[coex_version]", ver_main, ver_sub, ver_hotfix,
		     id_to_str(BTC_STR_BRANCH, id_branch));
	out(buf);

	if (dm->wl_fw_cx_offload != BTC_CX_FW_OFFLOAD)
		dm->error.map.offload_mismatch = true;
	else
		dm->error.map.offload_mismatch = false;

	ver_main = (wl->ver_info.fw_coex & bMASKB3) >> 24;
	ver_sub = (wl->ver_info.fw_coex & bMASKB2) >> 16;
	ver_hotfix = (wl->ver_info.fw_coex & bMASKB1) >> 8;
	id_branch = wl->ver_info.fw_coex & bMASKB0;

	_os_snprintf(buf, len, "WL_FW_coex:%d.%d.%d(branch:%s)",
		     ver_main, ver_sub, ver_hotfix,
		     id_to_str(BTC_STR_BRANCH, id_branch));
	out(buf);

	ver_main = (btc->chip->wlcx_desired & bMASKB3) >> 24;
	ver_sub = (btc->chip->wlcx_desired & bMASKB2) >> 16;
	ver_hotfix = (btc->chip->wlcx_desired & bMASKB1) >> 8;

	_os_snprintf(buf, len, "(%s, desired:%d.%d.%d), ",
		     (wl->ver_info.fw_coex >= btc->chip->wlcx_desired ?
		     "Match" : "Mis-Match"),
		     ver_main, ver_sub, ver_hotfix);
	out(buf);

	_os_snprintf(buf, len, "BT_FW_coex:%d.0.0(%s, desired:%d.0.0)",
		     bt->ver_info.fw_coex,
		     (bt->ver_info.fw_coex >= btc->chip->btcx_desired ?
		     "Match" : "Mis-Match"),
		     btc->chip->btcx_desired);
	out(buf);

	if (bt->enable.now && bt->ver_info.fw == 0)
		hal_btc_fw_en_rpt(btc, RPT_EN_BT_VER_INFO, 1);
	else
		hal_btc_fw_en_rpt(btc, RPT_EN_BT_VER_INFO, 0);

	_os_snprintf(buf, len, "\n\r %-15s : WL_FW:%d.%d.%d.%d, BT_FW:0x%x(%s)",
		     "[sub_module]", (wl->ver_info.fw & bMASKB3) >> 24,
		     (wl->ver_info.fw & bMASKB2) >> 16,
		     (wl->ver_info.fw & bMASKB1) >> 8,
		     (wl->ver_info.fw & bMASKB0), bt->ver_info.fw,
		     (bt->run_patch_code ? "patch" : "ROM"));
	out(buf);

#if 0 /* TODO: revise for u64 */
	_os_snprintf(buf, len, ", HAL_MAC:0x%x, HAL_BB:0x%x, HAL_RF:0x%x",
		     btc->hal->mac_vc.mac_ver, btc->hal->bb_vc.bb_ver,
		     btc->hal->rf_vc.rf_ver);
	out(buf);
#endif
	_os_snprintf(buf, len, "\n\r %-15s : kt_ver:%x, rfe_type:0x%x, ant_iso:%d, ant_pg:%d, %s",
		     "[hw_info]", btc->mdinfo.kt_ver, btc->mdinfo.rfe_type,
		     btc->mdinfo.ant.isolation, btc->mdinfo.ant.num,
		     (btc->mdinfo.ant.num > 1? "" : (btc->mdinfo.ant.single_pos?
		     "1Ant_Pos:S1, " : "1Ant_Pos:S0, ")));
	out(buf);

	_os_snprintf(buf, len, "3rd_coex:%d, dbcc:%d, tx_num:%d, rx_num:%d",
		     btc->cx.other.type, h->dbcc_en, h->rfpath_tx_num,
		     h->rfpath_rx_num);
	out(buf);
}

void _disp_wl_role_info(struct btc_t *btc, void (*out)(const char *))
{
	struct btc_wl_link_info *plink = NULL;
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_wl_dbcc_info *wl_dinfo = &wl->dbcc_info;
	struct btc_traffic t;
	char *buf = btc->mbuf;
	size_t len = btc->mlen;
	u8 i;

	if (btc->hal->dbcc_en) {
		_os_snprintf(buf, len, "\n\r %-15s : PHY0_band(op:%d/scan:%d/real:%d), ",
			     "[dbcc_info]", wl_dinfo->op_band[HW_PHY_0],
			     wl_dinfo->scan_band[HW_PHY_0],
			     wl_dinfo->real_band[HW_PHY_0]);
		out(buf);
		_os_snprintf(buf, len, "PHY1_band(op:%d/scan:%d/real:%d)",
			     wl_dinfo->op_band[HW_PHY_1],
			     wl_dinfo->scan_band[HW_PHY_1],
			     wl_dinfo->real_band[HW_PHY_1]);
		out(buf);
	}

	for (i = 0; i < MAX_WIFI_ROLE_NUMBER; i++) {
		plink = &btc->cx.wl.link_info[i];

		if (!plink->active)
			continue;

		_os_snprintf(buf, len, "\n\r [port_%d]        : role=%s(phy-%d), connect=%s, mode=%s, center_ch=%d, bw=%s, ",
			     plink->pid,
			     id_to_str(BTC_STR_ROLE, (u32)plink->role),
			     plink->phy,
			     id_to_str(BTC_STR_MSTATE, (u32)plink->connected),
			     id_to_str(BTC_STR_WLMODE, (u32)plink->mode),
			     plink->ch,
			     id_to_str(BTC_STR_WLBW, (u32)plink->bw));
		out(buf);

		_os_snprintf(buf, len, "mac_id=%d, max_tx_time=%dus, max_tx_retry=%d",
			     plink->mac_id, plink->tx_time, plink->tx_retry);
		out(buf);

		_os_snprintf(buf, len, "\n\r [port_%d]        : rssi=-%ddBm(%d), busy=%d, dir=%s, ",
			     plink->pid, 110-plink->stat.rssi,
			     plink->stat.rssi, plink->busy,
			     (plink->dir == TRAFFIC_UL ? "UL" : "DL"));
		out(buf);

		t = plink->stat.traffic;

		_os_snprintf(buf, len, "tx[rate:%s/busy_level:%d/sts:0x%x], ",
			     id_to_str(BTC_STR_RATE, (u32)t.tx_rate),
			     t.tx_lvl, t.tx_sts);
		out(buf);

		_os_snprintf(buf, len, "rx[rate:%s/busy_level:%d/sts:0x%x]",
			     id_to_str(BTC_STR_RATE, (u32)t.rx_rate),
			     t.rx_lvl, t.rx_sts);
		out(buf);
	}
}

void _disp_wl_info(struct btc_t *btc, void (*out)(const char *))
{
	struct btc_cx *cx = &btc->cx;
	struct btc_wl_info *wl = &cx->wl;
	struct btc_wl_role_info *wl_rinfo = &wl->role_info;
	size_t len = btc->mlen;
	char *buf = btc->mbuf;

	if (!(btc->dm.coex_info_map & BTC_COEX_INFO_WL))
		return;

	_os_snprintf(buf, len, "%s", "\n\r========== [WL Status] ==========");
	out(buf);

	_os_snprintf(buf, len, "\n\r %-15s : link_mode:%s, ",
		     "[status]",
		     id_to_str(BTC_STR_WLLINK, (u32)wl_rinfo->link_mode));
	out(buf);

	_os_snprintf(buf, len, "rf_off:%s, scan:%s(band:%d/phy_map:0x%x), ",
		     (wl->status.map.rf_off? "Y" : "N"),
		     (wl->status.map.scan? "Y" : "N"),
		     wl->scan_info.band[HW_PHY_0], wl->scan_info.phy_map);
	out(buf);

	_os_snprintf(buf, len, "connecting:%s, roam:%s, 4way:%s, init_ok:%s",
		     (wl->status.map.connecting? "Y" : "N"),
		     (wl->status.map.roaming?  "Y" : "N"),
		     (wl->status.map._4way? "Y" : "N"),
		     (wl->status.map.init_ok? "Y" : "N"));
	out(buf);

	_disp_wl_role_info(btc, out);
}

void _disp_bt_profile_info(struct btc_t *btc, void (*out)(const char *))
{
	struct btc_bt_link_info *bt_linfo = &btc->cx.bt.link_info;
	struct btc_bt_hfp_desc hfp = bt_linfo->hfp_desc;
	struct btc_bt_hid_desc hid = bt_linfo->hid_desc;
	struct btc_bt_a2dp_desc a2dp = bt_linfo->a2dp_desc;
	struct btc_bt_pan_desc pan = bt_linfo->pan_desc;
	size_t len = btc->mlen;
	char *buf = btc->mbuf;

	if (hfp.exist) {
		_os_snprintf(buf, len, "\n\r %-15s : type:%s, sut_pwr:%d, golden-rx:%d",
			     "[HFP]",
			     (hfp.type == 0? "SCO" : "eSCO"),
			      bt_linfo->sut_pwr_level[0],
			      bt_linfo->golden_rx_shift[0]);
		out(buf);
	}

	if (hid.exist) {
		_os_snprintf(buf, len, "\n\r %-15s : type:%s%s%s%s%s pair-cnt:%d, sut_pwr:%d, golden-rx:%d",
			     "[HID]",
			     (hid.type & BTC_HID_218? "2/18," : ""),
			     (hid.type & BTC_HID_418? "4/18," : ""),
			     (hid.type & BTC_HID_BLE? "BLE," : ""),
			     (hid.type & BTC_HID_RCU? "RCU," : ""),
			     (hid.type & BTC_HID_RCU_VOICE? "RCU-Voice," : ""),
			      hid.pair_cnt, bt_linfo->sut_pwr_level[1],
			      bt_linfo->golden_rx_shift[1]);
		out(buf);
	}

	if (a2dp.exist) {
		_os_snprintf(buf, len, "\n\r %-15s : type:%s, bit-pool:%d, flush-time:%d, ",
			     "[A2DP]",
			     (a2dp.type == BTC_A2DP_LEGACY ? "Legacy" : "TWS"),
			      a2dp.bitpool, a2dp.flush_time);
		out(buf);

		_os_snprintf(buf, len, "vid:0x%x, Dev-name:0x%x, sut_pwr:%d, golden-rx:%d",
			     a2dp.vendor_id, a2dp.device_name,
			     bt_linfo->sut_pwr_level[2],
			     bt_linfo->golden_rx_shift[2]);
		out(buf);
	}

	if (pan.exist) {
		_os_snprintf(buf, len, "\n\r %-15s : sut_pwr:%d, golden-rx:%d",
			     "[PAN]",
			     bt_linfo->sut_pwr_level[3],
			     bt_linfo->golden_rx_shift[3]);
		out(buf);
	}
}

void _disp_bt_info(struct btc_t *btc, void (*out)(const char *))
{
	struct btc_cx *cx = &btc->cx;
	struct btc_bt_info *bt = &cx->bt;
	struct btc_wl_info *wl = &cx->wl;
	struct btc_module *module = &btc->mdinfo;
	struct btc_bt_link_info *bt_linfo = &bt->link_info;
	u8 *afh = bt_linfo->afh_map;
	size_t len = btc->mlen;
	char *buf = btc->mbuf;
	u16 polt_cnt = 0;

	btc->chip->ops->update_bt_cnt(btc);
	_chk_err_state(btc, BTC_DCNT_BTCNT_FREEZE, 0);

	if (!(btc->dm.coex_info_map & BTC_COEX_INFO_BT))
		return;

	_os_snprintf(buf, len, "%s", "\n\r========== [BT Status] ==========");
	out(buf);

	/*bt->btg_type = (bt->ver_info.fw & BIT(28) ? BTC_BT_BTG: BTC_BT_ALONE);*/

	_os_snprintf(buf, len, "\n\r %-15s : enable:%s, btg:%s%s, connect:%s, ",
		     "[status]", (bt->enable.now? "Y" : "N"),
		     (bt->btg_type? "Y" : "N"),
		     (bt->enable.now && (bt->btg_type != module->bt_pos)?
		      "(efuse-mismatch!!)" : ""),
		     (bt_linfo->status.map.connect? "Y" : "N"));
	out(buf);

	_os_snprintf(buf, len, "igno_wl:%s, mailbox_avl:%s, rfk_state:0x%x",
		     (bt->igno_wl? "Y" : "N"),
		     (bt->mbx_avl? "Y" : "N"), bt->rfk_info.val);
	out(buf);

	_os_snprintf(buf, len, "\n\r %-15s : profile:%s%s%s%s%s ",
		     "[profile]",
		     ((bt_linfo->profile_cnt.now == 0) ? "None," : ""),
		     (bt_linfo->hfp_desc.exist? "HFP," : ""),
		     (bt_linfo->hid_desc.exist? "HID," : ""),
		     (bt_linfo->a2dp_desc.exist?
		     (bt_linfo->a2dp_desc.sink ? "A2DP_sink," :"A2DP,") : ""),
		     (bt_linfo->pan_desc.exist? "PAN," : ""));
	out(buf);

	_os_snprintf(buf, len, "multi-link:%s, role:%s, ble-connect:%s, CQDDR:%s, A2DP_active:%s, PAN_active:%s",
		     (bt_linfo->multi_link.now? "Y" : "N"),
		     (bt_linfo->slave_role ? "Slave" : "Master"),
		     (bt_linfo->status.map.ble_connect? "Y" : "N"),
		     (bt_linfo->cqddr? "Y" : "N"),
		     (bt_linfo->a2dp_desc.active? "Y" : "N"),
		     (bt_linfo->pan_desc.active? "Y" : "N"));
	out(buf);

	_os_snprintf(buf, len, "\n\r %-15s : rssi:%ddBm,%s%s%s",
		     "[link]", bt_linfo->rssi-100,
		     (bt_linfo->status.map.inq_pag? " inq-page!!" : ""),
		     (bt_linfo->status.map.acl_busy? " acl_busy!!" : ""),
		     (bt_linfo->status.map.mesh_busy? " mesh_busy!!" : ""));
	out(buf);

	_os_snprintf(buf, len, "%s afh_map[%02x%02x_%02x%02x_%02x%02x_%02x%02x_%02x%02x], ",
		     (bt_linfo->relink.now? " ReLink!!" : ""),
		     afh[0], afh[1], afh[2], afh[3], afh[4],
		     afh[5], afh[6], afh[7], afh[8], afh[9]);
	out(buf);

	_os_snprintf(buf, len, "wl_ch_map[en:%d/ch:%d/bw:%d/cnt:%d]",
		     wl->afh_info.en, wl->afh_info.ch, wl->afh_info.bw,
		     cx->cnt_wl[BTC_WCNT_CH_UPDATE]);
	out(buf);

	_os_snprintf(buf, len, "\n\r %-15s : retry:%d, relink:%d, reinit:%d, reenable:%d, ",
		     "[stat_cnt]", cx->cnt_bt[BTC_BCNT_RETRY],
		     cx->cnt_bt[BTC_BCNT_RELINK],
		     cx->cnt_bt[BTC_BCNT_REINIT],
		     cx->cnt_bt[BTC_BCNT_REENABLE]);
	out(buf);

	_os_snprintf(buf, len, "role-switch:%d, afh:%d, inq_page:%d(inq:%d/page:%d), igno_wl:%d",
		     cx->cnt_bt[BTC_BCNT_ROLESW],
		     cx->cnt_bt[BTC_BCNT_AFH],
		     cx->cnt_bt[BTC_BCNT_INQPAG],
		     cx->cnt_bt[BTC_BCNT_INQ],
		     cx->cnt_bt[BTC_BCNT_PAGE],
		     cx->cnt_bt[BTC_BCNT_IGNOWL]);
	out(buf);

	_disp_bt_profile_info(btc, out);

	_os_snprintf(buf, len, "\n\r %-15s : raw_data[%02x %02x %02x %02x %02x %02x] (type:%s/cnt:%d/same:%d)",
		     "[bt_info]",
		     bt->raw_info[2], bt->raw_info[3],
		     bt->raw_info[4], bt->raw_info[5],
		     bt->raw_info[6], bt->raw_info[7],
		     (bt->raw_info[0] == BTC_BTINFO_AUTO ? "auto" : "reply"),
		     cx->cnt_bt[BTC_BCNT_INFOUPDATE],
		     cx->cnt_bt[BTC_BCNT_INFOSAME]);
	out(buf);

	rtw_hal_mac_get_bt_polt_cnt(btc->hal, HW_PHY_0, &polt_cnt);
	_os_snprintf(buf, len, "\n\r %-15s : Hi-rx = %d, Hi-tx = %d, Lo-rx = %d, Lo-tx = %d (bt_polut_wl_tx = %d)",
		     "[trx_req_cnt]", cx->cnt_bt[BTC_BCNT_HIPRI_RX],
		     cx->cnt_bt[BTC_BCNT_HIPRI_TX],
		     cx->cnt_bt[BTC_BCNT_LOPRI_RX],
		     cx->cnt_bt[BTC_BCNT_LOPRI_TX],
		     polt_cnt);
	out(buf);
}

/*
 * local functions
 */
static void _disp_fbtc_slots(struct btc_t *btc, struct btf_fwinfo *pfwinfo,
			     char *buf, size_t len, void (*out)(const char *))
{
	struct btc_rpt_cmn_info *pcinfo = NULL;
	struct fbtc_slots *pslots = NULL;
	struct fbtc_slot s;
	u8 i = 0;

	pcinfo = &pfwinfo->rpt_fbtc_slots.cinfo;
	if (!pcinfo->valid)
		return;

	pslots = &pfwinfo->rpt_fbtc_slots.finfo;

	for (i = 0; i < CXST_MAX; i++) {
		s = pslots->slot[i];
		if (i % 6 == 0)
			_os_snprintf(buf, len, "\n\r %-15s : %s[%d/0x%x/%d]",
				     "[slot_list]",
				     id_to_str(BTC_STR_SLOT, (u32)i),
				     s.dur, s.cxtbl, s.cxtype);
		else
			_os_snprintf(buf, len, ", %s[%d/0x%x/%d]",
				     id_to_str(BTC_STR_SLOT, (u32)i),
				     s.dur, s.cxtbl, s.cxtype);
		out(buf);
	}
}

static void _disp_fbtc_tdma(struct btc_t *btc, struct btf_fwinfo *pfwinfo,
			    char *buf, size_t len, void (*out)(const char *))
{
	struct btc_rpt_cmn_info *pcinfo = NULL;
	struct fbtc_tdma *t = NULL;
	struct fbtc_slot *s = NULL;
	struct btc_dm *dm = &btc->dm;
	u8 i, cnt = 0;

	pcinfo = &pfwinfo->rpt_fbtc_tdma.cinfo;
	if (!pcinfo->valid)
		return;

	t = &pfwinfo->rpt_fbtc_tdma.finfo;

	_os_snprintf(buf, len, "\n\r %-15s : ", "[tdma_policy]");
	out(buf);
	_os_snprintf(buf, len, "type:%s, rx_flow_ctrl:%d, tx_pause:%d, ",
		     id_to_str(BTC_STR_TDMA, (u32)t->type),
		     t->rxflctrl, t->txpause);
	out(buf);
	_os_snprintf(buf, len, "wl_toggle_n:%d, leak_n:%d, ext_ctrl:%d, ",
		     t->wtgle_n, t->leak_n, t->ext_ctrl);
	out(buf);

	_os_snprintf(buf, len, "policy_type:%s",
		     id_to_str(BTC_STR_POLICY, (u32)btc->policy_type));
	out(buf);

	s = pfwinfo->rpt_fbtc_slots.finfo.slot;

	for (i = 0; i < CXST_MAX; i++) {
		if (dm->update_slot_map == BIT(CXST_MAX) - 1)
			break;

		if (!(dm->update_slot_map & BIT(i)))
			continue;

		if (cnt % 6 == 0)
			_os_snprintf(buf, len, "\n\r %-15s : %s[%d/0x%x/%d]",
				     "[slot_policy]",
				     id_to_str(BTC_STR_SLOT, (u32)i),
				     s[i].dur, s[i].cxtbl, s[i].cxtype);
		else
			_os_snprintf(buf, len, ", %s[%d/0x%x/%d]",
				     id_to_str(BTC_STR_SLOT, (u32)i),
				     s[i].dur, s[i].cxtbl, s[i].cxtype);
		out(buf);
		cnt++;
	}
}

static void _disp_fbtc_step(struct btc_t *btc, struct btf_fwinfo *pfwinfo,
			     char *buf, size_t len, void (*out)(const char *))
{
	struct btc_rpt_cmn_info *pcinfo = NULL;
	struct fbtc_steps *pstep = NULL;
	u8 i, type, val, cnt;
	u16 diff_t;

	pcinfo = &pfwinfo->rpt_fbtc_step.cinfo;
	if (!pcinfo->valid)
		return;

	pstep = &pfwinfo->rpt_fbtc_step.finfo;

	cnt = 0;
	for (i = 0; i < btc->ctrl.trace_step; i++) {
		type = pstep->step[i].type;
		val = pstep->step[i].val;
		diff_t = pstep->step[i].difft;

		if (type == CXSTEP_NONE || type >= CXSTEP_MAX)
			continue;

		if (cnt % 10 == 0) {
			_os_snprintf(buf, len, "\n\r [step_%d]        : ",
				     cnt/10+1);
			out(buf);
		}

		_os_snprintf(buf, len, "-> %s(%d)",
			     (type == CXSTEP_SLOT?
			     id_to_str(BTC_STR_SLOT, (u32)val) :
			     id_to_str(BTC_STR_EVENT, (u32)val)), diff_t);
		out(buf);
		cnt++;

		if (cnt == pstep->cnt)
			break;
	}
}


static void _disp_fbtc_cysta(struct btc_t *btc, struct btf_fwinfo *pfwinfo,
			     char *buf, size_t len, void (*out)(const char *))
{
	struct btc_dm *dm = &btc->dm;
	struct btc_bt_a2dp_desc *a2dp = &btc->cx.bt.link_info.a2dp_desc;
	struct btc_rpt_cmn_info *pcinfo = NULL;
	struct fbtc_cysta *pcysta = NULL;
	union fbtc_rxflct r;
	u8 i;

	pcinfo = &pfwinfo->rpt_fbtc_cysta.cinfo;
	if (!pcinfo->valid)
		return;

	pcysta = &pfwinfo->rpt_fbtc_cysta.finfo;
	_os_snprintf(buf, len, "\n\r %-15s : cycle:%d, bcn_erly:%d",
		     "[cycle_cnt]", pcysta->cycles, pcysta->brly_cnt);
	out(buf);

	_chk_err_state(btc, BTC_DCNT_CYCLE_FREEZE, (u32)pcysta->cycles);

	for (i = 0; i < CXST_MAX; i++) {
		if (!pcysta->slot_cnt[i])
			continue;
		_os_snprintf(buf, len, ", %s:%d",
			     id_to_str(BTC_STR_SLOT, (u32)i),
			     pcysta->slot_cnt[i]);
		out(buf);
	}

	if (dm->tdma_now.rxflctrl) {
		_os_snprintf(buf, len, ", leak_rx:%d", pcysta->leakrx_cnt);
		out(buf);
	}

	if (pcysta->collision_cnt) {
		_os_snprintf(buf, len, ", collision:%d", pcysta->collision_cnt);
		out(buf);
	}

	_chk_err_state(btc, BTC_DCNT_W1_FREEZE, pcysta->slot_cnt[CXST_W1]);
	_chk_err_state(btc, BTC_DCNT_B1_FREEZE, pcysta->slot_cnt[CXST_B1]);

	_os_snprintf(buf, len, "\n\r %-15s : avg_t[wl:%d/bt:%d/lk:%d.%03d]",
		     "[cycle_time]",
		     pcysta->tavg_cycle[CXT_WL],
		     pcysta->tavg_cycle[CXT_BT],
		     pcysta->tavg_lk/1000, pcysta->tavg_lk%1000);
	out(buf);
	_os_snprintf(buf, len, ", max_t[wl:%d/bt:%d/lk:%d.%03d]",
		     pcysta->tmax_cycle[CXT_WL],
		     pcysta->tmax_cycle[CXT_BT],
		     pcysta->tmax_lk/1000, pcysta->tmax_lk%1000);
	out(buf);
	_os_snprintf(buf, len, ", maxdiff_t[wl:%d/bt:%d]",
		     pcysta->tmaxdiff_cycle[CXT_WL],
		     pcysta->tmaxdiff_cycle[CXT_BT]);
	out(buf);

	if (a2dp->exist) {
		_os_snprintf(buf, len, "\n\r %-15s : a2dp_ept:%d, a2dp_late:%d",
			     "[a2dp_t_sta]",
			     pcysta->a2dpept, pcysta->a2dpeptto);
		out(buf);
		r.val = dm->tdma_now.rxflctrl;
		if (r.type && r.tgln_n) {
			_os_snprintf(buf, len, ", cycle[PSTDMA:%d/TDMA:%d], ",
				     pcysta->cycles_a2dp[CXT_FLCTRL_ON],
				     pcysta->cycles_a2dp[CXT_FLCTRL_OFF]);
			out(buf);

			_os_snprintf(buf, len, "avg_t[PSTDMA:%d/TDMA:%d], ",
				     pcysta->tavg_a2dp[CXT_FLCTRL_ON],
				     pcysta->tavg_a2dp[CXT_FLCTRL_OFF]);
			out(buf);

			_os_snprintf(buf, len, "max_t[PSTDMA:%d/TDMA:%d]",
				     pcysta->tmax_a2dp[CXT_FLCTRL_ON],
				     pcysta->tmax_a2dp[CXT_FLCTRL_OFF]);
			out(buf);
		}
	}
}

static void _disp_fbtc_nullsta(struct btc_t *btc, struct btf_fwinfo *pfwinfo,
			       char *buf, size_t len, void (*out)(const char *))
{
	struct btc_rpt_cmn_info *pcinfo = NULL;
	struct fbtc_cynullsta *ns = NULL;
	u8 i = 0;

	if (!btc->dm.tdma_now.rxflctrl)
		return;

	pcinfo = &pfwinfo->rpt_fbtc_nullsta.cinfo;
	if (!pcinfo->valid)
		return;

	ns = &pfwinfo->rpt_fbtc_nullsta.finfo;

	_os_snprintf(buf, len, "\n\r %-15s : ", "[null_sta]");
	out(buf);

	for (i = 0; i < 2; i++) {
		if (i != 0)
			_os_snprintf(buf, len, ", null-%d", i);
		else
			_os_snprintf(buf, len, "null-%d", i);
		out(buf);
		_os_snprintf(buf, len, "[ok:%d/", ns->result[i][1]);
		out(buf);
		_os_snprintf(buf, len, "fail:%d/", ns->result[i][0]);
		out(buf);
		_os_snprintf(buf, len, "on_time:%d/", ns->result[i][2]);
		out(buf);
		_os_snprintf(buf, len, "retry:%d/", ns->result[i][3]);
		out(buf);
		_os_snprintf(buf, len, "avg_t:%d.%03d/",
			     ns->avg_t[i]/1000, ns->avg_t[i]%1000);
		out(buf);
		_os_snprintf(buf, len, "max_t:%d.%03d]",
			     ns->max_t[i]/1000, ns->max_t[i]%1000);
		out(buf);
	}
}

static void _disp_error(struct btc_t *btc, struct btf_fwinfo *pfwinfo,
			char *buf, size_t len, void (*out)(const char *))
{
	struct fbtc_cysta *pcysta = NULL;

	pcysta = &pfwinfo->rpt_fbtc_cysta.finfo;

	if (pfwinfo->event[BTF_EVNT_BUF_OVERFLOW] == 0 &&
	    pcysta->except_cnt == 0 &&
	    !pfwinfo->len_mismch && !pfwinfo->fver_mismch)
	    return;

	_os_snprintf(buf, len, "\n\r %-15s : ", "[error]");
	out(buf);

	if (pfwinfo->event[BTF_EVNT_BUF_OVERFLOW]) {
		_os_snprintf(buf, len, "overflow-cnt: %d, ",
			     pfwinfo->event[BTF_EVNT_BUF_OVERFLOW]);
		out(buf);
	}

	if (pfwinfo->len_mismch) {
		_os_snprintf(buf, len, "len-mismatch: 0x%x, ",
			     pfwinfo->len_mismch);
		out(buf);
	}

	if (pfwinfo->fver_mismch) {
		_os_snprintf(buf, len, "fver-mismatch: 0x%x, ",
			     pfwinfo->fver_mismch);
		out(buf);
	}

	/* cycle statistics exceptions */
	if (pcysta->exception || pcysta->except_cnt) {
		_os_snprintf(buf, len, "exception-type: 0x%x, exception-cnt = %d",
			     pcysta->exception, pcysta->except_cnt);
		out(buf);
	}
}

static void _update_bt_report(struct btc_t *btc, u8 rpt_type, u8* pfinfo)
{
	struct rtw_hal_com_t *h = btc->hal;
	struct btc_bt_info *bt = &btc->cx.bt;
	struct btc_bt_link_info *bt_linfo = &bt->link_info;
	struct btc_bt_a2dp_desc *a2dp = &bt_linfo->a2dp_desc;

	struct fbtc_btver* pver = (struct fbtc_btver*) pfinfo;
	struct fbtc_btscan* pscan = (struct fbtc_btscan*) pfinfo;
	struct fbtc_btafh* pafh = (struct fbtc_btafh*) pfinfo;
	struct fbtc_btdevinfo* pdev = (struct fbtc_btdevinfo*) pfinfo;

	switch (rpt_type) {
	case BTC_RPT_TYPE_BT_VER:
		bt->ver_info.fw = pver->fw_ver;
		bt->ver_info.fw_coex = (pver->coex_ver & bMASKB0);
		bt->feature = pver->feature;
		break;
	case BTC_RPT_TYPE_BT_SCAN:
		hal_mem_cpy(h, bt->scan_info, pscan->scan, BTC_SCAN_MAX1);
		break;
	case BTC_RPT_TYPE_BT_AFH:
		hal_mem_cpy(h, &bt_linfo->afh_map[0], pafh->afh_l, 4);
		hal_mem_cpy(h, &bt_linfo->afh_map[4], pafh->afh_m, 4);
		hal_mem_cpy(h, &bt_linfo->afh_map[8], pafh->afh_h, 2);
		break;
	case BTC_RPT_TYPE_BT_DEVICE:
		a2dp->device_name = pdev->dev_name;
		a2dp->vendor_id = pdev->vendor_id;
		a2dp->flush_time = pdev->flush_time;
		break;
	default:
		break;
	}
}

static u8 _check_report(struct btc_t *btc, struct btf_fwinfo *pfwinfo,
			u8 *prptbuf, u32 index)
{
	struct rtw_hal_com_t *hal = btc->hal;
	struct btc_rpt_cmn_info *pcinfo = NULL;
	u8 rpt_type = 0;
	u16 trace_step = btc->ctrl.trace_step;
	u8 *rpt_content = NULL, *pfinfo = NULL;
	u32 rpt_len = 0;

	if (!prptbuf) {
		pfwinfo->err[BTFRE_INVALID_INPUT]++;
		return 0;
	}

	rpt_type = prptbuf[index];
	rpt_len = (prptbuf[index+2] << 8) + prptbuf[index+1];
	rpt_content = &prptbuf[index+3];

	switch (rpt_type) {
	case BTC_RPT_TYPE_CTRL:
		pcinfo = &pfwinfo->rpt_ctrl.cinfo;
		pfinfo = (u8 *)(&pfwinfo->rpt_ctrl.finfo);
		pcinfo->req_len = sizeof(struct fbtc_rpt_ctrl);
		pcinfo->req_fver = BTCRPT_VER;
		pcinfo->rx_len = rpt_len;
		pcinfo->rx_cnt++;
		break;
	case BTC_RPT_TYPE_TDMA:
		pcinfo = &pfwinfo->rpt_fbtc_tdma.cinfo;
		pfinfo = (u8 *)(&pfwinfo->rpt_fbtc_tdma.finfo);
		pcinfo->req_len = sizeof(struct fbtc_tdma);
		pcinfo->req_fver = FCXTDMA_VER;
		pcinfo->rx_len = rpt_len;
		pcinfo->rx_cnt++;
		break;
	case BTC_RPT_TYPE_SLOT:
		pcinfo = &pfwinfo->rpt_fbtc_slots.cinfo;
		pfinfo = (u8 *)(&pfwinfo->rpt_fbtc_slots.finfo);
		pcinfo->req_len = sizeof(struct fbtc_slots);
		pcinfo->req_fver = FCXSLOTS_VER;
		pcinfo->rx_len = rpt_len;
		pcinfo->rx_cnt++;
		break;
	case BTC_RPT_TYPE_CYSTA:
		pcinfo = &pfwinfo->rpt_fbtc_cysta.cinfo;
		pfinfo = (u8 *)(&pfwinfo->rpt_fbtc_cysta.finfo);
		pcinfo->req_len = sizeof(struct fbtc_cysta);
		pcinfo->req_fver = FCXCYSTA_VER;
		pcinfo->rx_len = rpt_len;
		pcinfo->rx_cnt++;
		break;
	case BTC_RPT_TYPE_STEP:
		pcinfo = &pfwinfo->rpt_fbtc_step.cinfo;
		pfinfo = (u8 *)(&pfwinfo->rpt_fbtc_step.finfo);
		pcinfo->req_len = 4 + sizeof(struct fbtc_step) * trace_step;
		pcinfo->req_fver = FCXSTEP_VER;
		pcinfo->rx_len = rpt_len;
		pcinfo->rx_cnt++;
		break;
	case BTC_RPT_TYPE_NULLSTA:
		pcinfo = &pfwinfo->rpt_fbtc_nullsta.cinfo;
		pfinfo = (u8 *)(&pfwinfo->rpt_fbtc_nullsta.finfo);
		pcinfo->req_len = sizeof(struct fbtc_cynullsta);
		pcinfo->req_fver = FCXNULLSTA_VER;
		pcinfo->rx_len = rpt_len;
		pcinfo->rx_cnt++;
		break;
	case BTC_RPT_TYPE_MREG:
		pcinfo = &pfwinfo->rpt_fbtc_mregval.cinfo;
		pfinfo = (u8 *)(&pfwinfo->rpt_fbtc_mregval.finfo);
		pcinfo->req_len = sizeof(struct fbtc_mreg_val);
		pcinfo->req_fver = FCXMREG_VER;
		pcinfo->rx_len = rpt_len;
		pcinfo->rx_cnt++;
		break;
	case BTC_RPT_TYPE_GPIO_DBG:
		pcinfo = &pfwinfo->rpt_fbtc_gpio_dbg.cinfo;
		pfinfo = (u8 *)(&pfwinfo->rpt_fbtc_gpio_dbg.finfo);
		pcinfo->req_len = sizeof(struct fbtc_gpio_dbg);
		pcinfo->req_fver = FCXGPIODBG_VER;
		pcinfo->rx_len = rpt_len;
		pcinfo->rx_cnt++;
		break;
	case BTC_RPT_TYPE_BT_VER:
		pcinfo = &pfwinfo->rpt_fbtc_btver.cinfo;
		pfinfo = (u8 *)(&pfwinfo->rpt_fbtc_btver.finfo);
		pcinfo->req_len = sizeof(struct fbtc_btver);
		pcinfo->req_fver = FCX_BTVER_VER;
		pcinfo->rx_len = rpt_len;
		pcinfo->rx_cnt++;
		break;
	case BTC_RPT_TYPE_BT_SCAN:
		pcinfo = &pfwinfo->rpt_fbtc_btscan.cinfo;
		pfinfo = (u8 *)(&pfwinfo->rpt_fbtc_btscan.finfo);
		pcinfo->req_len = sizeof(struct fbtc_btscan);
		pcinfo->req_fver = FCX_BTSCAN_VER;
		pcinfo->rx_len = rpt_len;
		pcinfo->rx_cnt++;
		break;
	case BTC_RPT_TYPE_BT_AFH:
		pcinfo = &pfwinfo->rpt_fbtc_btafh.cinfo;
		pfinfo = (u8 *)(&pfwinfo->rpt_fbtc_btafh.finfo);
		pcinfo->req_len = sizeof(struct fbtc_btafh);
		pcinfo->req_fver = FCX_BTAFH_VER;
		pcinfo->rx_len = rpt_len;
		pcinfo->rx_cnt++;
		break;
	case BTC_RPT_TYPE_BT_DEVICE:
		pcinfo = &pfwinfo->rpt_fbtc_btdev.cinfo;
		pfinfo = (u8 *)(&pfwinfo->rpt_fbtc_btdev.finfo);
		pcinfo->req_len = sizeof(struct fbtc_btdevinfo);
		pcinfo->req_fver = FCX_BTDEVINFO_VER;
		pcinfo->rx_len = rpt_len;
		pcinfo->rx_cnt++;
		break;
	default:
		pfwinfo->err[BTFRE_UNDEF_TYPE]++;
		return 0;
	}

	if (rpt_len != pcinfo->req_len) {
		if (rpt_type < BTC_RPT_TYPE_MAX)
			pfwinfo->len_mismch |= (0x1 << rpt_type);
		else
			pfwinfo->len_mismch |= BIT31;
		PHL_INFO("[BTC], rpt_type=%d, rpt_len=%d, req_len=%d \n",
			 rpt_type, rpt_len, pcinfo->req_len);
		pcinfo->valid = 0;
		return 0;
	} else if (!pfinfo || !rpt_content || !pcinfo->req_len) {
		pfwinfo->err[BTFRE_EXCEPTION]++;
		pcinfo->valid = 0;
		return 0;
	}


	hal_mem_cpy(hal, (void *)pfinfo, (void *)rpt_content, pcinfo->req_len);
	pcinfo->valid = 1;

	if (rpt_type == BTC_RPT_TYPE_TDMA) {
#if BTC_CX_FW_OFFLOAD /* update tdma_now if fw offload for debug */
		_tdma_cpy(&btc->dm.tdma_now, &pfwinfo->rpt_fbtc_tdma.finfo);
#else
		_chk_err_state(btc, BTC_DCNT_TDMA_NONSYNC,
			       _tdma_cmp(&btc->dm.tdma_now,
				   	 &pfwinfo->rpt_fbtc_tdma.finfo));
#endif
	}

	if (rpt_type == BTC_RPT_TYPE_SLOT) {
#if BTC_CX_FW_OFFLOAD /* update slot_now if fw offload for debug */
		_slots_cpy(btc->dm.slot_now,
			   pfwinfo->rpt_fbtc_slots.finfo.slot);
#else
		_chk_err_state(btc, BTC_DCNT_SLOT_NONSYNC,
			       _tdma_cmp(btc->dm.slot_now,
					 pfwinfo->rpt_fbtc_slots.finfo.slot));
#endif
	}

	if (rpt_type >= BTC_RPT_TYPE_BT_VER &&
	    rpt_type <= BTC_RPT_TYPE_BT_DEVICE)
		_update_bt_report(btc, rpt_type, pfinfo);

	return (u8)(rpt_len + BTC_RPT_HDR_SIZE);
}

static void _parse_report(struct btc_t *btc, struct btf_fwinfo *pfwinfo,
			  u8 *pbuf, u32 buf_len)
{
	u32 index = 0;
	u8 rpt_len = 0;

	while (pbuf) {
		/* At least 2 bytes: type & len */
		if ((index + pbuf[index+1] + BTC_RPT_HDR_SIZE) > buf_len)
			break;

		rpt_len = _check_report(btc, pfwinfo, pbuf, index);
		if (!rpt_len)
			break;
		index += rpt_len;
	}
}

static void _parse_policy(struct btc_t *btc, u16 len)
{
	struct btf_tlv *tlv = NULL;
	struct fbtc_1slot *v = NULL;
	u16 i = 0;

	if (!btc->policy || len <= 2)
		return;

	PHL_INFO("[BTC], policy parsing, len =%d \n", len);
	while (1) {
		tlv = (struct btf_tlv *)&btc->policy[i];
		v = (struct fbtc_1slot *)&tlv->val[0];
		PHL_INFO("[BTC], type=%d, len=%d \n", tlv->type, tlv->len);

		i += (2 + tlv->len);
		if (i >= len)
			break;
	}
}

#if 0
static void _append_policy(struct btc_t *btc, bool force_exec, u16 policy_type)
{
	if (!force_exec && policy_type == btc->policy_type) {
		PHL_INFO("[BTC], %s: policy type no change (type=%d)\n",
			 __func__, policy_type);
		return;
	}

	btc->policy[0] = CXPOLICY_TYPE;
	btc->policy[1] = sizeof(u16);
	btc->policy[2] = (u8)(policy_type & bMASKB0);
	btc->policy[3] = (u8)((policy_type & bMASKB1) >> 8);
	btc->policy_len = 4;
}
#endif

static void _append_tdma(struct btc_t *btc, bool force_exec)
{
	struct btc_dm *dm = &btc->dm;
	struct btf_tlv *tlv = NULL;
	struct fbtc_tdma *v = NULL;
	u16 len = btc->policy_len;

	if (!force_exec && !_tdma_cmp(&dm->tdma, &dm->tdma_now)) {
		/* PHL_INFO("[BTC], %s: tdma no change!\n", __func__); */
		return;
	}

	tlv = (struct btf_tlv *)&btc->policy[len];
	tlv->type = CXPOLICY_TDMA;
	tlv->len = sizeof(struct fbtc_tdma);
	v = (struct fbtc_tdma *)&tlv->val[0];

	_tdma_cpy(v, &dm->tdma);

	btc->policy_len = len + 2 + sizeof(struct fbtc_tdma);

	PHL_INFO("[BTC], %s: type:%d, rxflctrl=%d, txpause=%d, wtgle_n=%d, leak_n=%d, ext_ctrl=%d\n",
		 __func__, dm->tdma.type, dm->tdma.rxflctrl, dm->tdma.txpause,
		 dm->tdma.wtgle_n, dm->tdma.leak_n, dm->tdma.ext_ctrl);

	PHL_INFO("[BTC], %s: tdma update!!\n", __func__);
}

static void _append_slot(struct btc_t *btc, bool force_exec)
{
	struct btc_dm *dm = &btc->dm;
	struct btf_tlv *tlv = NULL;
	struct fbtc_1slot *v = NULL;
	u16 len = 0;
	u8 i, cnt = 0;

	for (i = 0; i < CXST_MAX; i++) {
		if (!force_exec && !_slot_cmp(&dm->slot[i], &dm->slot_now[i]))
			continue;

		len = btc->policy_len;

		tlv = (struct btf_tlv *)&btc->policy[len];
		tlv->type = CXPOLICY_SLOT;
		tlv->len = sizeof(struct fbtc_1slot);
		v = (struct fbtc_1slot *)&tlv->val[0];

		v->fver = FCXONESLOT_VER;
		v->sid = i;
		_slot_cpy(&v->slot, &dm->slot[i]);

		PHL_INFO("[BTC], %s: slot-%d: dur=%d, table=0x%08x, type=%d\n",
			 __func__, i,dm->slot[i].dur, dm->slot[i].cxtbl,
			 dm->slot[i].cxtype);
		cnt++;
		btc->policy_len = len + 2 + sizeof(struct fbtc_1slot);
	}

	if (cnt > 0)
		PHL_INFO("[BTC], %s: slot update (cnt=%d)!!\n", __func__, cnt);
}

/*
 * extern functions
 */
void hal_btc_fw_dm_msg(struct btc_t *btc, struct btf_fwinfo *pfwinfo,
		       void (*out)(const char *))
{
	char *buf = btc->mbuf;
	size_t len = btc->mlen;

	if (!(btc->dm.coex_info_map & BTC_COEX_INFO_DM))
		return;

	_disp_error(btc, pfwinfo, buf, len, out);
	_disp_fbtc_tdma(btc, pfwinfo, buf, len, out);
	_disp_fbtc_slots(btc, pfwinfo, buf, len, out);
	_disp_fbtc_cysta(btc, pfwinfo, buf, len, out);
	_disp_fbtc_nullsta(btc, pfwinfo, buf, len, out);
	_disp_fbtc_step(btc, pfwinfo, buf, len, out);
}

#endif // if 0 NEO

void hal_btc_fw_en_rpt(struct btc_t *btc, u32 rpt_map, u32 rpt_state)
{
	RTW_ERR("%s NEO TODO\n", __func__);
#if 0 // NEO
	struct btc_ops *ops = btc->ops;
	struct btf_set_report r = {0};
	struct btf_fwinfo* fwinfo = &btc->fwinfo;
	u32 val = 0;
	u8 en;

	if (!ops || !ops->fw_cmd)
		return;

	en = rpt_state & 0x1;
	if (en)
		val = fwinfo->rpt_en_map | rpt_map;
	else
		val = fwinfo->rpt_en_map & (~rpt_map);

	if (val == fwinfo->rpt_en_map)
		return;

	fwinfo->rpt_en_map = val;

	r.fver = BTF_SET_REPORT_VER;
	r.enable = val;
	r.para = en;

	ops->fw_cmd(btc, BTFC_SET, SET_REPORT_EN, (u8 *)&r, sizeof(r));
#endif // if 0
}


void hal_btc_fw_set_slots(struct btc_t *btc, u8 num, struct fbtc_slot *s)
{
	RTW_ERR("%s NEO TODO\n", __func__);
#if 0 // NEO

#if !BTC_CX_FW_OFFLOAD
	struct rtw_hal_com_t *h = btc->hal;
	struct btc_ops *ops = btc->ops;
	struct btf_set_slot_table *tbl = NULL;
	u8 *ptr = NULL;
	u16 n = 0;

	if (!ops || !ops->fw_cmd)
		return;

	n = (sizeof(struct fbtc_slot) * num) + sizeof(*tbl) - 1;
	tbl = hal_mem_alloc(h, n);
	if (!tbl)
		return;

	tbl->fver = BTF_SET_SLOT_TABLE_VER;
	tbl->tbl_num = num;
	ptr = &tbl->buf[0];
	hal_mem_cpy(h, (void*)ptr, s, num * sizeof(struct fbtc_slot));

	ops->fw_cmd(btc, BTFC_SET, SET_SLOT_TABLE, (u8*)tbl, n);
	hal_mem_free(h, (void*)tbl, n);
#endif
#endif // if 0 NEO
}


/* set RPT_EN_MREG = 0 to stop 2s monitor timer in WL FW,
 * before SET_MREG_TABLE, and set RPT_EN_MREG = 1 after
 * SET_MREG_TABLE
 */
void hal_btc_fw_set_monreg(struct btc_t *btc)
{
	RTW_ERR("%s NEO TODO\n", __func__);
#if 0 // NEO
	struct rtw_hal_com_t *h = btc->hal;
	struct btc_ops *ops = btc->ops;
	struct btf_set_mon_reg *monreg = NULL;
	u8 n, *ptr = NULL, ulen;
	u16 sz = 0;

	if (!ops || !ops->fw_cmd)
		return;

	n = btc->chip->mon_reg_num;

	if (n > CXMREG_MAX) {
		PHL_INFO("[BTC], mon reg count %d > %d !! \n", n, CXMREG_MAX);
		return;
	}

	ulen = sizeof(struct fbtc_mreg);
	sz = (ulen * n) + sizeof(*monreg) - 1;
	monreg = hal_mem_alloc(h, sz);
	if (!monreg)
		return;

	monreg->fver = BTF_SET_MON_REG_VER;
	monreg->reg_num = n;
	ptr = &monreg->buf[0];
	hal_mem_cpy(h, (void *)ptr, btc->chip->mon_reg, n * ulen);

	ops->fw_cmd(btc, BTFC_SET, SET_MREG_TABLE, (u8 *)monreg, sz);
	hal_mem_free(h, (void *)monreg, sz);
	hal_btc_fw_en_rpt(btc, RPT_EN_MREG, 1);
#endif // if 0 NEO
}

bool hal_btc_fw_set_1tdma(struct btc_t *btc, u16 len, u8 *buf)
{ /* for wlcli manual control  */
	RTW_ERR("%s NEO TODO\n", __func__);
	return false;
#if 0 // NEO
	struct btc_dm *dm = &btc->dm;

	if (len != sizeof(struct fbtc_tdma)) {
		PHL_INFO("[BTC], %s(): return because len != %d\n",
			 __func__, (int)sizeof(struct fbtc_tdma));
		return false;
	} else if (buf[0] >= CXTDMA_MAX) {
		PHL_INFO("[BTC], %s(): return because tdma_type >= %d\n",
			 __func__, CXTDMA_MAX);
		return false;
	}

	_tdma_cpy(&dm->tdma, buf);
	return true;
#endif // if 0 NEO
}

bool hal_btc_fw_set_1slot(struct btc_t *btc, u16 len, u8 *buf)
{ /* for wlcli manual control  */
	RTW_ERR("%s NEO TODO\n", __func__);
	return false;
#if 0 // NEO
	struct btc_dm *dm = &btc->dm;

	if (len != sizeof(struct fbtc_slot) + 1) {
		PHL_INFO("[BTC], %s(): return because len != %d\n",
			 __func__, (int)sizeof(struct fbtc_slot) + 1);
		return false;
	} else if (buf[0] >= CXST_MAX) {
		PHL_INFO("[BTC], %s(): return because slot_id >= %d\n",
			 __func__, CXST_MAX);
		return false;
	}

	_slot_cpy(&dm->slot[buf[0]], &buf[1]);
	return true;
#endif // if 0 NEO
}

bool hal_btc_fw_set_policy(struct btc_t *btc, bool force_exec, u16 policy_type,
			   const char* action)
{
	RTW_ERR("%s TODO NEO\n", __func__);
	return false;
#if 0 // NEO
	struct btc_dm *dm = &btc->dm;
	struct btc_ops *ops = btc->ops;

	if (!ops || !ops->fw_cmd)
		return false;

	PHL_INFO("[BTC], %s(): action = %s\n", __func__, action);
	_act_cpy(dm->run_action, (char*)action);

	btc->policy_len = 0; /* clear length before append */
	btc->policy_type = policy_type;

	_append_tdma(btc, force_exec);
	_append_slot(btc, force_exec);

	if (btc->policy_len == 0 || btc->policy_len > BTC_POLICY_MAXLEN)
		return false;

	PHL_INFO("[BTC], %s(): policy type/len: 0x%04x/%d\n",
		 __func__, policy_type, btc->policy_len);

	ops->fw_cmd(btc, BTFC_SET, SET_CX_POLICY, btc->policy, btc->policy_len);

	_tdma_cpy(&dm->tdma_now, &dm->tdma);
	_slots_cpy(dm->slot_now, dm->slot);

	return true;
#endif // if 0 NEO 
}


void hal_btc_fw_set_gpio_dbg(struct btc_t *btc, u8 type, u32 val)
{
	RTW_ERR("%s NEO TODO\n", __func__);
#if 0 // NEO TODO
	struct rtw_hal_com_t *h = btc->hal;
	struct btc_ops *ops = btc->ops;
	u8 data[7] = {0}, len = 0;

	if (!ops || !ops->fw_cmd || type >= CXDGPIO_MAX)
		return;

	PHL_INFO("[BTC], %s !! \n", __func__);

	data[0] = FCXGPIODBG_VER;
	data[1] = 0;
	data[2] = type;

	switch(type) {
	case CXDGPIO_EN_MAP:
		len = sizeof(u32) + 3;
		hal_mem_cpy(h, &data[3], &val, sizeof(u32));
		break;
	case CXDGPIO_MUX_MAP:
		len = sizeof(8) * 2 + 3;
		data[3] = (u8)(val & bMASKB0);
		data[4] = (u8)((val & bMASKB1) >> 8);
		break;
	default:
		return;
	}

	ops->fw_cmd(btc, BTFC_SET, SET_GPIO_DBG, data, len);
#endif // if 0 NEO
}

void hal_btc_fw_set_drv_info(struct btc_t *btc, u8 type)
{
	RTW_ERR("%s NEO TODO\n", __func__);
#if 0 // NEO TODO
	struct rtw_hal_com_t *h = btc->hal;
	struct btc_wl_info *wl = &btc->cx.wl;
	struct btc_dm *dm = &btc->dm;
	struct btc_ops *ops = btc->ops;
	u8 buf[64] = {0};
	u8 sz = 0, n = 0;

	if (!ops || !ops->fw_cmd || type >= CXDRVINFO_MAX)
		return;

	switch (type) {
	case CXDRVINFO_INIT:
		n = sizeof(dm->init_info);
		sz = n + 2;

		if (sz > sizeof(buf))
			return;

		buf[0] = CXDRVINFO_INIT;
		buf[1] = n;
		hal_mem_cpy(h, (void *)&buf[2], &dm->init_info, n);
		break;
	case CXDRVINFO_ROLE:
		n = sizeof(wl->role_info);
		sz = n + 2;

		if (sz > sizeof(buf))
			return;

		buf[0] = CXDRVINFO_ROLE;
		buf[1] = n;
		hal_mem_cpy(h, (void *)&buf[2], &wl->role_info, n);
		break;
	case CXDRVINFO_CTRL:
		n = sizeof(btc->ctrl);
		sz = n + 2;

		if (sz > sizeof(buf))
			return;

		buf[0] = CXDRVINFO_CTRL;
		buf[1] = n;
		hal_mem_cpy(h, (void *)&buf[2], &btc->ctrl, n);
		break;
	case CXDRVINFO_RFK:
		n = sizeof(wl->rfk_info);
		sz = n + 2;

		if (sz > sizeof(buf))
			return;

		buf[0] = CXDRVINFO_RFK;
		buf[1] = n;
		hal_mem_cpy(h, (void *)&buf[2], &wl->rfk_info, n);
		break;
#if BTC_CX_FW_OFFLOAD
	case CXDRVINFO_DBCC:
		n = sizeof(wl->dbcc_info);
		sz = n + 2;

		if (sz > sizeof(buf))
			return;

		buf[0] = CXDRVINFO_DBCC;
		buf[1] = n;
		hal_mem_cpy(h, (void *)&buf[2], &wl->dbcc_info, n);
		break;
	case CXDRVINFO_SMAP:
		n = sizeof(wl->status);
		sz = n + 2;

		if (sz > sizeof(buf))
			return;

		buf[0] = CXDRVINFO_SMAP;
		buf[1] = n;
		hal_mem_cpy(h, (void *)&buf[2], &wl->status, n);
		break;
	case CXDRVINFO_RUN:
		n = BTC_RSN_MAXLEN;
		sz = n + 2;

		if (sz > sizeof(buf))
			return;

		buf[0] = CXDRVINFO_RUN;
		buf[1] = n;
		hal_mem_cpy(h, (void *)&buf[2], dm->run_reason, n);
		break;
	case CXDRVINFO_SCAN:
		n = sizeof(wl->scan_info);
		sz = n + 2;

		if (sz > sizeof(buf))
			return;

		buf[0] = CXDRVINFO_SCAN;
		buf[1] = n;
		hal_mem_cpy(h, (void *)&buf[2], &wl->scan_info, n);
		break;
#endif
	default:
		return;
	}

	ops->fw_cmd(btc, BTFC_SET, SET_DRV_INFO, (u8*)buf, sz);
#endif // if 0 NEO
}

void hal_btc_fw_set_drv_event(struct btc_t *btc, u8 type)
{
	RTW_ERR("%s NEO TODO\n", __func__);
#if 0 // NEO TODO
	struct btc_ops *ops = btc->ops;

	if (!ops || !ops->fw_cmd)
		return;

	ops->fw_cmd(btc, BTFC_SET, SET_DRV_EVENT, &type, 1);
#endif // if 0 NEO
}


void hal_btc_fw_set_bt(struct btc_t *btc, u8 type, u16 len, u8* buf)
{
	RTW_ERR("%s NEO TODO\n", __func__);
#if 0 // NEO TODO
	struct btc_ops *ops = btc->ops;

	if (!ops || !ops->fw_cmd ||
	    (type < SET_BT_WREG_ADDR || type > SET_BT_GOLDEN_RX_RANGE))
		return;

	PHL_INFO("[BTC], %s !! \n", __func__);

	ops->fw_cmd(btc, BTFC_SET, type, buf, len);
#endif // if 0 NEO
}


void hal_btc_fw_event(struct btc_t *btc, u8 evt_id, void *data, u32 len)
{
	RTW_ERR("%s NEO TODO\n", __func__);
#if 0 // NEO TODO
	struct btf_fwinfo *pfwinfo = &btc->fwinfo;

	if (!len || !data)
		return;
#if 0
	PHL_INFO("[BTC], fw event=0x%x, len=%d\n", evt_id, len);
	/* debug_dump_data(data, len, "[BTC]"); */
#endif

	switch (evt_id) {
	case BTF_EVNT_RPT:
		_parse_report(btc, pfwinfo, data, len);
		break;
	default:
		break;
	}
#endif // if 0 NEO
}


#endif
