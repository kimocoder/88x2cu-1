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
#ifndef _PHL_TRX_MIT_H_
#define _PHL_TRX_MIT_H_

enum rtw_phl_status phl_pcie_trx_mit_start(struct phl_info_t *phl_info,
					   u8 dispr_idx);
enum rtw_phl_status phl_evt_pcie_trx_mit_hdlr(struct phl_info_t *phl_info,
					      struct phl_msg *msg);

enum rtw_phl_status phl_pcie_trx_mit(struct phl_info_t *phl_info, u32 tx_timer,
				     u8 tx_counter, u32 rx_timer,
				     u8 rx_counter);

void phl_pcie_trx_mit_watchdog(struct phl_info_t *phl_info);

#endif /*_PHL_CMD_GENERAL_H_*/
