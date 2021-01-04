/******************************************************************************
 *
 * Copyright(c)2019 Realtek Corporation.
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
#ifndef _HAL_SER_H_
#define _HAL_SER_H_

u32
rtw_hal_ser_get_error_status(void *hal, u32 *err, bool *ignored);

u32
rtw_hal_ser_set_error_status(void *hal, u32 err);

u32
rtw_hal_lv1_rcvy(void *hal, u32 step);

void
rtw_phl_mac_dbg_dump_fw_rsvd_ple(void *phl);

#endif /* _HAL_SER_H_ */

