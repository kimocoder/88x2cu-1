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
#ifndef _RTL8822C_HAL_H_
#define _RTL8822C_HAL_H_
#include "../hal_headers.h"

/* rtl8822c_ops.c */
void hal_set_ops_8822c(struct rtw_phl_com_t *phl_com, struct hal_info_t *hal);

#include "hal_trx_8822c.h"



/*usage under rtl8822c folder*/
//#include "rtl8822c_spec.h"

#ifdef CONFIG_PCI_HCI
#include "pci/rtl8822ce_hal.h"
#endif

#ifdef CONFIG_USB_HCI
#include "usb/rtl8822cu_hal.h"
#endif

#ifdef CONFIG_SDIO_HCI
#include "sdio/rtl8822cs_hal.h"
#endif


/* rtl8822c_halinit.c */
void init_hal_spec_8822c(struct rtw_phl_com_t *phl_com,
					struct hal_info_t *hal);
enum rtw_hal_status hal_cfg_fw_8822c(struct rtw_phl_com_t *phl_com,
				     struct hal_info_t *hal,
				     char *ic_name,
				     enum rtw_fw_type fw_type);

/* rtl8822c_ops.c */
void hal_set_ops_8822c(struct rtw_phl_com_t *phl_com,
					struct hal_info_t *hal);
/*void hal_set_trx_ops_8822c(struct hal_info_t *hal);*/


/* rtl8822c_ps.c */
void hal_hook_ps_ops_8822c(struct hal_info_t *hal);

void init_default_value_8822c(struct hal_info_t *hal);
enum rtw_hal_status hal_get_efuse_8822c(struct rtw_phl_com_t *phl_com,
					struct hal_info_t *hal,
					struct hal_init_info_t *init_info);
enum rtw_hal_status hal_start_8822c(struct rtw_phl_com_t *phl_com,
				   struct hal_info_t *hal,
				   struct hal_init_info_t *init_info);
enum rtw_hal_status hal_stop_8822c(struct rtw_phl_com_t *phl_com,
				     struct hal_info_t *hal);

#ifdef CONFIG_WOWLAN
enum rtw_hal_status
hal_wow_init_8822c(struct rtw_phl_com_t *phl_com,
				struct hal_info_t *hal_info, struct rtw_phl_stainfo_t *sta,
					struct hal_init_info_t *init_info);
enum rtw_hal_status
hal_wow_deinit_8822c(struct rtw_phl_com_t *phl_com,
				struct hal_info_t *hal_info, struct rtw_phl_stainfo_t *sta,
					struct hal_init_info_t *init_info);
#endif /* CONFIG_WOWLAN */

#ifdef RTW_PHL_BCN
enum rtw_hal_status hal_config_beacon_8822c(struct rtw_phl_com_t *phl_com, struct hal_info_t *hal, struct rtw_bcn_entry *bcn_entry);
enum rtw_hal_status hal_update_beacon_8822c(struct rtw_phl_com_t *phl_com, struct hal_info_t *hal, struct rtw_bcn_entry *bcn_entry);
#endif


#endif /* _RTL8852A_HAL_H_ */
