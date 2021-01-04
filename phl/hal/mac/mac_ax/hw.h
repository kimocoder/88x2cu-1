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

#ifndef _MAC_AX_HW_H_
#define _MAC_AX_HW_H_

#define BT_2_DW(B3, B2, B1, B0)	\
	(((B3) << 24) | ((B2) << 16) | ((B1) << 8) | (B0))

#define NIB_2_DW(B7, B6, B5, B4, B3, B2, B1, B0)	\
	((((B7) & 0xf) << 28) | (((B6) & 0xf) << 24) | \
	(((B5) & 0xf) << 20) | (((B4) & 0xf) << 16) | \
	(((B3) & 0xf) << 12) | (((B2) & 0xf) << 8) | \
	(((B1) & 0xf) << 4) | ((B0) & 0xf))

#include "../type.h"
#include "status.h"
#include "wowlan.h"
#include "tblupd.h"

#if MAC_AX_SDIO_SUPPORT
#include "_sdio.h"
#endif

#if MAC_AX_PCIE_SUPPORT
#include "_pcie.h"
#endif

#if MAC_AX_USB_SUPPORT
#include "_usb.h"
#endif

#define BITS_WLRF_CTRL 0x82
#define BITS_WLRF1_CTRL 0x8200
#define PHYREG_SET_ALL_CYCLE 0x8
#define PHYREG_SET_X_CYCLE 0x4
#define PHYREG_SET_N_CYCLE 0x2
#define PHYREG_SET_Y_CYCLE 0x1

#define TXSC_80M 0x91
#define TXSC_40M 0x1
#define TXSC_20M 0x0

#define TBL_READ_OP 0x0
#define TBL_WRITE_OP 0x1
#define TXCNT_LMT_MSK 0x1

#define CHANNEL_5G 34

#define CR_TXCNT_MSK 0x7FFFFFFF

#define XTAL_SI_WL_RFC_S0 0x80
#define XTAL_SI_WL_RFC_S1 0x81

/* For TXPWR Usage*/
#define PWR_BY_RATE_LGCY_OFFSET 0XC0
#define PWR_BY_RATE_OFFSET 0XCC

#define PWR_LMT_CCK_OFFSET 0XEC
#define PWR_LMT_LGCY_OFFSET 0XF0
#define PWR_LMT_TBL2_OFFSET 0XF4
#define PWR_LMT_TBL5_OFFSET 0X100
#define PWR_LMT_TBL6_OFFSET 0X104
#define PWR_LMT_TBL7_OFFSET 0X108
#define PWR_LMT_TBL8_OFFSET 0X10C
#define PWR_LMT_TBL9_OFFSET 0X110

#define PWR_LMT_TBL_UNIT 0X28
#define PWR_BY_RATE_TBL_UNIT 0XF

enum tx_tf_info {
	USER_INFO0_SEL		= 0,
	USER_INFO1_SEL		= 1,
	USER_INFO2_SEL		= 2,
	USER_INFO3_SEL		= 3,
	COMMON_INFO_SEL		= 4,
};

struct mac_ax_hw_info *mac_get_hw_info(struct mac_ax_adapter *adapter);
u32 mac_set_hw_value(struct mac_ax_adapter *adapter,
		     enum mac_ax_hw_id hw_id, void *val);
u32 mac_get_hw_value(struct mac_ax_adapter *adapter,
		     enum mac_ax_hw_id hw_id, void *val);
u32 mac_write_lte(struct mac_ax_adapter *adapter,
		  const u32 offset, u32 val);
u32 mac_read_lte(struct mac_ax_adapter *adapter,
		 const u32 offset, u32 *val);
u32 mac_write_xtal_si(struct mac_ax_adapter *adapter,
		      const u32 offset, u32 val);
u32 mac_read_xtal_si(struct mac_ax_adapter *adapter,
		     const u32 offset, u32 *val);
u32 set_host_rpr(struct mac_ax_adapter *adapter,
		 struct mac_ax_host_rpr_cfg *cfg);
u32 mac_read_pwr_reg(struct mac_ax_adapter *adapter, u8 band,
		     const u32 offset, u32 *val);
u32 mac_write_pwr_reg(struct mac_ax_adapter *adapter, u8 band,
		      const u32 offset, u32 val);
u32 mac_write_pwr_ofst_mode(struct mac_ax_adapter *adapter,
			    u8 band, struct rtw_tpu_info *tpu);
u32 mac_write_pwr_ofst_bw(struct mac_ax_adapter *adapter,
			  u8 band, struct rtw_tpu_info *tpu);
u32 mac_write_pwr_ref_reg(struct mac_ax_adapter *adapter,
			  u8 band, struct rtw_tpu_info *tpu);
u32 mac_write_pwr_limit_en(struct mac_ax_adapter *adapter,
			   u8 band, struct rtw_tpu_info *tpu);
u32 mac_write_pwr_limit_rua_reg(struct mac_ax_adapter *adapter,
				u8 band, struct rtw_tpu_info *tpu);
u32 mac_write_pwr_limit_reg(struct mac_ax_adapter *adapter,
			    u8 band, struct rtw_tpu_pwr_imt_info *tpu);
u32 mac_write_pwr_by_rate_reg(struct mac_ax_adapter *adapter,
			      u8 band, struct rtw_tpu_pwr_by_rate_info *tpu);
u32 mac_lv1_rcvy(struct mac_ax_adapter *adapter,
		 enum mac_ax_lv1_rcvy_step step);
u32 mac_get_err_status(struct mac_ax_adapter *adapter,
		       enum mac_ax_err_info *err);
u32 mac_set_err_status(struct mac_ax_adapter *adapter,
		       enum mac_ax_err_info err);
u32 mac_dump_err_status(struct mac_ax_adapter *adapter,
			enum mac_ax_err_info err);
u32 mac_read_xcap_reg(struct mac_ax_adapter *adapter, u8 sc_xo, u32 *val);
u32 mac_write_xcap_reg(struct mac_ax_adapter *adapter, u8 sc_xo, u32 val);
u32 mac_write_bbrst_reg(struct mac_ax_adapter *adapter, u8 val);
u32 mac_trigger_cmac_err(struct mac_ax_adapter *adapter);
u32 mac_trigger_cmac1_err(struct mac_ax_adapter *adapter);
u32 mac_trigger_dmac_err(struct mac_ax_adapter *adapter);
u32 set_macid_pause(struct mac_ax_adapter *adapter,
		    struct mac_ax_macid_pause_cfg *cfg);
u32 macid_pause(struct mac_ax_adapter *adapter,
		struct mac_ax_macid_pause_grp *grp);
u32 get_macid_pause(struct mac_ax_adapter *adapter,
		    struct mac_ax_macid_pause_cfg *cfg);
u32 get_ss_wmm_tbl(struct mac_ax_adapter *adapter,
		   struct mac_ax_ss_wmm_tbl_ctrl *ctrl);
u32 set_enable_bb_rf(struct mac_ax_adapter *adapter, u8 enable);
u32 set_cctl_rty_limit(struct mac_ax_adapter *adapter,
		       struct mac_ax_cctl_rty_lmt_cfg *cfg);
u32 set_cr_rty_limit(struct mac_ax_adapter *adapter,
		     struct mac_ax_cr_rty_lmt_cfg *cfg);
u32 set_rrsr_val(struct mac_ax_adapter *adapter,
		 struct mac_ax_rrsr_cfg *cfg);
u32 get_rrsr_val(struct mac_ax_adapter *adapter,
		 struct mac_ax_rrsr_cfg *cfg);
u32 cfg_mac_bw(struct mac_ax_adapter *adapter,
	       struct mac_ax_cfg_bw *cfg);

#endif
