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

#ifndef _MAC_AX_EFUSE_H_
#define _MAC_AX_EFUSE_H_

#include "../type.h"
#include "fwcmd.h"

#define RSVD_EFUSE_SIZE		16
#define RSVD_CS_EFUSE_SIZE	24
#define EFUSE_WAIT_CNT		1000000
#define OTP_PHY_SIZE		0x800

#define BT_1B_ENTRY_SIZE	0x80
#define VER_LEN				6
#define UNLOCK_CODE			0x69

struct mac_efuse_tbl {
	mac_ax_mutex lock;
};

enum efuse_map_sel {
	EFUSE_MAP_SEL_PHY_WL,
	EFUSE_MAP_SEL_PHY_BT,
	EFUSE_MAP_SEL_LOG,
	EFUSE_MAP_SEL_LOG_BT,
	EFUSE_MAP_SEL_PHY_OTP,

	/* keep last */
	EFUSE_MAP_SEL_LAST,
	EFUSE_MAP_SEL_MAX = EFUSE_MAP_SEL_LAST,
	EFUSE_MAP_SEL_INVALID = EFUSE_MAP_SEL_LAST,
};

enum mac_info_offset {
	/*USB*/
	OFS_ADDR_AU = 0x438,
	OFS_PID_AU = 0x432,
	OFS_VID_AU = 0x430,
	/*PCIE*/
	OFS_ADDR_AE = 0x400,
	OFS_DID_AE = 0x408,
	OFS_VID_AE = 0x406,
	OFS_SVID_AE = 0x40A,
	OFS_SMID_AE = 0x40C,
	/*SDIO*/
	OFS_ADDR_AS = 0x41A,
};

enum mac_info_length {
	/*USB*/
	LEN_ADDR_AU = 6,
	LEN_PID_AU = 2,
	LEN_VID_AU = 2,
	/*PCIE*/
	LEN_ADDR_AE = 6,
	LEN_DID_AE = 2,
	LEN_VID_AE = 2,
	LEN_SVID_AE = 2,
	LEN_SMID_AE = 2,
	/*SDIO*/
	LEN_ADDR_AS = 6,
};

enum mac_info_default_value {
	/*USB*/
	VAL_ADDR_AU = 0x0,
	VAL_PID_AU = 0x5A,
	VAL_VID_AU = 0xDA,
	/*PCIE*/
	VAL_ADDR_AE = 0x0,
	VAL_DID_AE = 0x52,
	VAL_VID_AE = 0xEC,
	VAL_SVID_AE = 0xEC,
	VAL_SMID_AE = 0x52,
	/*SDIO*/
	VAL_ADDR_AS = 0x0,
};

enum mac_checksum_offset {
	chksum_offset_1 = 0x1AC,
	chksum_offset_2 = 0x1AD,
};

struct mac_bank_efuse_info {
	/* efuse_param */
	u8 **phy_map;
	u8 **log_map;
	u8 *phy_map_valid;
	u8 *log_map_valid;
	u32 *efuse_end;
	/* hw_info */
	u32 *phy_map_size;
	u32 *log_map_size;
};

enum mac_defeature_offset {
	rx_spatial_stream = 0xB,
	rx_spatial_stream_sh = 0x4,
	rx_spatial_stream_msk = 0x7,
	bandwidth = 0xD,
	bandwidth_sh = 0x0,
	bandwidth_msk = 0x7,
	tx_spatial_stream = 0xD,
	tx_spatial_stream_sh = 0x4,
	tx_spatial_stream_msk = 0x7,
	protocol_80211 = 0x11,
	protocol_80211_sh = 0x2,
	protocol_80211_msk = 0x3,
	NIC_router = 0x11,
	NIC_router_sh = 0x6,
	NIC_router_msk = 0x3,
};

enum mac_cntlr_mode_sel {
	MODE_READ,
	MODE_AUTOLOAD_EN,
	MODE_WRITE,
	MODE_CMP,
};

u32 mac_dump_efuse_map_wl(struct mac_ax_adapter *adapter,
			  enum mac_ax_efuse_read_cfg cfg,
			  u8 *efuse_map);
u32 mac_dump_efuse_map_bt(struct mac_ax_adapter *adapter,
			  enum mac_ax_efuse_read_cfg cfg,
			  u8 *efuse_map);
u32 mac_write_efuse(struct mac_ax_adapter *adapter, u32 addr, u8 val,
		    enum mac_ax_efuse_bank bank);
u32 mac_read_efuse(struct mac_ax_adapter *adapter, u32 addr, u32 size, u8 *val,
		   enum mac_ax_efuse_bank bank);
u32 mac_get_efuse_avl_size(struct mac_ax_adapter *adapter, u32 *size);
u32 mac_get_efuse_avl_size_bt(struct mac_ax_adapter *adapter, u32 *size);
u32 mac_dump_log_efuse(struct mac_ax_adapter *adapter,
		       enum mac_ax_efuse_parser_cfg parser_cfg,
		       enum mac_ax_efuse_read_cfg cfg,
		       u8 *efuse_map, bool is_limit);
u32 mac_read_log_efuse(struct mac_ax_adapter *adapter, u32 addr, u32 size,
		       u8 *val);
u32 mac_write_log_efuse(struct mac_ax_adapter *adapter, u32 addr, u8 val);
u32 mac_dump_log_efuse_bt(struct mac_ax_adapter *adapter,
			  enum mac_ax_efuse_parser_cfg parser_cfg,
			  enum mac_ax_efuse_read_cfg cfg,
			  u8 *efuse_map);
u32 mac_read_log_efuse_bt(struct mac_ax_adapter *adapter, u32 addr, u32 size,
			  u8 *val);
u32 mac_write_log_efuse_bt(struct mac_ax_adapter *adapter, u32 addr, u8 val);
u32 mac_pg_efuse_by_map(struct mac_ax_adapter *adapter,
			struct mac_ax_pg_efuse_info *info,
			enum mac_ax_efuse_read_cfg cfg, bool part,
			bool is_limit);
u32 mac_pg_efuse_by_map_bt(struct mac_ax_adapter *adapter,
			   struct mac_ax_pg_efuse_info *info,
			   enum mac_ax_efuse_read_cfg cfg);
u32 mac_mask_log_efuse(struct mac_ax_adapter *adapter,
		       struct mac_ax_pg_efuse_info *info);
u32 mac_pg_sec_data_by_map(struct mac_ax_adapter *adapter,
			   struct mac_ax_pg_efuse_info *info);
u32 mac_cmp_sec_data_by_map(struct mac_ax_adapter *adapter,
			    struct mac_ax_pg_efuse_info *info);
u32 mac_get_efuse_info(struct mac_ax_adapter *adapter, u8 *efuse_map,
		       enum rtw_efuse_info id, void *value, u32 length,
		       u8 *autoload_status);
u32 mac_set_efuse_info(struct mac_ax_adapter *adapter, u8 *efuse_map,
		       enum rtw_efuse_info id, void *value, u32 length);
u32 mac_read_hidden_rpt(struct mac_ax_adapter *adapter,
			struct mac_defeature_value *rpt);
u32 mac_check_efuse_autoload(struct mac_ax_adapter *adapter,
			     u8 *autoload_status);
u32 mac_pg_simulator(struct mac_ax_adapter *adapter,
		     struct mac_ax_pg_efuse_info *info, u8 *phy_map);
u32 mac_checksum_update(struct mac_ax_adapter *adapter);
u32 mac_checksum_rpt(struct mac_ax_adapter *adapter, u16 *chksum);
u32 mac_set_efuse_ctrl(struct mac_ax_adapter *adapter, bool is_secure);
u32 mac_otp_test(struct mac_ax_adapter *adapter, bool is_OTP_test);
void cfg_efuse_auto_ck(struct mac_ax_adapter *adapter, u8 enable);
u32 efuse_tbl_init(struct mac_ax_adapter *adapter);
u32 efuse_tbl_exit(struct mac_ax_adapter *adapter);

#endif
