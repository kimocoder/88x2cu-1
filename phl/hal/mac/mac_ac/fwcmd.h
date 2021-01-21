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

#ifndef _MAC_AX_FW_CMD_H_
#define _MAC_AX_FW_CMD_H_

#include "../type.h"


//#include "fwcmd_intf.h"
//#include "fwcmd_intf_f2p.h"
//#include "trx_desc.h"
//#include "fwofld.h"

#define FWCMD_HDR_LEN		8

#define H2C_CMD_LEN		64
#define H2C_DATA_LEN		256
#define H2C_LONG_DATA_LEN	2048

#define SET_FWCMD_ID(_t, _ca, _cl, _f)                                         \
		(SET_WORD(_t, H2C_HDR_DEL_TYPE) | SET_WORD(_ca, H2C_HDR_CAT) | \
		 SET_WORD(_cl, H2C_HDR_CLASS) | SET_WORD(_f, H2C_HDR_FUNC))
#define GET_FWCMD_TYPE(id)	(GET_FIELD(id, C2H_HDR_DEL_TYPE))
#define GET_FWCMD_CAT(id)	(GET_FIELD(id, C2H_HDR_CAT))
#define GET_FWCMD_CLASS(id)	(GET_FIELD(id, C2H_HDR_CLASS))
#define GET_FWCMD_FUNC(id)	(GET_FIELD(id, C2H_HDR_FUNC))

#define FWCMD_TYPE_H2C	0
#define FWCMD_TYPE_C2H	1

#define FWCMD_C2H_CL_NULL		0xFF
#define FWCMD_C2H_FUNC_NULL		0xFF

#if 0 // NEO mark off first

enum h2c_buf_class {
	H2CB_CLASS_CMD,		/* FW command */
	H2CB_CLASS_DATA,	/* FW command + data */
	H2CB_CLASS_LONG_DATA,	/* FW command + long data */

	/* keep last */
	H2CB_CLASS_LAST,
	H2CB_CLASS_MAX = H2CB_CLASS_LAST,
	H2CB_CLASS_INVALID = H2CB_CLASS_LAST,
};

struct h2c_buf_head {
	/* keep first */
	struct h2c_buf *next;
	struct h2c_buf *prev;
	u8 *pool;
	u32 size;
	u32 qlen;
	u8 suspend;
	mac_ax_mutex lock;
};

struct fwcmd_wkb_head {
	/* keep first */
	struct h2c_buf *next;
	struct h2c_buf *prev;
	u32 qlen;
	mac_ax_mutex lock;
};

struct h2c_buf {
	/* keep first */
	struct h2c_buf *next;
	struct h2c_buf *prev;
	enum h2c_buf_class _class_;
	u32 id;
	u8 master;
	u32 len;
	u8 *head;
	u8 *end;
	u8 *data;
	u8 *tail;
	u32 hdr_len;
#define H2CB_FLAGS_FREED	BIT(0)
	u32 flags;
	u8 h2c_seq;
};

struct c2h_proc_class {
	u16 id;
	u32 (*handler)(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
		       struct rtw_c2h_info *info);
};

struct c2h_proc_func {
	u16 id;
	u32 (*handler)(struct mac_ax_adapter *adapter, u8 *buf, u32 len,
		       struct rtw_c2h_info *info);
};

struct mac_ax_c2hreg_info {
#define C2HREG_LEN 16
	u8 id;
	u8 total_len;
	u8 *content;
	u8 c2hreg[C2HREG_LEN];
};

u32 h2cb_init(struct mac_ax_adapter *adapter);
u32 h2cb_exit(struct mac_ax_adapter *adapter);
#if MAC_AX_PHL_H2C
struct rtw_h2c_pkt *h2cb_alloc(struct mac_ax_adapter *adapter,
			       enum h2c_buf_class buf_class);
void h2cb_free(struct mac_ax_adapter *adapter, struct rtw_h2c_pkt *h2cb);
u8 *h2cb_push(struct rtw_h2c_pkt *h2cb, u32 len);
u8 *h2cb_pull(struct rtw_h2c_pkt *h2cb, u32 len);
u8 *h2cb_put(struct rtw_h2c_pkt *h2cb, u32 len);
u32 h2c_pkt_set_hdr(struct mac_ax_adapter *adapter, struct rtw_h2c_pkt *h2cb,
		    u8 type, u8 cat, u8 _class_, u8 func, u16 rack, u16 dack);
u32 h2c_pkt_set_hdr_fwdl(struct mac_ax_adapter *adapter,
			 struct rtw_h2c_pkt *h2cb, u8 type, u8 cat, u8 _class_,
			 u8 func, u16 rack, u16 dack);
u32 h2c_pkt_set_cmd(struct mac_ax_adapter *adapter, struct rtw_h2c_pkt *h2cb,
		    u8 *cmd, u32 len);
u32 h2c_pkt_build_txd(struct mac_ax_adapter *adapter, struct rtw_h2c_pkt *h2cb);
#else
struct h2c_buf *h2cb_alloc(struct mac_ax_adapter *adapter,
			   enum h2c_buf_class buf_class);
void h2cb_free(struct mac_ax_adapter *adapter, struct h2c_buf *h2cb);
u8 *h2cb_push(struct h2c_buf *h2cb, u32 len);
u8 *h2cb_pull(struct h2c_buf *h2cb, u32 len);
u8 *h2cb_put(struct h2c_buf *h2cb, u32 len);
u32 h2c_pkt_set_hdr(struct mac_ax_adapter *adapter, struct h2c_buf *h2cb,
		    u8 type, u8 cat, u8 _class_, u8 func, u16 rack, u16 dack);
u32 h2c_pkt_set_hdr_fwdl(struct mac_ax_adapter *adapter, struct h2c_buf *h2cb,
			 u8 type, u8 cat, u8 _class_, u8 func, u16 rack,
			 u16 dack);
u32 h2c_pkt_set_cmd(struct mac_ax_adapter *adapter, struct h2c_buf *h2cb,
		    u8 *cmd, u32 len);
u32 h2c_pkt_build_txd(struct mac_ax_adapter *adapter, struct h2c_buf *h2cb);
#endif
u32 fwcmd_wq_enqueue(struct mac_ax_adapter *adapter, struct h2c_buf *h2cb);
struct h2c_buf *fwcmd_wq_dequeue(struct mac_ax_adapter *adapter, u32 id);
u32 fwcmd_wq_idle(struct mac_ax_adapter *adapter, u32 id);
#endif // if 0 NEO

u32 mac_process_c2h(struct mac_adapter *adapter, u8 *buf, u32 len,
		    u8 *ret);

#if 0 // NEO
u8 c2h_field_parsing(struct fwcmd_hdr *hdr, struct rtw_c2h_info *info);
u32 mac_fw_log_cfg(struct mac_ax_adapter *adapter,
		   struct mac_ax_fw_log *log_cfg);
u32 mac_send_bcn_h2c(struct mac_ax_adapter *adapter,
		     struct mac_ax_bcn_info *info);
u32 mac_host_getpkt_h2c(struct mac_ax_adapter *adapter, u8 macid, u8 pkttype);
u32 mac_outsrc_h2c_common(struct mac_ax_adapter *adapter,
			  struct rtw_g6_h2c_hdr *hdr, u32 *pvalue);
u32 mac_ie_cam_upd(struct mac_ax_adapter *adapter,
		   struct mac_ax_ie_cam_cmd_info *info);
u32 h2c_end_flow(struct mac_ax_adapter *adapter);
u32 mac_send_h2creg(struct mac_ax_adapter *adapter, u32 *content, u8 len);
u32 mac_process_c2hreg(struct mac_ax_adapter *adapter,
		       struct mac_ax_c2hreg_info *info);

#endif // if 0 NEO

#endif
