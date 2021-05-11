/******************************************************************************
 *
 * Copyright(c) 2007 - 2020  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/
#ifndef __HALBB_DBG_H__
#define __HALBB_DBG_H__

#include "../../hal_headers_le.h"

/*@--------------------------[Define] ---------------------------------------*/
#define HALBB_WATCHDOG_PERIOD	2 /*second*/

#define PHY_HIST_SIZE		12
#define PHY_HIST_TH_SIZE	(PHY_HIST_SIZE - 1)

#define LA_CLK_EN 	0x014 /*Just for dbg, will be removed*/
#define LA_CLK_EN_M 	0x1 /*Just for dbg, will be removed*/

#ifdef HALBB_DBG_TRACE_SUPPORT
	#ifdef HALBB_DBCC_SUPPORT
		#define BB_DBG(bb, comp, fmt, ...)     \
			do {\
				if(bb->dbg_component & comp) {\
					_os_dbgdump("[BB][%d]" fmt, bb->bb_phy_idx, ##__VA_ARGS__);\
				} \
			} while (0)
	#else
		#define BB_DBG(bb, comp, fmt, ...)     \
			do {\
				if(bb->dbg_component & comp) {\
					_os_dbgdump("[BB]" fmt, ##__VA_ARGS__);\
				} \
			} while (0)
	#endif

	#define BB_TRACE(fmt, ...)     \
		do {\
			_os_dbgdump("[BB]" fmt, ##__VA_ARGS__);\
		} while (0)
		
	#define BB_WARNING(fmt, ...)     \
		do {\
			_os_dbgdump("[WARNING][BB]" fmt, ##__VA_ARGS__);\
		} while (0)

	#define	BB_DBG_CNSL2(in_cnsl, max_buff_len, used_len, buff_addr, remain_len, fmt, ...)\
		do {								\
			u32 *used_len_tmp = &(used_len);			\
			u32 len_tmp = 0;					\
			if (*used_len_tmp < max_buff_len) {			\
				len_tmp = _os_snprintf(buff_addr, remain_len, fmt, ##__VA_ARGS__); \
				if (in_cnsl) {					\
					*used_len_tmp += len_tmp; 		\
				} else {					\
					BB_TRACE("%s\n", buff_addr); 		\
				}						\
			}\
		} while (0)
#else
	#define BB_DBG
	#define BB_TRACE
	#define BB_WARNING
	#define	BB_DBG_CNSL2(in_cnsl, max_buff_len, used_len, buff_addr, remain_len, fmt, ...)\
		do {								\
			u32 *used_len_tmp = &(used_len);				\
			if (*used_len_tmp < max_buff_len)				\
				*used_len_tmp += _os_snprintf(buff_addr, remain_len, fmt, ##__VA_ARGS__);\
		} while (0)
#endif

#define BB_DBG_VAST(max_buff_len, used_len, buff_addr, remain_len, fmt, ...)\
	do {\
		_os_dbgdump("[CNSL]" fmt, ##__VA_ARGS__);\
	} while (0)

#define	BB_DBG_CNSL(max_buff_len, used_len, buff_addr, remain_len, fmt, ...)\
	do {									\
		u32 *used_len_tmp = &(used_len);				\
		if (*used_len_tmp < max_buff_len)				\
			*used_len_tmp += _os_snprintf(buff_addr, remain_len, fmt, ##__VA_ARGS__);\
	} while (0)

#define	DBGPORT_PRI_3	3	/*@Debug function (the highest priority)*/
#define	DBGPORT_PRI_2	2	/*@Check hang function & Strong function*/
#define	DBGPORT_PRI_1	1	/*Watch dog function*/
#define	DBGPORT_RELEASE	0	/*@Init value (the lowest priority)*/

/*@--------------------------[Enum]------------------------------------------*/
enum bb_dbg_port_ip_t
{
	DBGPORT_IP_TD		= 1,
	DBGPORT_IP_RX_INNER	= 2,
	DBGPORT_IP_TX_INNER	= 3,
	DBGPORT_IP_OUTER	= 4,
	DBGPORT_IP_INTF		= 5,
	DBGPORT_IP_CCK		= 6,
	DBGPORT_IP_BF		= 7,
	DBGPORT_IP_RX_OUTER	= 8,
	DBGPORT_IP_RFC0		= 0X1B,
	DBGPORT_IP_RFC1		= 0X1C,
	DBGPORT_IP_RFC2		= 0X1D,
	DBGPORT_IP_RFC3		= 0X1E,
	DBGPORT_IP_TST		= 0X1F,
};
/*@--------------------------[Structure]-------------------------------------*/
struct bb_dbg_cr_info {
	u32 dbgport_ip;
	u32 dbgport_ip_m;
	u32 dbgport_idx;
	u32 dbgport_idx_m;
	u32 dbgport_val;
	u32 dbgport_val_m;
	u32 clk_en;
	u32 clk_en_m;
	u32 dbgport_en;
	u32 dbgport_en_m;
	/*mac_phy_intf*/
	u32 mac_phy_0;
	//u32 mac_phy_0_m;
	u32 mac_phy_1;
	//u32 mac_phy_1_m;
	u32 mac_phy_2;
	//u32 mac_phy_2_m;
	u32 mac_phy_3;
	//u32 mac_phy_3_m;
	u32 mac_phy_4;
	//u32 mac_phy_4_m;
	u32 mac_phy_5;
	//u32 mac_phy_5_m;
	u32 mac_phy_6;
	//u32 mac_phy_6_m;
};

struct bb_dbg_info {
	bool	cr_recorder_en;
	bool	cr_recorder_rf_en; /*HALRF write BB CR*/
	/*CR init debug control*/
	bool	cr_dbg_mode_en;
	u32	cut_curr_dbg;
	u32	rfe_type_curr_dbg;
	struct bb_dbg_cr_info bb_dbg_cr_i;
};

/*@--------------------------[Prptotype]-------------------------------------*/
struct bb_info;
void halbb_dbg_comp_init(struct bb_info *bb);
void halbb_bb_dbg_port_clock_en(struct bb_info *bb, u8 enable);
u32 halbb_get_bb_dbg_port_idx(struct bb_info *bb);
void halbb_set_bb_dbg_port(struct bb_info *bb, u32 dbg_port);
void halbb_set_bb_dbg_port_ip(struct bb_info *bb, enum bb_dbg_port_ip_t ip);
void halbb_release_bb_dbg_port(struct bb_info *bb);
bool halbb_bb_dbg_port_racing(struct bb_info *bb, u8 curr_dbg_priority);
u32 halbb_get_bb_dbg_port_val(struct bb_info *bb);
void halbb_basic_dbg_message(struct bb_info *bb);
void halbb_basic_profile_dbg(struct bb_info *bb, u32 *_used, char *output, u32 *_out_len);
void halbb_dump_reg_dbg(struct bb_info *bb, char input[][16], u32 *_used, char *output, u32 *_out_len);
void halbb_dd_dump_dbg(struct bb_info *bb, char input[][16], u32 *_used, char *output, u32 *_out_len);
void halbb_show_rx_rate(struct bb_info *bb, char input[][16], u32 *_used,
			      char *output, u32 *_out_len);
void halbb_cmn_dbg(struct bb_info *bb, char input[][16], u32 *_used, char *output, u32 *_out_len);
void halbb_dbg_setting_init(struct bb_info *bb);
void halbb_cr_cfg_dbg_init(struct bb_info *bb);
#endif
