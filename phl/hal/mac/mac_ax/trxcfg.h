/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation. All rights reserved.
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
 ******************************************************************************/

#ifndef _MAC_AX_TRXCFG_H_
#define _MAC_AX_TRXCFG_H_

#include "../type.h"
#include "hw.h"
#include "init.h"
#include "role.h"
#include "cmac_tx.h"
#include "rx_filter.h"
#include "dle.h"
#include "hci_fc.h"

/*--------------------Define -------------------------------------------*/
#define TRXCFG_WAIT_CNT		2000
#define TRXCFG_WAIT_US		1

/* MPDU Processor Control */
#define TRXCFG_MPDU_PROC_ACT_FRWD	0x02A95A95
#define TRXCFG_MPDU_PROC_TF_FRWD	0x0000AA55
#define TRXCFG_MPDU_PROC_CUT_CTRL	0x010E05F0

/* RMAC timeout control */
#define TRXCFG_RMAC_CCA_TO	32
#define TRXCFG_RMAC_DATA_TO	15

#define S_AX_TXSC_20M_0		0
#define S_AX_TXSC_20M_4		4
#define S_AX_TXSC_40M_0		0
#define S_AX_TXSC_40M_4		4
#define S_AX_TXSC_80M_0		0
#define S_AX_TXSC_80M_4		4

/* TRXPTCL SIFS TIME*/
#define WMAC_SPEC_SIFS_OFDM_52A 0x15
#define WMAC_SPEC_SIFS_OFDM_52B 0x11
#define WMAC_SPEC_SIFS_OFDM_52C 0x11
#define WMAC_SPEC_SIFS_CCK	 0xA

/* SRAM fifo address */
#define CMAC_TBL_BASE_ADDR	0x18840000

#define CMAC1_START_ADDR	0xE000
#define CMAC1_END_ADDR		0xFFFF

#if MAC_AX_ASIC_TEMP
#define R_AX_LTECOEX_CTRL 0x38
#define R_AX_LTECOEX_CTRL_2 0x3C
#endif

#define S_AX_CTS2S_TH_1K 4
#define S_AX_CTS2S_TH_SEC_256B 1
#define S_AX_PTCL_TO_2MS 0x3F

#define LBK_PLCP_DLY_DEF 0x28
#define LBK_PLCP_DLY_FPGA 0x46

#define PLD_RLS_MAX_PG 127
#define RX_MAX_LEN_UNIT 512

#define SCH_PREBKF_24US 0x18

/*--------------------Define MACRO--------------------------------------*/
/*--------------------Define Enum---------------------------------------*/
/*--------------------Define Struct-------------------------------------*/
u32 mac_enable_imr(struct mac_ax_adapter *adapter, u8 band,
		   enum mac_ax_hwmod_sel sel);
u32 check_mac_en(struct mac_ax_adapter *adapter, u8 band,
		 enum mac_ax_hwmod_sel sel);
u32 mac_check_access(struct mac_ax_adapter *adapter, u32 offset);
u32 mac_trx_init(struct mac_ax_adapter *adapter, struct mac_ax_trx_info *info);
u32 mac_sr_update(struct mac_ax_adapter *adapter,
		  struct mac_ax_sr_info *sr_info, u8 band);
u32 mac_dbcc_enable(struct mac_ax_adapter *adapter,
		    struct mac_ax_trx_info *info, u8 dbcc_en);
u32 mac_tx_mode_sel(struct mac_ax_adapter *adapter,
		    struct mac_ax_mac_tx_mode_sel *mode_sel);
u32 mac_two_nav_cfg(struct mac_ax_adapter *adapter,
		    struct mac_ax_2nav_info *info);
#endif
