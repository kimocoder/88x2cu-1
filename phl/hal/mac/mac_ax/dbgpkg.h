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

#ifndef _MAC_AX_DBGPKG_H_
#define _MAC_AX_DBGPKG_H_

#include "../mac_def.h"
#include "../mac_ax.h"
#include "fwcmd.h"
#include "trx_desc.h"
#include "trxcfg.h"
#include "dle.h"

#define FW_RSVD_PLE_SIZE 0x800
#define SHARE_BUFFER_SIZE_8852A 0x70000
#define SHARE_BUFFER_SIZE_8852B 0x30000
#define STA_SCHED_MEM_SIZE 0x1200
#define RXPLD_FLTR_CAM_MEM_SIZE 0x200
#define SECURITY_CAM_MEM_SIZE 0x800
#define WOW_CAM_MEM_SIZE 0x240
#define ADDR_CAM_MEM_SIZE 0x4000
#define TXD_FIFO_SIZE	0x200

#define B_AX_AXIDMA_INT_SEL_SH 22
#define B_AX_AXIDMA_INT_SEL_MSK 0x7

enum mac_ax_sram_dbg_sel {
	STA_SCHED_SEL,
	RXPLD_FLTR_CAM_SEL,
	SEC_CAM_SEL,
	WOW_CAM_SEL,
	CMAC_TBL_SEL,
	ADDR_CAM_SEL,
	BSSID_CAM_SEL,
	BA_CAM_SEL,
	BCN_IE_CAM0_SEL,
	SHARED_BUF_SEL,
	DMAC_TBL_SEL,
	SHCUT_MACHDR_SEL,
	BCN_IE_CAM1_SEL,
};

enum mac_ax_dbg_port_sel {
	/* CMAC 0 related */
	MAC_AX_DBG_PORT_SEL_PTCL_C0 = 0,
	MAC_AX_DBG_PORT_SEL_SCH_C0,
	MAC_AX_DBG_PORT_SEL_TMAC_C0,
	MAC_AX_DBG_PORT_SEL_RMAC_C0,
	MAC_AX_DBG_PORT_SEL_RMACST_C0,
	MAC_AX_DBG_PORT_SEL_RMAC_PLCP_C0,
	MAC_AX_DBG_PORT_SEL_TRXPTCL_C0,
	MAC_AX_DBG_PORT_SEL_TX_INFOL_C0,
	MAC_AX_DBG_PORT_SEL_TX_INFOH_C0,
	MAC_AX_DBG_PORT_SEL_TXTF_INFOL_C0,
	MAC_AX_DBG_PORT_SEL_TXTF_INFOH_C0,
	MAC_AX_DBG_PORT_SEL_CMAC_DMA0_C0,
	MAC_AX_DBG_PORT_SEL_CMAC_DMA1_C0,
	/* CMAC 1 related */
	MAC_AX_DBG_PORT_SEL_PTCL_C1,
	MAC_AX_DBG_PORT_SEL_SCH_C1,
	MAC_AX_DBG_PORT_SEL_TMAC_C1,
	MAC_AX_DBG_PORT_SEL_RMAC_C1,
	MAC_AX_DBG_PORT_SEL_RMACST_C1,
	MAC_AX_DBG_PORT_SEL_RMAC_PLCP_C1,
	MAC_AX_DBG_PORT_SEL_TRXPTCL_C1,
	MAC_AX_DBG_PORT_SEL_TX_INFOL_C1,
	MAC_AX_DBG_PORT_SEL_TX_INFOH_C1,
	MAC_AX_DBG_PORT_SEL_TXTF_INFOL_C1,
	MAC_AX_DBG_PORT_SEL_TXTF_INFOH_C1,
	MAC_AX_DBG_PORT_SEL_CMAC_DMA0_C1,
	MAC_AX_DBG_PORT_SEL_CMAC_DMA1_C1,
	/* DLE related */
	MAC_AX_DBG_PORT_SEL_WDE_BUFMGN_FREEPG,
	MAC_AX_DBG_PORT_SEL_WDE_BUFMGN_QUOTA,
	MAC_AX_DBG_PORT_SEL_WDE_BUFMGN_PAGELLT,
	MAC_AX_DBG_PORT_SEL_WDE_BUFMGN_PKTINFO,
	MAC_AX_DBG_PORT_SEL_WDE_QUEMGN_PREPKT,
	MAC_AX_DBG_PORT_SEL_WDE_QUEMGN_NXTPKT,
	MAC_AX_DBG_PORT_SEL_WDE_QUEMGN_QLNKTBL,
	MAC_AX_DBG_PORT_SEL_WDE_QUEMGN_QEMPTY,
	MAC_AX_DBG_PORT_SEL_PLE_BUFMGN_FREEPG,
	MAC_AX_DBG_PORT_SEL_PLE_BUFMGN_QUOTA,
	MAC_AX_DBG_PORT_SEL_PLE_BUFMGN_PAGELLT,
	MAC_AX_DBG_PORT_SEL_PLE_BUFMGN_PKTINFO,
	MAC_AX_DBG_PORT_SEL_PLE_QUEMGN_PREPKT,
	MAC_AX_DBG_PORT_SEL_PLE_QUEMGN_NXTPKT,
	MAC_AX_DBG_PORT_SEL_PLE_QUEMGN_QLNKTBL,
	MAC_AX_DBG_PORT_SEL_PLE_QUEMGN_QEMPTY,
	MAC_AX_DBG_PORT_SEL_PKTINFO,
	/* TXPKT_CTRL related */
	MAC_AX_DBG_PORT_SEL_TXPKT_CTRL0,
	MAC_AX_DBG_PORT_SEL_TXPKT_CTRL1,
	MAC_AX_DBG_PORT_SEL_TXPKT_CTRL2,
	MAC_AX_DBG_PORT_SEL_TXPKT_CTRL3,
	MAC_AX_DBG_PORT_SEL_TXPKT_CTRL4,
	/* PCIE related */
	MAC_AX_DBG_PORT_SEL_PCIE_TXDMA,
	MAC_AX_DBG_PORT_SEL_PCIE_RXDMA,
	MAC_AX_DBG_PORT_SEL_PCIE_CVT,
	MAC_AX_DBG_PORT_SEL_PCIE_EMAC04,
	MAC_AX_DBG_PORT_SEL_PCIE_EMAC5,
	MAC_AX_DBG_PORT_SEL_PCIE_EMAC6,
	MAC_AX_DBG_PORT_SEL_PCIE_EMAC7,
	MAC_AX_DBG_PORT_SEL_PCIE_PNP_IO,
	MAC_AX_DBG_PORT_SEL_PCIE_EMAC814,
	MAC_AX_DBG_PORT_SEL_PCIE_EMAC15,
	MAC_AX_DBG_PORT_SEL_PCIE_EMAC16,
	MAC_AX_DBG_PORT_SEL_PCIE_EMAC17,
	MAC_AX_DBG_PORT_SEL_PCIE_EMAC18,
	/* DISPATCHER related */
	MAC_AX_DBG_PORT_SEL_DSPT_HDT_TX0,
	MAC_AX_DBG_PORT_SEL_DSPT_HDT_TX1,
	MAC_AX_DBG_PORT_SEL_DSPT_HDT_TX2,
	MAC_AX_DBG_PORT_SEL_DSPT_HDT_TX3,
	MAC_AX_DBG_PORT_SEL_DSPT_HDT_TX4,
	MAC_AX_DBG_PORT_SEL_DSPT_HDT_TX5,
	MAC_AX_DBG_PORT_SEL_DSPT_HDT_TX6,
	MAC_AX_DBG_PORT_SEL_DSPT_HDT_TX7,
	MAC_AX_DBG_PORT_SEL_DSPT_HDT_TX8,
	MAC_AX_DBG_PORT_SEL_DSPT_HDT_TX9,
	MAC_AX_DBG_PORT_SEL_DSPT_HDT_TXA,
	MAC_AX_DBG_PORT_SEL_DSPT_HDT_TXB,
	MAC_AX_DBG_PORT_SEL_DSPT_HDT_TXC,
	MAC_AX_DBG_PORT_SEL_DSPT_HDT_TXD,
	MAC_AX_DBG_PORT_SEL_DSPT_CDT_TX0,
	MAC_AX_DBG_PORT_SEL_DSPT_CDT_TX3,
	MAC_AX_DBG_PORT_SEL_DSPT_CDT_TX4,
	MAC_AX_DBG_PORT_SEL_DSPT_CDT_TX5,
	MAC_AX_DBG_PORT_SEL_DSPT_CDT_TX6,
	MAC_AX_DBG_PORT_SEL_DSPT_CDT_TX7,
	MAC_AX_DBG_PORT_SEL_DSPT_CDT_TX8,
	MAC_AX_DBG_PORT_SEL_DSPT_CDT_TX9,
	MAC_AX_DBG_PORT_SEL_DSPT_HDT_RX0,
	MAC_AX_DBG_PORT_SEL_DSPT_HDT_RX1,
	MAC_AX_DBG_PORT_SEL_DSPT_HDT_RX2,
	MAC_AX_DBG_PORT_SEL_DSPT_HDT_RX3,
	MAC_AX_DBG_PORT_SEL_DSPT_CDT_RX_P0,
	MAC_AX_DBG_PORT_SEL_DSPT_CDT_RX_P1,
	MAC_AX_DBG_PORT_SEL_DSPT_STF_CTRL,
	MAC_AX_DBG_PORT_SEL_DSPT_ADDR_CTRL,
	MAC_AX_DBG_PORT_SEL_DSPT_WDE_INTF,
	MAC_AX_DBG_PORT_SEL_DSPT_PLE_INTF,
	MAC_AX_DBG_PORT_SEL_DSPT_FLOW_CTRL,
	/*AXIDMAC related*/
	MAC_AX_DBG_PORT_SEL_AXI_TXDMA_CTRL,
	MAC_AX_DBG_PORT_SEL_AXI_RXDMA_CTRL,
	MAC_AX_DBG_PORT_SEL_AXI_MST_WLAN,
	MAC_AX_DBG_PORT_SEL_AXI_INT_WLAN,
	MAC_AX_DBG_PORT_SEL_AXI_PAGE_FLOW_CTRL,

	/* keep last */
	MAC_AX_DBG_PORT_SEL_LAST,
	MAC_AX_DBG_PORT_SEL_MAX = MAC_AX_DBG_PORT_SEL_LAST,
	MAC_AX_DBG_PORT_SEL_INVALID = MAC_AX_DBG_PORT_SEL_LAST,
};

struct mac_ax_dbg_port_info {
	u32 sel_addr;
	u8 sel_byte;
	u32 sel_sh;
	u32 sel_msk;
	u32 srt;
	u32 end;
	u32 inc_num;
	u32 rd_addr;
	u8 rd_byte;
	u32 rd_sh;
	u32 rd_msk;
};

u32 mac_fwcmd_lb(struct mac_ax_adapter *adapter, u32 len, u8 burst);
u32 c2h_sys_cmd_path(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
		     struct rtw_c2h_info *info);
u32 c2h_sys_plat_autotest(struct mac_ax_adapter *adapter,  u8 *buf, u32 len,
			  struct rtw_c2h_info *info);
u32 mac_mem_dump(struct mac_ax_adapter *adapter, enum mac_ax_mem_sel sel,
		 u32 strt_addr, u8 *buf, u32 len, u32 dbg_path);
u32 mac_get_mem_size(struct mac_ax_adapter *adapter, enum mac_ax_mem_sel sel);
//u32 mac_reg_dump(struct mac_ax_adapter *adapter, enum mac_ax_reg_sel sel);
u8 chk_dbg_port_valid(struct mac_ax_adapter *adapter, u32 dbg_sel);
void mac_dbg_status_dump(struct mac_ax_adapter *adapter,
			 struct mac_ax_dbgpkg *val,
			 struct mac_ax_dbgpkg_en *en);
u32 dbg_port_sel(struct mac_ax_adapter *adapter,
		 struct mac_ax_dbg_port_info **info, u32 sel);
u32 mac_sram_dbg_write(struct mac_ax_adapter *adapter, u32 offset,
		       u32 val, enum mac_ax_sram_dbg_sel sel);
u32 mac_sram_dbg_read(struct mac_ax_adapter *adapter, u32 offset,
		      enum mac_ax_sram_dbg_sel sel);
u32 mac_rx_cnt(struct mac_ax_adapter *adapter,
	       struct mac_ax_rx_cnt *rxcnt);
u32 mac_dump_fw_rsvd_ple(struct mac_ax_adapter *adapter, u8 **buf);
u32 mac_fw_dbg_dump(struct mac_ax_adapter *adapter,
		    u8 **buf,
		    struct mac_ax_fwdbg_en *en);
#endif
