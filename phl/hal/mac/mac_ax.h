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

#ifndef _MAC_AX_H_
#define _MAC_AX_H_

#include "mac_def.h"
#include "mac_ax/fwcmd.h"
#include "mac_ax/security_cam.h"
#include "mac_ax/efuse.h"
#if MAC_AX_SDIO_SUPPORT
#include "mac_ax/_sdio.h"
#endif
#if MAC_AX_FEATURE_HV
#include "hv_ax/init_hv.h"
#include "hv_ax/fwcmd_hv.h"
#endif

#if MAC_AX_FEATURE_HV
#include "hv_type.h"
#endif

#define MAC_AX_MAJOR_VER	0	/*Software Architcture Modify*/
#define MAC_AX_PROTOTYPE_VER	20	/*New Feature;Regular Release*/
#define MAC_AX_SUB_VER		15	/*for bug fix*/
#define MAC_AX_SUB_INDEX	220	/*for HP branch used*/

#define MAC_AX_SRC_VER(a, b, c, d)                                             \
				(((a) << 24) + ((b) << 16) + ((c) << 8) + (d))

#ifdef CONFIG_NEW_HALMAC_INTERFACE
u32 mac_ax_ops_init_v1(void *phl_adapter, void *drv_adapter,
		       enum rtw_chip_id chip_id,
		       enum rtw_hci_type hci,
		       struct mac_ax_adapter **mac_adapter,
		       struct mac_ax_ops **mac_ops);
#else
u32 mac_ax_ops_init(void *drv_adapter, struct mac_ax_pltfm_cb *pltfm_cb,
		    enum mac_ax_intf intf,
		    struct mac_ax_adapter **mac_adapter,
		    struct mac_ax_ops **mac_ops);

#endif
#if MAC_AX_PHL_H2C
u32 mac_ax_phl_init(void *phl_adapter, struct mac_ax_adapter *mac_adapter);
#endif

u32 mac_ax_ops_exit(struct mac_ax_adapter *adapter);

u32 is_chip_id(struct mac_ax_adapter *adapter, enum mac_ax_chip_id id);

u32 is_chip_cut(struct mac_ax_adapter *adapter, enum rtw_cut_version cut);

#endif
