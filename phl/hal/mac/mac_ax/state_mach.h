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

#ifndef _MAC_AX_STATE_MACH_H_
#define _MAC_AX_STATE_MACH_H_

struct mac_ax_state_mach {
#define MAC_AX_PWR_OFF	0
#define MAC_AX_PWR_ON	1
#define MAC_AX_PWR_ERR	2
	u8 pwr;
#define MAC_AX_FWDL_IDLE 0
#define MAC_AX_FWDL_CPU_ON 1
#define MAC_AX_FWDL_H2C_PATH_RDY 2
#define MAC_AX_FWDL_PATH_RDY 3
#define MAC_AX_FWDL_INIT_RDY 4
	u8 fwdl;
#define MAC_AX_EFUSE_IDLE 0
#define MAC_AX_EFUSE_PHY 1
#define MAC_AX_EFUSE_LOG_MAP 2
#define MAC_AX_EFUSE_LOG_MASK 3
#define MAC_AX_EFUSE_MAX 4
	u8 efuse;
#define MAC_AX_OFLD_REQ_IDLE 0
#define MAC_AX_OFLD_REQ_H2C_SENT 1
#define MAC_AX_OFLD_REQ_CREATED 2
#define MAC_AX_OFLD_REQ_CLEANED 3
	u8 read_request;
	u8 write_request;
	u8 conf_request;
#define MAC_AX_OFLD_H2C_IDLE 0
#define MAC_AX_OFLD_H2C_SENDING 1
#define MAC_AX_OFLD_H2C_RCVD 2
#define MAC_AX_OFLD_H2C_ERROR 4
	u8 write_h2c;
	u8 conf_h2c;
#define MAC_AX_OFLD_H2C_DONE 3
	u8 read_h2c;
	u8 pkt_ofld;
	u8 efuse_ofld;
	u8 macid_pause;
#define MAC_AX_MCC_EMPTY 0
#define MAC_AX_MCC_STATE_H2C_SENT 1
#define MAC_AX_MCC_STATE_H2C_RCVD 2
#define MAC_AX_MCC_ADD_DONE 3
#define MAC_AX_MCC_START_DONE 4
#define MAC_AX_MCC_STOP_DONE 5
#define MAC_AX_MCC_STATE_ERROR 6
	u8 mcc_group[4];
#define MAC_AX_MCC_REQ_IDLE 0
#define MAC_AX_MCC_REQ_H2C_SENT 1
#define MAC_AX_MCC_REQ_H2C_RCVD 2
#define MAC_AX_MCC_REQ_DONE 3
#define MAC_AX_MCC_REQ_FAIL 4
	u8 mcc_request[4];
#define MAC_AX_FW_RESET_IDLE 0
#define MAC_AX_FW_RESET_RECV 1
#define MAC_AX_FW_RESET_PROCESS 2
#define MAC_AX_FW_RESET_DONE 3
	u8 fw_rst;
#define MAC_AX_AOAC_RPT_IDLE 0
#define MAC_AX_AOAC_RPT_H2C_SENDING 1
#define MAC_AX_AOAC_RPT_H2C_RCVD 2
#define MAC_AX_AOAC_RPT_H2C_DONE 3
#define MAC_AX_AOAC_RPT_ERROR 4
	u8 aoac_rpt;
#define MAC_AX_P2P_ACT_IDLE 0
#define MAC_AX_P2P_ACT_BUSY 1
#define MAC_AX_P2P_ACT_FAIL 2
	u8 p2p_stat;
};

#define MAC_AX_DFLT_SM \
	{MAC_AX_PWR_OFF, MAC_AX_FWDL_IDLE, MAC_AX_EFUSE_IDLE, \
	MAC_AX_OFLD_REQ_IDLE, MAC_AX_OFLD_REQ_IDLE, MAC_AX_OFLD_REQ_IDLE, \
	MAC_AX_OFLD_H2C_IDLE, MAC_AX_OFLD_H2C_IDLE, MAC_AX_OFLD_H2C_IDLE, \
	MAC_AX_OFLD_H2C_IDLE, MAC_AX_OFLD_H2C_IDLE, MAC_AX_OFLD_H2C_IDLE, \
	{MAC_AX_MCC_EMPTY}, {MAC_AX_MCC_REQ_IDLE}, MAC_AX_FW_RESET_IDLE, \
	MAC_AX_AOAC_RPT_IDLE, MAC_AX_P2P_ACT_IDLE}
#endif
