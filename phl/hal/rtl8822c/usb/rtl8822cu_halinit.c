/******************************************************************************
 *
 * Copyright(c) 2016 - 2019 Realtek Corporation.
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
#define _RTL8822CU_HALINIT_C_
#include "../../hal_headers.h"
#include "../rtl8822c_hal.h"

static void _hal_pre_init_8822cu(struct rtw_phl_com_t *phl_com,
				 struct hal_info_t *hal_info,
				 struct hal_init_info_t *init_22cu)
{
#if 0 // NEO
	struct mac_ax_trx_info *trx_info = &init_52au->trx_info;
	struct mac_ax_host_rpr_cfg *rpr_cfg = (struct mac_ax_host_rpr_cfg *)hal_info->rpr_cfg;

	trx_info->trx_mode = MAC_AX_TRX_NORMAL;

	if (hal_info->hal_com->dbcc_en == false)
		trx_info->qta_mode = MAC_AX_QTA_SCC;
	else
		trx_info->qta_mode = MAC_AX_QTA_DBCC;

	#ifdef RTW_WKARD_LAMODE
	PHL_INFO("%s : la_mode %d\n", __func__,	phl_com->dev_cap.la_mode);
	if (phl_com->dev_cap.la_mode)
		trx_info->qta_mode = MAC_AX_QTA_LAMODE;
	#endif

	rpr_cfg->agg_def = 1;
	rpr_cfg->tmr_def = 1;
	rpr_cfg->txok_en = MAC_AX_FUNC_DEF;
	rpr_cfg->rty_lmt_en = MAC_AX_FUNC_DEF;
	rpr_cfg->lft_drop_en = MAC_AX_FUNC_DEF;
	rpr_cfg->macid_drop_en = MAC_AX_FUNC_DEF;

	trx_info->rpr_cfg = rpr_cfg;
#endif // if 0 NEO
	init_22cu->ic_name = "rtl8822cu";
}

void init_hal_spec_8822cu(struct rtw_phl_com_t *phl_com,
					struct hal_info_t *hal)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	struct bus_hw_cap_t *bus_hw_cap = &hal_com->bus_hw_cap;

	init_hal_spec_8822c(phl_com, hal);
	phl_com->dev_cap.hw_sup_flags |= HW_SUP_USB_MULTI_FUN;
	bus_hw_cap->tx_buf_size = 2048;
	bus_hw_cap->tx_buf_num = 4;
	bus_hw_cap->tx_mgnt_buf_size = 1536;
	bus_hw_cap->tx_mgnt_buf_num = 32;
	bus_hw_cap->tx_h2c_buf_num = MAX_H2C_PKT_NUM;
	bus_hw_cap->rx_buf_size = 24576;
	bus_hw_cap->rx_buf_num = 8;
	bus_hw_cap->in_token_num = 4;
}

#if 0 // NEO

enum rtw_hal_status hal_get_efuse_8852au(struct rtw_phl_com_t *phl_com,
					 struct hal_info_t *hal_info)
{
	struct hal_init_info_t init_52au;

	_os_mem_set(hal_to_drvpriv(hal_info), &init_52au, 0, sizeof(init_52au));
	_hal_pre_init_8852au(phl_com, hal_info, &init_52au);

	return hal_get_efuse_8852a(phl_com, hal_info, &init_52au);
}
#endif // if 0 NEO

enum rtw_hal_status hal_init_8822cu(struct rtw_phl_com_t *phl_com,
				    struct hal_info_t *hal_info)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

	/* allocate memory for hal */
	hal_info->rpr_cfg = _os_mem_alloc(phlcom_to_drvpriv(phl_com),
					  sizeof(struct mac_host_rpr_cfg));
	if (hal_info->rpr_cfg == NULL) {
		hal_status = RTW_HAL_STATUS_RESOURCE;
		PHL_ERR("%s: alloc rpr_cfg failed\n", __func__);
		goto error_rpr_cfg;
	}

	hal_status = RTW_HAL_STATUS_SUCCESS;

error_rpr_cfg:
	return hal_status;
}

void hal_deinit_8822cu(struct rtw_phl_com_t *phl_com,
		       struct hal_info_t *hal_info)
{
	/* free memory for hal */
	_os_mem_free(phlcom_to_drvpriv(phl_com),
		     hal_info->rpr_cfg,
		     sizeof(struct mac_host_rpr_cfg));
}

enum rtw_hal_status hal_start_8822cu(struct rtw_phl_com_t *phl_com,
				    struct hal_info_t *hal_info)
{
	struct hal_init_info_t init_22cu;

	_os_mem_set(hal_to_drvpriv(hal_info), &init_22cu, 0, sizeof(init_22cu));
	_hal_pre_init_8822cu(phl_com, hal_info, &init_22cu);

	return hal_start_8822c(phl_com, hal_info, &init_22cu);
}

#if 0 // NEO

static void hal_deinit_misc_8852au(struct hal_info_t *hal)
{

}
#ifdef CONFIG_WOWLAN
enum rtw_hal_status 
hal_wow_init_8852au(struct rtw_phl_com_t *phl_com, struct hal_info_t *hal_info,
						struct rtw_phl_stainfo_t *sta)
{
	struct hal_init_info_t init_52au;
	struct mac_ax_trx_info *trx_info = &init_52au.trx_info;

	_os_mem_set( hal_to_drvpriv(hal_info), &init_52au, 0, sizeof(init_52au));
	trx_info->trx_mode = MAC_AX_TRX_NORMAL;
	trx_info->qta_mode = MAC_AX_QTA_SCC;
	/*
	if (hal_info->hal_com->dbcc_en == false)
		trx_info->qta_mode = MAC_AX_QTA_SCC;
	else
		trx_info->qta_mode = MAC_AX_QTA_DBCC;
	*/
	init_52au.ic_name = "rtl8852au";

	return hal_wow_init_8852a(phl_com, hal_info, sta, &init_52au);
}

enum rtw_hal_status
hal_wow_deinit_8852au(struct rtw_phl_com_t *phl_com, struct hal_info_t *hal_info,
							struct rtw_phl_stainfo_t *sta)
{
	struct hal_init_info_t init_52au;
	struct mac_ax_trx_info *trx_info = &init_52au.trx_info;

	_os_mem_set( hal_to_drvpriv(hal_info), &init_52au, 0, sizeof(init_52au));
	trx_info->trx_mode = MAC_AX_TRX_NORMAL;
	trx_info->qta_mode = MAC_AX_QTA_SCC;
	/*
	if (hal_info->hal_com->dbcc_en == false)
		trx_info->qta_mode = MAC_AX_QTA_SCC;
	else
		trx_info->qta_mode = MAC_AX_QTA_DBCC;
	*/
	init_52au.ic_name = "rtl8852au";

	return hal_wow_deinit_8852a(phl_com, hal_info, sta, &init_52au);
}
#endif /* CONFIG_WOWLAN */

#endif // if 0 NEO

enum rtw_hal_status hal_stop_8822cu(struct rtw_phl_com_t *phl_com,
				      struct hal_info_t *hal)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

	hal_status = hal_stop_8822c(phl_com, hal);
	return hal_status;
}

static const char *const _usb_sp[] = {
	"USB_SPEED_LOW",
	"USB_SPEED_FULL",
	"USB_SPEED_HIGH",
	"USB_SPEED_SUPER",
	"USB_SPEED_SUPER_10G",
	"USB_SPEED_SUPER_20G",
	"USB_SPEED_UNKNOWN"
};
#define usb_speed_str(_spidx) (((_spidx) >= RTW_USB_SPEED_UNKNOWN)\
	? _usb_sp[RTW_USB_SPEED_UNKNOWN] : _usb_sp[(_spidx)])


u32 hal_hci_cfg_8822cu(struct rtw_phl_com_t *phl_com,
		struct hal_info_t *hal, struct rtw_ic_info *ic_info)
{
	struct hal_spec_t *hal_spec = phl_get_ic_spec(phl_com);

	/*get USB Bus-info from os*/
	PHL_INFO("%s ===>\n", __func__);
	PHL_INFO("%s\n", usb_speed_str(ic_info->usb_info.usb_speed));
	PHL_INFO("Bulk-Out size - %d\n", ic_info->usb_info.usb_bulkout_size);
	PHL_INFO("Bulk-Out Number - %d\n", ic_info->usb_info.outep_num);
	PHL_INFO("Bulk-In Number - %d\n", ic_info->usb_info.inep_num);

	hal_spec->max_bulkin_num = ic_info->usb_info.inep_num;
	hal_spec->max_bulkout_num = ic_info->usb_info.outep_num;
	hal_spec->cts2_thres_en = false;
	hal_spec->cts2_thres = 0;

	return RTW_HAL_STATUS_SUCCESS;
}

#if 0 // NEO

bool hal_recognize_int_8852au(struct hal_info_t *hal)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;
	bool recognized = false;

	/* halt c2h */
	hal_com->intr.halt_c2h_int.intr = hal_read32(hal_com, R_AX_HISR0);
	hal_com->intr.halt_c2h_int.intr &= hal_com->intr.halt_c2h_int.val_mask;
	hal_write32(hal_com,R_AX_HISR0, hal_com->intr.halt_c2h_int.intr);

	/* watchdog timer */
	hal_com->intr.watchdog_timer_int.intr = hal_read32(hal_com, R_AX_HD0ISR);
	hal_com->intr.watchdog_timer_int.intr &= hal_com->intr.watchdog_timer_int.val_mask;
	hal_write32(hal_com,R_AX_HD0ISR, hal_com->intr.watchdog_timer_int.intr);


	if (hal_com->intr.halt_c2h_int.intr || hal_com->intr.watchdog_timer_int.intr)
		recognized = true;

	return recognized;
}

static u32 hal_halt_c2h_handler_8852au(struct hal_info_t *hal, u32 *handled)
{
	u32 ret = 0;
	struct rtw_hal_com_t *hal_com = hal->hal_com;

	if (hal_com->intr.halt_c2h_int.intr & B_AX_HALT_C2H_INT_EN) {
		handled[0] |= B_AX_HALT_C2H_INT_EN;
		ret = 1;
	}

	return ret;
}

static u32 hal_watchdog_timer_handler_8852au(struct hal_info_t *hal, u32 *handled)
{
	u32 ret = 0;
	struct rtw_hal_com_t *hal_com = hal->hal_com;

	if (hal_com->intr.watchdog_timer_int.intr & B_AX_WDT_PTFM_INT_EN) {
		handled[1] |= B_AX_WDT_PTFM_INT_EN;
		ret = 1;
	}

	return ret;
}

u32 hal_int_hdler_8852au(struct hal_info_t *hal)
{
	u32 int_hdler_msk = 0x0;
	u32 generalhandled[8] = {0};

	/* Start General interrupt type */
	/* <4> Halt C2H related */
	int_hdler_msk |= (hal_halt_c2h_handler_8852au(hal, generalhandled) << 4);

	/* <4>watchdog timer related */
	int_hdler_msk |= (hal_watchdog_timer_handler_8852au(hal, generalhandled) << 5);

	PHL_TRACE(COMP_PHL_DBG,_PHL_INFO_, "%s: int_hdler_msk = 0x%x\n", __func__, int_hdler_msk);

	return int_hdler_msk;
}

#endif // if 0 NEO
