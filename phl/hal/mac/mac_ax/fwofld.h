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

#ifndef _MAC_AX_FW_OFLD_H_
#define _MAC_AX_FW_OFLD_H_

#include "../type.h"
#include "fwcmd.h"
#include "fwofld.h"
#include "trx_desc.h"

#define READ_OFLD_MAX_LEN 2000
#define WRITE_OFLD_MAX_LEN 2000
#define CONF_OFLD_MAX_LEN 2000

#define CONF_OFLD_RESTORE 0
#define CONF_OFLD_BACKUP 1

enum PKT_OFLD_OP {
	PKT_OFLD_OP_ADD = 0,
	PKT_OFLD_OP_DEL = 1,
	PKT_OFLD_OP_READ = 2,
	PKT_OFLD_OP_MAX
};

enum FW_OFLD_OP {
	FW_OFLD_OP_DUMP_EFUSE = 0,
	FW_OFLD_OP_PACKET_OFLD = 1,
	FW_OFLD_OP_READ_OFLD = 2,
	FW_OFLD_OP_WRITE_OFLD = 3,
	FW_OFLD_OP_CONF_OFLD = 4,
	FW_OFLD_OP_MAX
};

struct mac_ax_conf_ofld_hdr {
	u16 pattern_count;
	u16 rsvd;
};

struct mac_ax_pkt_ofld_hdr {
	u8 pkt_idx;
	u8 pkt_op:3;
	u8 rsvd:5;
	u16 pkt_len;
};

u32 mac_reset_fwofld_state(struct mac_ax_adapter *adapter, u8 op);
u32 mac_check_fwofld_done(struct mac_ax_adapter *adapter, u8 op);
u32 mac_clear_write_request(struct mac_ax_adapter *adapter);
u32 mac_add_write_request(struct mac_ax_adapter *adapter,
			  struct mac_ax_write_req *req,
			  u8 *value, u8 *mask);
u32 mac_write_ofld(struct mac_ax_adapter *adapter);
u32 mac_clear_conf_request(struct mac_ax_adapter *adapter);
u32 mac_add_conf_request(struct mac_ax_adapter *adapter,
			 struct mac_ax_conf_ofld_req *req);
u32 mac_conf_ofld(struct mac_ax_adapter *adapter);
u32 mac_read_pkt_ofld(struct mac_ax_adapter *adapter, u8 id);
u32 mac_del_pkt_ofld(struct mac_ax_adapter *adapter, u8 id);
u32 mac_add_pkt_ofld(struct mac_ax_adapter *adapter, u8 *pkt, u16 len, u8 *id);
u32 mac_pkt_ofld_packet(struct mac_ax_adapter *adapter,
			u8 **pkt_buf, u16 *pkt_len, u8 *pkt_id);
u32 mac_dump_efuse_ofld(struct mac_ax_adapter *adapter, u32 efuse_size,
			bool is_hidden);
u32 mac_efuse_ofld_map(struct mac_ax_adapter *adapter, u8 *efuse_map,
		       u32 efuse_size);
u32 mac_clear_read_request(struct mac_ax_adapter *adapter);
u32 mac_add_read_request(struct mac_ax_adapter *adapter,
			 struct mac_ax_read_req *req);
u32 mac_read_ofld(struct mac_ax_adapter *adapter);
u32 mac_read_ofld_value(struct mac_ax_adapter *adapter,
			u8 **val_buf, u16 *val_len);
u32 mac_general_pkt_ids(struct mac_ax_adapter *adapter,
			struct mac_ax_general_pkt_ids *ids);
#endif
