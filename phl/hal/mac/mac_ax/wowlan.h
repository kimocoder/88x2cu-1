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

#ifndef _MAC_AX_WOWLAN_H_
#define _MAC_AX_WOWLAN_H_

#include "../type.h"
#include "fwcmd.h"
#include "role.h"

struct keep_alive {
	u32 keepalive_en:1;
	u32 rsvd0:7;
	u32 packet_id:8;
	u32 period:8;
	u32 mac_id:8;
};

struct disconnect_detect {
	u32 disconnect_detect_en:1;
	u32 tryok_bcnfail_count_en:1;
	u32 disconnect_en:1;
	u32 rsvd0:5;
	u32 mac_id:8;
	u32 check_period:8;
	u32 try_pkt_count:8;
	u32 tryok_bcnfail_count_limit:8;
	u32 rsvd1:24;
};

struct wow_global {
	u32 wow_en:1;
	u32 drop_all_pkt:1;
	u32 rx_parse_after_wake:1;
	u32 rsvd0:5;
	u32 mac_id:8;
	u32 pairwise_sec_algo:8;
	u32 group_sec_algo:8;
	u32 remotectrl_info_content;
	u32 remotectrl_info_more[sizeof(struct
					mac_ax_remotectrl_info_parm_) / 4 - 1];
};

struct gtk_ofld {
	u32 gtk_en:1;
	u32 tkip_en:1;
	u32 ieee80211w_en:1;
	u32 pairwise_wakeup:1;
	u32 rsvd0:4;
	u32 aoac_rep_id:8;
	u32 mac_id:8;
	u32 gtk_rsp_id:8;
	u32 pmf_sa_query_id:8;
	u32 bip_sec_algo:2;
	u32 algo_akm_suit: 8;
	u32 rsvd1: 14;
	u32 gtk_info_content;
	//u32 gtk_info_more[30];
	u32 gtk_info_more[sizeof(struct mac_ax_gtk_info_parm_) / 4 - 1];
};

struct arp_ofld {
	u32 arp_en:1;
	u32 arp_action:1;
	u32 rsvd0:14;
	u32 mac_id:8;
	u32 arp_rsp_id:8;
	u32 arp_info_content:32;
};

struct ndp_ofld {
	u32 ndp_en:1;
	u32 rsvd0:15;
	u32 mac_id:8;
	u32 na_id:8;
	u32 ndp_info_content;
	//u32 ndp_info_more[27];
	u32 ndp_info_more[2 * sizeof(struct mac_ax_ndp_info_parm_) / 4 - 1];
};

struct realwow {
	u32 realwow_en:1;
	u32 auto_wakeup:1;
	u32 rsvd0:22;
	u32 mac_id:8;
	u32 keepalive_id:8;
	u32 wakeup_pattern_id:8;
	u32 ack_pattern_id:8;
	u32 rsvd1:8;
	u32 realwow_info_content;
	u32 realwow_info_more[sizeof(struct mac_ax_realwowv2_info_parm_)
			      / 4 - 1];
};

struct nlo {
	u32 nlo_en:1;
	u32 nlo_32k_en:1;
	u32 ignore_cipher_type:1;
	u32 rsvd0:21;
	u32 mac_id:8;
	u32 nlo_networklistinfo_content;
	u32 nlo_networklistinfo_more[sizeof(struct mac_ax_nlo_networklist_parm_)
				     / 4 - 1];
};

struct wakeup_ctrl {
	u32 pattern_match_en:1;
	u32 magic_en:1;
	u32 hw_unicast_en:1;
	u32 fw_unicast_en:1;
	u32 deauth_wakeup:1;
	u32 rekey_wakeup:1;
	u32 eap_wakeup:1;
	u32 all_data_wakeup:1;
	u32 rsvd0:1;
	u32 rsvd1:15;
	u32 mac_id:8;
};

struct negative_pattern {
	u32 negative_pattern_en:1;
	u32 rsvd0:19;
	u32 pattern_count:4;
	u32 mac_id:8;
	u32 pattern_content:32;
};

struct dev2hst_gpio {
	u32 dev2hst_gpio_en:1;
	u32 gpio_active:1;
	u32 gpio_input_en:1;
	u32 gpio_input_for_low:1;
	u32 disable_inband:1;
	u32 data_pin_wakeup:1;
	u32 rsvd0:2;
	u32 gpio_pulse_en:1;
	u32 gpio_pulse_nonstop:1;
	u32 gpio_duration_unit:1;
	u32 rsvd1:5;
	u32 gpio_num:8;
	u32 gpio_pulse_count:8;
	u32 gpio_pulse_duration:8;
	u32 rsvd2:24;
	u32 customer_id:8;
	u32 rsvd3:24;
	u32 gpio_pulse_en_a:1;
	u32 gpio_duration_unit_a:1;
	u32 gpio_pulse_nonstop_a:1;
	u32 rsvd4:5;
	u32 special_reason_a:8;
	u32 gpio_duration_a:8;
	u32 gpio_pulse_count_a:8;
	u32 gpio_pulse_en_b:1;
	u32 gpio_duration_unit_b:1;
	u32 gpio_pulse_nonstop_b:1;
	u32 rsvd5:5;
	u32 special_reason_b:8;
	u32 gpio_duration_b:8;
	u32 gpio_pulse_count_b:8;
};

struct uphy_ctrl {
	u32 disable_uphy:1;
	u32 handshake_mode:3;
	u32 rsvd0:4;
	u32 rise_hst2dev_dis_uphy:1;
	u32 uphy_dis_delay_unit:1;
	u32 pdn_as_uphy_dis:1;
	u32 pdn_to_enable_uphy:1;
	u32 rsvd1:4;
	u32 hst2dev_gpio_num:8;
	u32 uphy_dis_delay_count:8;
};

struct wowcam_upd {
	u32 r_w: 1;
	u32 idx: 7;
	u32 rsvd0: 24;
	u32 wkfm1: 32;
	u32 wkfm2: 32;
	u32 wkfm3: 32;
	u32 wkfm4: 32;
	u32 crc: 16;
	u32 rsvd1: 6;
	u32 negative_pattern_match: 1;
	u32 skip_mac_hdr: 1;
	u32 uc: 1;
	u32 mc: 1;
	u32 bc: 1;
	u32 rsvd2: 4;
	u32 valid: 1;
};

u32 mac_cfg_wow_wake(struct mac_ax_adapter *adapter,
		     u8 macid,
		     struct mac_ax_wow_wake_info *info,
		     struct mac_ax_remotectrl_info_parm_ *content);
u32 mac_cfg_disconnect_det(struct mac_ax_adapter *adapter,
			   u8 macid,
			   struct mac_ax_disconnect_det_info *info);
u32 mac_cfg_keep_alive(struct mac_ax_adapter *adapter,
		       u8 macid,
		       struct mac_ax_keep_alive_info *info);

u32 get_wake_reason(struct mac_ax_adapter *adapter,
		    u8 *wowlan_wake_reason);
u32 mac_cfg_gtk_ofld(struct mac_ax_adapter *adapter,
		     u8 macid,
		     struct mac_ax_gtk_ofld_info *info,
		     struct mac_ax_gtk_info_parm_ *content);
u32 mac_cfg_arp_ofld(struct mac_ax_adapter *adapter,
		     u8 macid,
		     struct mac_ax_arp_ofld_info *info,
		     void  *parp_info_content);
u32 mac_cfg_ndp_ofld(struct mac_ax_adapter *adapter,
		     u8 macid,
		     struct mac_ax_ndp_ofld_info *info,
		     struct mac_ax_ndp_info_parm_ *content);
u32 mac_cfg_realwow(struct mac_ax_adapter *adapter,
		    u8 macid,
		    struct mac_ax_realwow_info *info,
		    struct mac_ax_realwowv2_info_parm_ *content);
u32 mac_cfg_nlo(struct mac_ax_adapter *adapter,
		u8 macid,
		struct mac_ax_nlo_info *info,
		struct mac_ax_nlo_networklist_parm_ *content);

u32 mac_cfg_dev2hst_gpio(struct mac_ax_adapter *adapter,
			 struct mac_ax_dev2hst_gpio_info *info);
u32 mac_cfg_uphy_ctrl(struct mac_ax_adapter *adapter,
		      struct mac_ax_uphy_ctrl_info *info);
u32 mac_cfg_wowcam_upd(struct mac_ax_adapter *adapter,
		       struct mac_ax_wowcam_upd_info *info);
u32 mac_cfg_wow_sleep(struct mac_ax_adapter *adapter,
		      u8 sleep);
u32 mac_get_wow_fw_status(struct mac_ax_adapter *adapter,
			  u8 *status);
static u32 read_aoac_c2hreg(struct mac_ax_adapter *adapter,
			    struct mac_ax_aoac_report *aoac_rpt);
u32 mac_request_aoac_report(struct mac_ax_adapter *adapter,
			    u8 rx_ready);
u32 mac_read_aoac_report(struct mac_ax_adapter *adapter,
			 struct mac_ax_aoac_report *rpt_buf, u8 rx_ready);
#endif // #define _MAC_AX_WOWLAN_H_
