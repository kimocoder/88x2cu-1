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

#ifndef _TABLEUPD_H2C_H_
#define _TABLEUPD_H2C_H_

#include "../type.h"
#include "fwcmd.h"
#include "trx_desc.h"
#include "addr_cam.h"

#define DLRU_CLASS_GRP_TBL	0
#define ULRU_CLASS_GRP_TBL	1
#define CLASS_RUSTA_INFO	2
#define DLRU_CLASS_FIXED_TBL	3
#define ULRU_CLASS_FIXED_TBL	4
#define CLASS_BA_INFOTBL	5
#define CLASS_MUDECISION_PARA	6
#define CLASS_UL_FIXINFO	7
#define CLASS_F2P_FIXMODE_PARA	8

enum H2C_WLANINFO_SEL {
	DUMPWLANC = BIT0,
	DUMPWLANS = BIT1,
	DUMPWLAND = BIT2
};

enum DLDECISION_RESULT_TYPE {
	DLDECISION_SU_FORCESU = 0,
	DLDECISION_MU_FORCEMU = 1,
	DLDECISION_SU_FORCEMU_FAIL = 2,
	DLDECISION_SU_FORCERU_FAIL = 3,
	DLDECISION_SU_FORCERU_RUARST_RU2SU = 4,
	DLDECISION_RU_FORCERU_RUSRST_FIXTBL = 5,
	DLDECISION_RU_FORCERU = 6,
	DLDECISION_SU_WDINFO_USERATE = 7,
	DLDECISION_SU_PRINULLWD = 8,
	DLDECISION_MU_BYPASS_MUTPCOMPARE = 9,
	DLDECISION_SU_MUTXTIME_PASS_MU_NOTSUPPORT = 10,
	DLDECISION_SU_MUTXTIME_FAIL_RU_NOTSUPPORT = 11,
	DLDECISION_SU_RUARST_RU2SU = 12,
	DLDECISION_RU_RUARST_FIXTBL = 13,
	DLDECISION_MU_TPCOMPARE_RST = 14,
	DLDECISION_RU_TPCOMPARE_RST = 15,
	DLDECISION_SU_TPCOMPARE_RST = 16,
	DLDECISION_MAX = 17
};

u32 mac_upd_ba_infotbl(struct mac_ax_adapter *adapter,
		       struct mac_ax_ba_infotbl *info);
u32 mac_upd_mudecision_para(struct mac_ax_adapter *adapter,
			    struct mac_ax_mudecision_para *info);
u32 mac_upd_ul_fixinfo(struct mac_ax_adapter *adapter,
		       struct mac_ax_ul_fixinfo *info);
u32 mac_f2p_test_cmd(struct mac_ax_adapter *adapter,
		     struct mac_ax_f2p_test_para *info,
		     struct mac_ax_f2p_wd *f2pwd,
		     struct mac_ax_f2p_tx_cmd *ptxcmd,
		     u8 *psigb_addr);
u32 mac_upd_dctl_info(struct mac_ax_adapter *adapter,
		      struct mac_ax_dctl_info *info,
		      struct mac_ax_dctl_info *mask, u8 macid, u8 operation);
u32 mac_upd_shcut_mhdr(struct mac_ax_adapter *adapter,
		       struct mac_ax_shcut_mhdr *info, u8 macid);
u32 mac_upd_cctl_info(struct mac_ax_adapter *adapter,
		      struct mac_ax_cctl_info *info,
		      struct mac_ax_cctl_info *mask, u8 macid, u8 operation);
u32 mac_set_fixmode_mib(struct mac_ax_adapter *adapter,
			struct mac_ax_fixmode_para *info);
u32 mac_snd_test_cmd(struct mac_ax_adapter *adapter,
		     u8 *cmd_buf);
u32 mac_bacam_info(struct mac_ax_adapter *adapter,
		   struct mac_ax_bacam_info *info);
u32 mac_ss_dl_grp_upd(struct mac_ax_adapter *adapter,
		      struct mac_ax_ss_dl_grp_upd *info);
u32 mac_ss_ul_grp_upd(struct mac_ax_adapter *adapter,
		      struct mac_ax_ss_ul_grp_upd *info);
u32 mac_ss_ul_sta_upd(struct mac_ax_adapter *adapter,
		      struct mac_ax_ss_ul_sta_upd *info);
u32 mac_mu_sta_upd(struct mac_ax_adapter *adapter,
		   struct mac_ax_mu_sta_upd *info);
u32 mac_wlaninfo_get(struct mac_ax_adapter *adapter,
		     struct mac_ax_wlaninfo_get *info);
u32 mac_dumpwlanc(struct mac_ax_adapter *adapter,
		  struct mac_ax_dumpwlanc *para);
u32 mac_dumpwlans(struct mac_ax_adapter *adapter,
		  struct mac_ax_dumpwlans *para);
u32 mac_dumpwland(struct mac_ax_adapter *adapter,
		  struct mac_ax_dumpwland *para);

#if MAC_AX_FEATURE_DBGPKG
u32 cctl_info_debug_write(struct mac_ax_adapter *adapter, u8 macid,
			  struct fwcmd_cctlinfo_ud *tbl);
u32 dctl_info_debug_write(struct mac_ax_adapter *adapter, u8 macid,
			  struct fwcmd_dctlinfo_ud *tbl);
#endif
#endif
