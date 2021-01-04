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

#ifndef _MAC_AX_CMAC_TX_H_
#define _MAC_AX_CMAC_TX_H_

#include "../type.h"
#include "trxcfg.h"
#include "role.h"
#include "hw.h"

/*--------------------Define ----------------------------------------*/
#define PTCL_TXQ_WMM0_BE	0
#define PTCL_TXQ_WMM0_BK	1
#define PTCL_TXQ_WMM0_VI	2
#define PTCL_TXQ_WMM0_VO	3
#define PTCL_TXQ_WMM1_BE	4
#define PTCL_TXQ_WMM1_BK	5
#define PTCL_TXQ_WMM1_VI	6
#define PTCL_TXQ_WMM1_VO	7
#define PTCL_TXQ_MG0		8
#define PTCL_TXQ_MG1		9
#define PTCL_TXQ_MG2		10
#define PTCL_TXQ_HIQ		11
#define PTCL_TXQ_BCNQ		12
#define PTCL_TXQ_UL		13
#define PTCL_TXQ_TWT0		14
#define PTCL_TXQ_TWT1		15
#define PTCL_TXQ_TB		16

#define TX_PAUSE_WAIT_CNT	1000000

/*--------------------Define Enum------------------------------------*/
enum tb_stop_sel {
	TB_STOP_SEL_BE,
	TB_STOP_SEL_BK,
	TB_STOP_SEL_VI,
	TB_STOP_SEL_VO,
	TB_STOP_SEL_ALL,
};

enum sch_tx_sel {
	SCH_TX_SEL_ALL,
	SCH_TX_SEL_HIQ,
	SCH_TX_SEL_MG0,
	SCH_TX_SEL_MACID,
};

enum ptcl_tx_sel {
	PTCL_TX_SEL_HIQ,
	PTCL_TX_SEL_MG0,
};

/*--------------------Define MACRO----------------------------------*/
/*--------------------Define Struct----------------------------------*/
struct sch_tx_en_h2creg {
	u8 func:7;
	u8 ack:1;
	u8 total_len:4;
	u8 seq_num:4;
	u16 tx_en:16;
	u16 mask:16;
	u8 band:1;
	u16 rsvd:15;
};

/*--------------------Export global variable----------------------------*/
/*--------------------Function declaration-----------------------------*/
u32 set_hw_ampdu_cfg(struct mac_ax_adapter *adapter,
		     struct mac_ax_ampdu_cfg *cfg);
u32 get_hw_ampdu_cfg(struct mac_ax_adapter *adapter,
		     struct mac_ax_ampdu_cfg *cfg);
u32 set_hw_usr_edca_param(struct mac_ax_adapter *adapter,
			  struct mac_ax_usr_edca_param *param);
u32 set_hw_edca_param(struct mac_ax_adapter *adapter,
		      struct mac_ax_edca_param *param);
u32 get_hw_edca_param(struct mac_ax_adapter *adapter,
		      struct mac_ax_edca_param *param);
u32 set_hw_edcca_param(struct mac_ax_adapter *adapter,
		       struct mac_ax_edcca_param *param);
u32 get_hw_edcca_param(struct mac_ax_adapter *adapter,
		       struct mac_ax_edcca_param *param);
u32 set_hw_muedca_param(struct mac_ax_adapter *adapter,
			struct mac_ax_muedca_param *param);
u32 get_hw_muedca_param(struct mac_ax_adapter *adapter,
			struct mac_ax_muedca_param *param);
u32 set_hw_muedca_timer(struct mac_ax_adapter *adapter,
			struct mac_ax_muedca_timer *timer);
u32 get_hw_muedca_timer(struct mac_ax_adapter *adapter,
			struct mac_ax_muedca_timer *timer);
u32 set_hw_muedca_ctrl(struct mac_ax_adapter *adapter,
		       struct mac_ax_muedca_cfg *cfg);
u32 get_hw_muedca_ctrl(struct mac_ax_adapter *adapter,
		       struct mac_ax_muedca_cfg *cfg);
u32 set_hw_tb_ppdu_ctrl(struct mac_ax_adapter *adapter,
			struct mac_ax_tb_ppdu_ctrl *ctrl);
u32 get_hw_tb_ppdu_ctrl(struct mac_ax_adapter *adapter,
			struct mac_ax_tb_ppdu_ctrl *ctrl);
u32 set_hw_sch_tx_en(struct mac_ax_adapter *adapter,
		     struct mac_ax_sch_tx_en_cfg *cfg);
u32 hw_sch_tx_en(struct mac_ax_adapter *adapter, u8 band,
		 u16 tx_en_u16, u16 mask_u16);
u32 get_hw_sch_tx_en(struct mac_ax_adapter *adapter,
		     struct mac_ax_sch_tx_en_cfg *cfg);
u32 set_hw_lifetime_cfg(struct mac_ax_adapter *adapter,
			struct mac_ax_lifetime_cfg *cfg);
u32 get_hw_lifetime_cfg(struct mac_ax_adapter *adapter,
			struct mac_ax_lifetime_cfg *cfg);
u32 stop_sch_tx(struct mac_ax_adapter *adapter, enum sch_tx_sel sel,
		struct mac_ax_sch_tx_en_cfg *bak);
u32 resume_sch_tx(struct mac_ax_adapter *adapter,
		  struct mac_ax_sch_tx_en_cfg *bak);
u32 stop_macid_tx(struct mac_ax_adapter *adapter, struct mac_role_tbl *role,
		  enum tb_stop_sel stop_sel, struct macid_tx_bak *bak);
u32 stop_macid_tx_b(struct mac_ax_adapter *adapter, struct mac_role_tbl *role,
		    enum tb_stop_sel stop_sel, struct macid_tx_bak *bak);
u32 resume_macid_tx(struct mac_ax_adapter *adapter, struct mac_role_tbl *role,
		    struct macid_tx_bak *bak);
u32 resume_macid_tx_b(struct mac_ax_adapter *adapter, struct mac_role_tbl *role,
		      struct macid_tx_bak *bak);
u32 tx_idle_poll_macid(struct mac_ax_adapter *adapter,
		       struct mac_role_tbl *role);
u32 tx_idle_poll_band(struct mac_ax_adapter *adapter, u8 band);
u32 tx_idle_poll_sel(struct mac_ax_adapter *adapter, enum ptcl_tx_sel sel,
		     u8 band);
u32 stop_ac_tb_tx(struct mac_ax_adapter *adapter, enum tb_stop_sel stop_sel,
		  struct mac_ax_tb_ppdu_ctrl *ac_dis_bak);
u32 mac_set_cctl_max_tx_time(struct mac_ax_adapter *adapter,
			     struct mac_ax_max_tx_time *tx_time);
u32 mac_get_max_tx_time(struct mac_ax_adapter *adapter,
			struct mac_ax_max_tx_time *tx_time);
#endif
