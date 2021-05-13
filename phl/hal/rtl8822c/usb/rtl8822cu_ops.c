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
#define _RTL8822CU_OPS_C_
#include "../rtl8822c_hal.h"
#include "rtl8822cu.h"

static void init_default_value_8822cu(struct hal_info_t *hal)
{
	struct rtw_hal_com_t *hal_com = hal->hal_com;

	init_default_value_8822c(hal);

	//hal_com->intr.halt_c2h_int.val_default = (u32)(B_AX_HALT_C2H_INT_EN | 0);
	//hal_com->intr.halt_c2h_int.val_mask = hal_com->intr.halt_c2h_int.val_default;
}

void hal_set_ops_8822cu(struct rtw_phl_com_t *phl_com,
			struct hal_info_t *hal)
{
	struct hal_ops_t *ops = hal_get_ops(hal);

	hal_set_ops_8822c(phl_com, hal);

	ops->init_hal_spec = init_hal_spec_8822cu;
#if 0 // NEO TODO
	ops->hal_get_efuse = hal_get_efuse_8852au;
#endif // if 0 NEO
	ops->hal_init = hal_init_8822cu;
	ops->hal_deinit = hal_deinit_8822cu;
	ops->hal_start = hal_start_8822cu;
	ops->hal_stop = hal_stop_8822cu;
#ifdef CONFIG_WOWLAN
	ops->hal_wow_init = hal_wow_init_8852au;
	ops->hal_wow_deinit = hal_wow_deinit_8852au;
#endif /* CONFIG_WOWLAN */
	ops->hal_hci_configure = hal_hci_cfg_8822cu;
	ops->init_default_value = init_default_value_8822cu;
#if 0 // NEO TODO
	ops->recognize_interrupt = hal_recognize_int_8852au;
	ops->interrupt_handler = hal_int_hdler_8852au;
#endif // NEO if 0
}



