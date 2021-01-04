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

#ifndef _MAC_AX_ADDR_CAM_H_
#define _MAC_AX_ADDR_CAM_H_

#include "../type.h"
#include "fwcmd.h"
#include "fwcmd_intf.h"

#define ADDR_CAM_ENT_SIZE  0x40
#define BSSID_CAM_ENT_SIZE 0x08

u32 fill_addr_cam_info(struct mac_ax_adapter *adapter,
		       struct mac_ax_role_info *info,
		       struct fwcmd_addrcam_info *fw_addrcam);

u32 fill_bssid_cam_info(struct mac_ax_adapter *adapter,
			struct mac_ax_role_info *info,
			struct fwcmd_addrcam_info *fw_addrcam);

u32 init_addr_cam_info(struct mac_ax_adapter *adapter,
		       struct mac_ax_role_info *info,
		       struct fwcmd_addrcam_info *fw_addrcam);

u32 change_addr_cam_info(struct mac_ax_adapter *adapter,
			 struct mac_ax_role_info *info,
			 struct fwcmd_addrcam_info *fw_addrcam);

u32 mac_upd_addr_cam(struct mac_ax_adapter *adapter,
		     struct mac_ax_role_info *info,
		     u8 change_role);

#endif
