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

#ifndef _MAC_AX_DLE_H_
#define _MAC_AX_DLE_H_

#include "../type.h"
#include "../mac_ax.h"
#include "cpuio.h"

/*--------------------Define ----------------------------------------*/
#define DLE_DFI_WAIT_CNT 1000
#define DLE_DFI_WAIT_US 100

#define DLE_WAIT_CNT 2000
#define DLE_WAIT_US 1

// DLE_DFI_TYPE_FREEPG
#define B_AX_DLE_FREE_TAILPG_SH 16
#define B_AX_DLE_FREE_TAILPG_MSK 0xfff
#define B_AX_DLE_FREE_HEADPG_SH 0
#define B_AX_DLE_FREE_HEADPG_MSK 0xfff

#define B_AX_DLE_PUB_PGNUM_SH 0
#define B_AX_DLE_PUB_PGNUM_MSK 0x1fff

// DLE_DFI_TYPE_QUOTA
#define B_AX_DLE_USE_PGNUM_SH 16
#define B_AX_DLE_USE_PGNUM_MSK 0xfff
#define B_AX_DLE_RSV_PGNUM_SH 0
#define B_AX_DLE_RSV_PGNUM_MSK 0xfff

// DLE_DFI_TYPE_QEMPTY
#define B_AX_DLE_QEMPTY_GRP_SH 0
#define B_AX_DLE_QEMPTY_GRP_MSK 0xffffffff

#define QUEUE_EMPTY_CHK_CNT 2

#define S_AX_WDE_PAGE_SEL_64	0
#define S_AX_WDE_PAGE_SEL_128	1
/* #define S_AX_WDE_PAGE_SEL_256	2 // HDP not support */

/* #define S_AX_PLE_PAGE_SEL_64	0 // HDP not support */
#define S_AX_PLE_PAGE_SEL_128	1
#define S_AX_PLE_PAGE_SEL_256	2

#define DLE_BOUND_UNIT (8 * 1024)

#define WDE_MGN_INI_RDY (B_AX_WDE_Q_MGN_INI_RDY | B_AX_WDE_BUF_MGN_INI_RDY)
#define PLE_MGN_INI_RDY (B_AX_PLE_Q_MGN_INI_RDY | B_AX_PLE_BUF_MGN_INI_RDY)
#define DLE_QUEUE_NONEMPTY	0
#define DLE_QUEUE_EMPTY		1

// DLE_EMPTY_STATUS
#define WDE_MACID_QUEUE_OFFSET 0x80070000
#define WDE_MGNT_QUEUE_OFFSET 0x80070010
#define macid_acq_mask 0xf

#define B_CMAC0_MGQ_NORMAL	BIT2
#define B_CMAC0_MGQ_NO_PWRSAV	BIT3
#define B_CMAC0_CPUMGQ		BIT4
#define B_CMAC1_MGQ_NORMAL	BIT10
#define B_CMAC1_MGQ_NO_PWRSAV	BIT11
#define B_CMAC1_CPUMGQ		BIT12

#define DLE_LAMODE_SIZE_8852A (256 * 1024)
#define DLE_LAMODE_SIZE_8852B (128 * 1024)

/*--------------------Define Enum------------------------------------*/
enum WDE_QTAID {
	WDE_QTAID_HOST_IF = 0,
	WDE_QTAID_WLAN_CPU = 1,
	WDE_QTAID_DATA_CPU = 2,
	WDE_QTAID_PKTIN = 3,
	WDE_QTAID_CPUIO = 4
};

enum PLE_QTAID {
	PLE_QTAID_B0_TXPL = 0,
	PLE_QTAID_B1_TXPL = 1,
	PLE_QTAID_C2H = 2,
	PLE_QTAID_H2C = 3,
	PLE_QTAID_WLAN_CPU = 4,
	PLE_QTAID_MPDU = 5,
	PLE_QTAID_CMAC0_RX = 6,
	PLE_QTAID_CMAC1_RX = 7,
	PLE_QTAID_CMAC1_BBRPT = 8,
	PLE_QTAID_WDRLS = 9,
	PLE_QTAID_CPUIO = 10
};

enum DLE_CTRL_TYPE {
	DLE_CTRL_TYPE_WDE = 0,
	DLE_CTRL_TYPE_PLE = 1,
	DLE_CTRL_TYPE_NUM = 2
};

enum DLE_DFI_TYPE {
	DLE_DFI_TYPE_FREEPG = 0,
	DLE_DFI_TYPE_QUOTA = 1,
	DLE_DFI_TYPE_PAGELLT = 2,
	DLE_DFI_TYPE_PKTINFO = 3,
	DLE_DFI_TYPE_PREPKTLLT = 4,
	DLE_DFI_TYPE_NXTPKTLLT = 5,
	DLE_DFI_TYPE_QLNKTBL = 6,
	DLE_DFI_TYPE_QEMPTY = 7
};

/*--------------------Define MACRO----------------------------------*/

/*--------------------Define Struct-----------------------------------*/
struct dle_dfi_ctrl_t {
	u8 ctrl_type;
	// input parameter
	u32 dfi_ctrl;
	// output parameter
	u32 dfi_data;
};

struct dle_dfi_freepg_t {
	// input parameter
	u8 dle_type;
	// output parameter
	u16 free_headpg;
	u16 free_tailpg;
	u16 pub_pgnum;
};

struct dle_dfi_quota_t {
	// input parameter
	u8 dle_type;
	u8 qtaid;
	// output parameter
	u16 rsv_pgnum;
	u16 use_pgnum;
};

struct dle_dfi_qempty_t {
	// input parameter
	u8 dle_type;
	u8 grpsel;
	// output parameter
	u32 qempty;
};

struct dle_size_t {
	u16 pge_size;
	u16 lnk_pge_num;
	u16 unlnk_pge_num;
};

struct wde_quota_t {
	u16 hif;
	u16 wcpu;
	u16 pkt_in;
	u16 cpu_io;
};

struct ple_quota_t {
	u16 cma0_tx;
	u16 cma1_tx;
	u16 c2h;
	u16 h2c;
	u16 wcpu;
	u16 mpdu_proc;
	u16 cma0_dma;
	u16 cma1_dma;
	u16 bb_rpt;
	u16 wd_rel;
	u16 cpu_io;
};

struct dle_mem_t {
	enum mac_ax_qta_mode mode;
	struct dle_size_t *wde_size;
	struct dle_size_t *ple_size;
	struct wde_quota_t *wde_min_qt;
	struct wde_quota_t *wde_max_qt;
	struct ple_quota_t *ple_min_qt;
	struct ple_quota_t *ple_max_qt;
};

/*--------------------Export global variable----------------------------*/

/*--------------------Function declaration-----------------------------*/
u32 dle_dfi_freepg(struct mac_ax_adapter *adapter,
		   struct dle_dfi_freepg_t *freepg);

u32 dle_dfi_quota(struct mac_ax_adapter *adapter,
		  struct dle_dfi_quota_t *quota);

u32 dle_dfi_qempty(struct mac_ax_adapter *adapter,
		   struct dle_dfi_qempty_t *qempty);

u32 mac_chk_allq_empty(struct mac_ax_adapter *adapter, u8 *empty);

u32 dle_quota_change(struct mac_ax_adapter *adapter, enum mac_ax_qta_mode mode);

u32 dle_init(struct mac_ax_adapter *adapter, enum mac_ax_qta_mode mode,
	     enum mac_ax_qta_mode ext_mode);

u32 dle_is_txq_empty(struct mac_ax_adapter *adapter, u8 *val);

u32 dle_is_rxq_empty(struct mac_ax_adapter *adapter, u8 *val);

u32 mac_is_txq_empty(struct mac_ax_adapter *adapter,
		     struct mac_ax_tx_queue_empty *val);

u32 mac_is_rxq_empty(struct mac_ax_adapter *adapter,
		     struct mac_ax_rx_queue_empty *val);

u32 is_qta_dbcc(struct mac_ax_adapter *adapter, enum mac_ax_qta_mode mode,
		u8 *is_dbcc);

u32 is_qta_poh(struct mac_ax_adapter *adapter, enum mac_ax_qta_mode mode,
	       u8 *is_poh);
#endif
