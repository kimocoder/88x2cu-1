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

#ifndef _MAC_AX_USB_H_
#define _MAC_AX_USB_H_

#include "../type.h"
#define phyoffset            0x20
#define USB11                0x1
#define USB2                 0x2
#define USB3                 0x3
#define U3SWITCHU2           0x13
#define U2SWITCHU3_ACUT      0x03
#define U2SWITCHU3_BCUT      0x1B
#define SWITCHMODE           0x2
#define FORCEUSB3MODE        0x1
#define FORCEUSB2MODE        0x0
#define USBEPMAPERR          0xFF
#define USB11_BULKSIZE       0x2
#define USB2_BULKSIZE        0x1
#define USB3_BULKSIZE        0x0
#define EP4                  0x4
#define EP5                  0x5
#define EP6                  0x6
#define EP7                  0x7
#define EP8                  0x8
#define EP9                  0x9
#define EP10                 0xA
#define EP11                 0xB
#define EP12                 0xC
#define NUMP                 0x1

struct wd_body_usb {
	u32 dword0;
	u32 dword1;
	u32 dword2;
	u32 dword3;
	u32 dword4;
	u32 dword5;
};

u8 reg_read8_usb(struct mac_ax_adapter *adapter, u32 addr);
void reg_write8_usb(struct mac_ax_adapter *adapter, u32 addr, u8 val);
u16 reg_read16_usb(struct mac_ax_adapter *adapter, u32 addr);
void reg_write16_usb(struct mac_ax_adapter *adapter, u32 addr, u16 val);
u32 reg_read32_usb(struct mac_ax_adapter *adapter, u32 addr);
void reg_write32_usb(struct mac_ax_adapter *adapter, u32 addr, u32 val);
u8 get_bulkout_id(struct mac_ax_adapter *adapter, u8 ch_dma, u8 mode);
u32 usb_pre_init(struct mac_ax_adapter *adapter, void *param);
u32 usb_init(struct mac_ax_adapter *adapter, void *param);
u32 usb_deinit(struct mac_ax_adapter *adapter, void *param);
u32 usb_flush_mode(struct mac_ax_adapter *adapter, u8 mode);
u32 read_usb2phy_para(struct mac_ax_adapter *adapter, u16 offset);
u32 write_usb2phy_para(struct mac_ax_adapter *adapter, u16 offset, u8 val);
u32 read_usb3phy_para(struct mac_ax_adapter *adapter, u16 offset, u8 b_sel);
u32 write_usb3phy_para(struct mac_ax_adapter *adapter, u16 offset, u8 b_sel,
		       u8 val);
u32 u2u3_switch(struct mac_ax_adapter *adapter);
u32 get_usb_mode(struct mac_ax_adapter *adapter);
u32 get_usb_support_ability(struct mac_ax_adapter *adapter);
u32 usb_tx_agg_cfg(struct mac_ax_adapter *adapter,
		   struct mac_ax_usb_tx_agg_cfg *agg);
u32 usb_rx_agg_cfg(struct mac_ax_adapter *adapter,
		   struct mac_ax_rx_agg_cfg *cfg);
u32 usb_pwr_switch(void *vadapter,
		   u8 pre_switch, u8 on);
u32 set_usb_wowlan(struct mac_ax_adapter *adapter, enum mac_ax_wow_ctrl w_c);
#endif
