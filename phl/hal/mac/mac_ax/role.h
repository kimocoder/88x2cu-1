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

#ifndef _MAC_AX_ROLE_H_
#define _MAC_AX_ROLE_H_

#include "../type.h"
#include "fwcmd.h"
#include "addr_cam.h"
#include "hw.h"

u32 role_tbl_init(struct mac_ax_adapter *adapter);
u32 role_tbl_exit(struct mac_ax_adapter *adapter);
u32 role_info_valid(struct mac_ax_adapter *adapter,
		    struct mac_ax_role_info *info, u8 change_role);
u32 mac_add_role(struct mac_ax_adapter *adapter, struct mac_ax_role_info *info);
u32 mac_remove_role(struct mac_ax_adapter *adapter, u8 macid);
u32 mac_remove_role_by_band(struct mac_ax_adapter *adapter, u8 band, u8 sw);
u32 mac_change_role(struct mac_ax_adapter *adapter,
		    struct mac_ax_role_info *info);
struct mac_role_tbl *mac_role_srch(struct mac_ax_adapter *adapter,
				   u8 macid);
struct mac_role_tbl *mac_role_srch_by_addr_cam(struct mac_ax_adapter *adapter,
					       u8 addr_cam_idx);
struct mac_role_tbl *mac_role_srch_by_bssid(struct mac_ax_adapter *adapter,
					    u8 bssid_cam_idx);

u32 mac_get_macaddr(struct mac_ax_adapter *adapter,
		    struct mac_ax_macaddr *macaddr,
		    u8 role_idx);

u32 mac_set_slot_time(struct mac_ax_adapter *adapter, enum mac_ax_slot_time);

static u32 mac_h2c_join_info(struct mac_ax_adapter *adapter,
			     struct mac_ax_role_info *info);
static u32 mac_fw_role_maintain(struct mac_ax_adapter *adapter,
				struct mac_ax_role_info *info);

#endif
