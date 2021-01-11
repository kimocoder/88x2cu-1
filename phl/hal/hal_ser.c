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
#define _HAL_SER_C_
#include "hal_headers.h"


u32
rtw_hal_ser_get_error_status(void *hal, u32 *err, bool *ignored)
{
	RTW_ERR("%s TODO NEO\n", __func__);
	return 0;
#if 0 // NEO TODO
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	enum RTW_PHL_SER_NOTIFY_EVENT notify = RTW_PHL_SER_L2_RESET;
	*err = rtw_hal_mac_ser_get_error_status(hal_info);

	if ( (*err == MAC_AX_ERR_L1_ERR_DMAC) ||(*err == MAC_AX_ERR_L0_PROMOTE_TO_L1)) {
				notify = RTW_PHL_SER_PAUSE_TRX;
	} else if (*err == MAC_AX_ERR_L1_RESET_DISABLE_DMAC_DONE) {
				notify = RTW_PHL_SER_DO_RECOVERY;
	} else if (*err == MAC_AX_ERR_L1_RESET_RECOVERY_DONE) {
				notify = RTW_PHL_SER_READY;
	} else if ( (*err >= MAC_AX_ERR_L1_PROMOTE_TO_L2) && (*err <=MAC_AX_ERR_L2_ERR_APB_BBRF_TO_OTHERS) ) {
#ifdef RTW_WKARD_SER_L2_IGNORED_WHILE_L1
		if (*err != MAC_AX_ERR_L1_PROMOTE_TO_L2)
			*ignored = true;
#endif
		notify = RTW_PHL_SER_L2_RESET;
	} else if (*err < MAC_AX_ERR_L0_PROMOTE_TO_L1) {
		notify = RTW_PHL_SER_L0_RESET;
	}

	return notify;
#endif
}


u32
rtw_hal_ser_set_error_status(void *hal, u32 err)
{
	RTW_ERR("%s TODO NEO\n", __func__);
	return MACPROCERR;
#if 0
	u32 mac_err;
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	mac_err = rtw_hal_mac_ser_set_error_status(hal_info,err);

	return mac_err;
#endif
}


u32
rtw_hal_lv1_rcvy(void *hal, u32 step)
{
	RTW_ERR("%s TODO NEO\n", __func__);
	return MACPROCERR;
#if 0
	u32 mac_err;
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "===> rtw_hal_lv1_rcvy step %d\n",step);
	mac_err = rtw_hal_mac_lv1_rcvy(hal_info, step);
	PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "<=== rtw_hal_lv1_rcvy step %d, mac_err 0x%x\n", step, mac_err);
	return mac_err;
#endif
}

void rtw_hal_dbg_dump_fw_rsvd_ple(void *hal)
{
	RTW_ERR("%s TODO NEO\n", __func__);
#if 0
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	u32 mac_err;
	mac_err = rtw_hal_mac_dbg_dump_fw_rsvd_ple(hal_info);
#endif
}
