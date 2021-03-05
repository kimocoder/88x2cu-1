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
#ifndef _PHL_WATCHDOG_H_
#define _PHL_WATCHDOG_H_

#define WDOG_PERIOD 2000

struct phl_watchdog {
	_os_timer wdog_timer;
	void (*core_wdog)(void *drv_priv);
	u16 period;
};

enum rtw_phl_status
phl_watchdog_cmd_hdl(struct phl_info_t *phl_info);

void rtw_phl_watchdog_callback(void *phl);

#endif /*_PHL_WATCHDOG_H_*/

