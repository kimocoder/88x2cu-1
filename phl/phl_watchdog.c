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
#define _PHL_WATCHDOG_C_
#include "phl_headers.h"

#ifdef CONFIG_CMD_DISP
static void _phl_watchdog_done(void *drv_priv, u8 *cmd, u32 cmd_len, enum rtw_phl_status status)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)cmd;
	struct phl_watchdog *wdog = &(phl_info->wdog);

	_os_set_timer(drv_priv,
	              &wdog->wdog_timer,
	              wdog->period);
}

static enum rtw_phl_status
_phl_watchdog_cmd(struct phl_info_t *phl_info,
                  enum phl_cmd_type cmd_type,
                  u32 cmd_timeout)
{
	void *drv = phl_to_drvpriv(phl_info);
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;

	if (cmd_type == PHL_CMD_DIRECTLY) {
		phl_status = phl_watchdog_cmd_hdl(phl_info);
		goto _exit;
	}

	phl_status = phl_cmd_enqueue(phl_info,
	                             HW_BAND_0,
	                             MSG_EVT_WATCHDOG,
	                             (u8 *)phl_info,
	                             0,
	                             _phl_watchdog_done,
	                             cmd_type,
	                             cmd_timeout);
	if (is_cmd_failure(phl_status))
		phl_status = RTW_PHL_STATUS_FAILURE;
	else
		phl_status = RTW_PHL_STATUS_SUCCESS;

_exit:
	return phl_status;
}
#endif

static void _phl_watchdog_timer_expired(void *context)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)context;
	struct phl_watchdog *wdog = &(phl_info->wdog);
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;

#ifdef CONFIG_CMD_DISP
	if (true == phl_disp_eng_is_fg_empty(phl_info, HW_BAND_MAX)) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "%s: trigger watchdog(period %d)\n",
		          __FUNCTION__, wdog->period);

		psts = _phl_watchdog_cmd(phl_info, PHL_CMD_NO_WAIT, 0);
	} else {
		/*
		 * shall do core handler for core statistic
		 * when phl watchdog separate sw and hw beahvior
		 */
		/* wdog->core_wdog(phl_to_drvpriv(phl_info)); */
		PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "%s: skip watchdog\n",
		          __FUNCTION__);
	}

	if ((false == is_cmd_enqueue(psts)) && (RTW_PHL_STATUS_SUCCESS != psts)) {
		_os_set_timer(phl_to_drvpriv(phl_info),
		              &wdog->wdog_timer,
		              wdog->period);
	}
#else
	PHL_TRACE(COMP_PHL_DBG, _PHL_ERR_, "%s: Not support watchdog\n", __FUNCTION__);
#endif
}

enum rtw_phl_status
phl_watchdog_cmd_hdl(struct phl_info_t *phl_info)
{
	struct phl_watchdog *wdog = &(phl_info->wdog);

	rtw_phl_watchdog_callback(phl_info);

	if (NULL != wdog->core_wdog)
		wdog->core_wdog(phl_to_drvpriv(phl_info));

	return RTW_PHL_STATUS_SUCCESS;
}

void phl_datapath_watchdog(struct phl_info_t *phl_info)
{
	phl_tx_watchdog(phl_info);
	phl_rx_watchdog(phl_info);
	phl_sta_trx_tfc_upd(phl_info);
}

#ifdef RTW_WKARD_WIN_REQ_PWR_IN_WATCHDOG
u8 _get_rssi_bcn_min(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct macid_ctl_t *macid_ctl = phl_to_mac_ctrl(phl_info);
	struct rtw_phl_stainfo_t *stainfo = NULL;
	struct rtw_hal_stainfo_t *hal_sta = NULL;
	u8 rssi_bcn_min = 0xFF;
	u16 i = 0;

	for (i = 0; i < macid_ctl->max_num; i++) {
		stainfo = rtw_phl_get_stainfo_by_macid(phl_info, i);

		if (NULL == stainfo)
			continue;

		hal_sta = stainfo->hal_sta;

		if (NULL == hal_sta)
			continue;

		PHL_INFO("_get_rssi_bcn_min: macid(%d), rssi_bcn=%d\n", i, PHL_TRANS_2_RSSI(hal_sta->rssi_stat.rssi_bcn));

		if (PHL_TRANS_2_RSSI(hal_sta->rssi_stat.rssi_bcn) == 0)
			continue;

		rssi_bcn_min = MIN(PHL_TRANS_2_RSSI(hal_sta->rssi_stat.rssi_bcn), rssi_bcn_min);
	}

	return rssi_bcn_min;
}

u8 _simple_watchdog_by_rssi(void *phl, u8 rssi)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	enum rtw_phl_status status = RTW_PHL_STATUS_FAILURE;
	u8 current_rssi = _get_rssi_bcn_min(phl_info);
	u8 ret_val = rssi;
	u8 io_en = false;
	#define _DIFF(_x_, _y_)	((_x_ >= _y_) ? (_x_ - _y_) : (_y_ - _x_))

	do {
		if (rssi == 0 || rssi == 0xFF) {
			PHL_INFO("_simple_watchdog_by_rssi: (criteria=%d, cur=%d) criteria invalid, set criteria = cur\n", rssi, current_rssi);
			ret_val = current_rssi; /* update with current_rssi */
			break;
		}

		if (_DIFF(rssi, current_rssi) < phl_com->ps_rssi_diff_threshold) {
			PHL_INFO("_simple_watchdog_by_rssi: (criteria=%d, cur=%d) RSSI diff < %d, do nothing\n",
						rssi, current_rssi, phl_com->ps_rssi_diff_threshold);
			ret_val = rssi; /* keep original rssi value */
			break;
		}

		PHL_INFO("_simple_watchdog_by_rssi: (criteria=%d, cur=%d) RSSI diff >= %d, try to call rtw_hal_simple_watchdog\n",
					rssi, current_rssi, phl_com->ps_rssi_diff_threshold);

		status = rtw_phl_ps_notify(phl, PHL_PS_NTFY_SIMPLE_WATCHDOG_START,
				    NULL);

		if (status != RTW_PHL_STATUS_SUCCESS) {
			PHL_INFO("_simple_watchdog_by_rssi: notify fail, SKIP rtw_hal_simple_watchdog\n");
			break;
		}

		PHL_INFO("_simple_watchdog_by_rssi: ===> rtw_hal_simple_watchdog\n");
		io_en = true; /* io_en==true, run bb dig */
		status = rtw_hal_simple_watchdog(phl_info->hal, io_en);
		if (status == RTW_PHL_STATUS_SUCCESS) {
			PHL_INFO("_simple_watchdog_by_rssi: update rssi criteria (%d)\n", current_rssi);
			ret_val = current_rssi; /* update with current_rssi */
		}
		PHL_INFO("_simple_watchdog_by_rssi: <=== rtw_hal_simple_watchdog\n");
		status = rtw_phl_ps_notify(phl, PHL_PS_NTFY_SIMPLE_WATCHDOG_END,
			    NULL);
	} while (false);

	if (io_en == false) /* io_en==false, notify bb update rssi */
		status = rtw_hal_simple_watchdog(phl_info->hal, io_en);

	return ret_val;
}
#endif

void rtw_phl_watchdog_callback(void *phl)
{
	RTW_ERR("%s NEO TODO\n", __func__);
#if 0 // NEO TODO
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_com_t* phl_com = phl_info->phl_com;
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
#ifdef RTW_WKARD_WIN_REQ_PWR_IN_WATCHDOG
	static u8 rssi_bcn_min = 0;
#endif

	do {
		phl_datapath_watchdog(phl_info);
		#if defined(CONFIG_PCI_HCI) && defined(PCIE_TRX_MIT_EN)
		phl_pcie_trx_mit_watchdog(phl_info);
		#endif
		phl_mr_watchdog(phl_info);
		phl_ps_watchdog(phl_info);

#ifdef RTW_WKARD_WIN_REQ_PWR_IN_WATCHDOG
		pstatus = rtw_phl_ps_notify(phl, PHL_PS_NTFY_WATCHDOG_START, NULL);

		if (pstatus == RTW_PHL_STATUS_RESOURCE) { /* Under PowerSave Cannot IO */
			PHL_INFO("rtw_phl_watchdog_callback: under ps\n");
			rssi_bcn_min = _simple_watchdog_by_rssi(phl, rssi_bcn_min);
			break;
		} else if(pstatus != RTW_PHL_STATUS_SUCCESS) {
			PHL_INFO("rtw_phl_watchdog_callback: Bypass hal watchdog by power (%d) !!\n", pstatus);
			break;
		}
		rssi_bcn_min = _get_rssi_bcn_min(phl_info);
		PHL_INFO("rtw_phl_watchdog_callback: not under ps, update rssi criteria (%d)\n", rssi_bcn_min);
#endif
		rtw_hal_watchdog(phl_info->hal);

#ifdef RTW_WKARD_WIN_REQ_PWR_IN_WATCHDOG
		pstatus = rtw_phl_ps_notify(phl, PHL_PS_NTFY_WATCHDOG_STOP, NULL);
#endif
	} while (false);

#endif // if 0 NEO
}

void rtw_phl_watchdog_init(void *phl,
                           u16 period,
                           void (*core_wdog)(void *drv_priv))
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_watchdog *wdog = &(phl_info->wdog);

	wdog->core_wdog = core_wdog;

	if (period > 0)
		wdog->period = period;
	else
		wdog->period = WDOG_PERIOD;

	_os_init_timer(phl_to_drvpriv(phl_info),
	               &wdog->wdog_timer,
	               _phl_watchdog_timer_expired,
	               phl,
	               "phl_watchdog_timer");
}

void rtw_phl_watchdog_deinit(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_watchdog *wdog = &(phl_info->wdog);

	_os_release_timer(phl_to_drvpriv(phl_info), &wdog->wdog_timer);
}

void rtw_phl_watchdog_start(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_watchdog *wdog = &(phl_info->wdog);

	_os_set_timer(phl_to_drvpriv(phl_info),
	              &wdog->wdog_timer,
	              wdog->period);
}

void rtw_phl_watchdog_stop(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_watchdog *wdog = &(phl_info->wdog);

	_os_cancel_timer(phl_to_drvpriv(phl_info), &wdog->wdog_timer);
}
