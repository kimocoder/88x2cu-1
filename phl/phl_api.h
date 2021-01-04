/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
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
 *****************************************************************************/
#ifndef _PHL_API_H_
#define _PHL_API_H_

u8 rtw_phl_read8(void *phl, u32 addr);
u16 rtw_phl_read16(void *phl, u32 addr);
u32 rtw_phl_read32(void *phl, u32 addr);
void rtw_phl_write8(void *phl, u32 addr, u8 val);
void rtw_phl_write16(void *phl, u32 addr, u16 val);
void rtw_phl_write32(void *phl, u32 addr, u32 val);

u32 rtw_phl_read_macreg(void *phl, u32 offset, u32 bit_mask);
void rtw_phl_write_macreg(void *phl,
			u32 offset, u32 bit_mask, u32 data);
u32 rtw_phl_read_bbreg(void *phl, u32 offset, u32 bit_mask);
void rtw_phl_write_bbreg(void *phl,
			u32 offset, u32 bit_mask, u32 data);
u32 rtw_phl_read_rfreg(void *phl,
			enum rf_path path, u32 offset, u32 bit_mask);
void rtw_phl_write_rfreg(void *phl,
			enum rf_path path, u32 offset, u32 bit_mask, u32 data);
enum rtw_phl_status rtw_phl_interrupt_handler(void *phl);
void rtw_phl_enable_interrupt(void *phl);
void rtw_phl_disable_interrupt(void *phl);
bool rtw_phl_recognize_interrupt(void *phl);
void rtw_phl_clear_interrupt(void *phl);
void rtw_phl_restore_interrupt(void *phl);
u8 rtw_phl_SER_inprogress(void *phl);
void rtw_phl_SER_clear_status(void *phl, u32 serstatus);
void rtw_phl_notify_watchdog_status(void *phl, bool inprogress);



#ifdef PHL_PLATFORM_LINUX
void rtw_phl_mac_reg_dump(void *sel, void *phl);
void rtw_phl_bb_reg_dump(void *sel, void *phl);
void rtw_phl_bb_reg_dump_ex(void *sel, void *phl);
void rtw_phl_rf_reg_dump(void *sel, void *phl);
#endif
bool rtw_phl_get_sec_cam(void *phl, u16 num, u8 *buf, u16 size);
bool rtw_phl_get_addr_cam(void *phl, u16 num, u8 *buf, u16 size);

struct rtw_phl_com_t *rtw_phl_get_com(void *phl);
enum rtw_phl_status rtw_phl_init(void *drv_priv, void **phl,
					struct rtw_ic_info *ic_info);
void rtw_phl_deinit(void *phl);

void rtw_phl_watchdog_callback(void *phl);

enum rtw_phl_status rtw_phl_trx_alloc(void *phl);
void rtw_phl_trx_free(void *phl);
void rtw_phl_cap_pre_config(void *phl);
enum rtw_phl_status rtw_phl_preload(void *phl);
enum rtw_phl_status rtw_phl_start(void *phl);
void rtw_phl_stop(void *phl);
bool rtw_phl_is_init_completed(void *phl);

enum rtw_phl_status rtw_phl_suspend(void *phl, struct rtw_phl_stainfo_t *sta, u8 wow_en);
enum rtw_phl_status rtw_phl_resume(void *phl, struct rtw_phl_stainfo_t *sta, u8 *hw_reinit);

enum rtw_phl_status rtw_phl_tx_req_notify(void *phl);
enum rtw_phl_status rtw_phl_add_tx_req(void *phl, struct rtw_xmit_req *tx_req);
void rtw_phl_tx_stop(void *phl);
void rtw_phl_tx_resume(void *phl);
u16 rtw_phl_tring_rsc(void *phl, u16 macid, u8 tid);
u16 rtw_phl_query_new_rx_num(void *phl);
struct rtw_recv_pkt *rtw_phl_query_rx_pkt(void *phl);

void rtw_phl_rx_deferred_In_token(void *phl);
void rtw_phl_post_in_complete(void *phl, void *rxobj, u32 inbuf_len, u8 status_code);
enum rtw_phl_status rtw_phl_return_rxbuf(void *phl, u8* rxpkt);

enum rtw_phl_status  rtw_phl_recycle_tx_buf(void *phl, u8 *tx_buf_ptr);

enum rtw_phl_status
rtw_phl_cfg_tx_ampdu(void *phl, struct rtw_phl_stainfo_t *sta);

void rtw_phl_proc_cmd(void *phl, char proc_cmd,
	struct rtw_proc_cmd *incmd, char *output, u32 out_len);

void rtw_phl_get_fw_ver(void *phl, char *ver_str, u16 len);

/* command thread jobs */
enum rtw_phl_status rtw_phl_job_run_func(void *phl,
	void *func, void *priv, void *parm, char *name);

/*WIFI Role management section*/
u8 rtw_phl_wifi_role_alloc(void *phl, u8 *mac_addr, enum role_type type,
				u8 ridx, struct rtw_wifi_role_t **wifi_role);

enum rtw_phl_status
rtw_phl_wifi_role_change(void *phl, struct rtw_wifi_role_t *wrole,
				enum wr_chg_id chg_id, void *chg_info);

void rtw_phl_wifi_role_free(void *phl, u8 role_idx);

/*WIFI sta_info management section*/

struct rtw_phl_stainfo_t *
rtw_phl_alloc_stainfo_sw(void *phl, u8 *sta_addr,
			struct rtw_wifi_role_t *wrole);

/* function have BUS IO process, can't be called in interrupt context*/
enum rtw_phl_status
rtw_phl_alloc_stainfo_hw(void *phl, struct rtw_phl_stainfo_t *sta);

/*
 * rtw_phl_alloc_stainfo = rtw_phl_alloc_stainfo_sw + rtw_phl_alloc_stainfo_hw
 * function have BUS IO process, can't be called in interrupt context
 */
struct rtw_phl_stainfo_t *
rtw_phl_alloc_stainfo(void *phl, u8 *sta_addr,
			struct rtw_wifi_role_t *wrole);


/* function have BUS IO process, can't be called in interrupt context*/
enum rtw_phl_status
rtw_phl_free_stainfo_hw(void *phl, struct rtw_phl_stainfo_t *sta);
enum rtw_phl_status
rtw_phl_free_stainfo_sw(void *phl, struct rtw_phl_stainfo_t *sta);
/*
 * rtw_phl_free_stainfo = rtw_phl_free_stainfo_hw + rtw_phl_free_stainfo_sw
 * function have BUS IO process, can't be called in interrupt context
 */
enum rtw_phl_status
rtw_phl_free_stainfo(void *phl, struct rtw_phl_stainfo_t *sta);

enum rtw_phl_status
rtw_phl_change_stainfo(void *phl,
	struct rtw_phl_stainfo_t *sta, enum sta_chg_id chg_id, void *param);

/*function have BUS IO process, can't be called in interrupt context*/
enum rtw_phl_status
rtw_phl_update_media_status(void *phl, struct rtw_phl_stainfo_t *sta,
			u8 *sta_addr, bool is_connect);

struct rtw_phl_stainfo_t *
rtw_phl_get_stainfo_self(void *phl, struct rtw_wifi_role_t *wrole);

struct rtw_phl_stainfo_t *
rtw_phl_get_stainfo_by_addr(void *phl, struct rtw_wifi_role_t *wrole, u8 *addr);

struct rtw_phl_stainfo_t *
rtw_phl_get_stainfo_by_macid(void *phl, u16 macid);

u8
rtw_phl_get_sta_rssi(struct rtw_phl_stainfo_t *sta);

void
rtw_phl_stainfo_link_notify(void *phl, struct rtw_wifi_role_t *wrole, bool add, u16 macid);

/*macid management section, temporary for debuge*/
u16
rtw_phl_get_macid_max_num(void *phl);

u16
rtw_phl_wrole_bcmc_id_get(void *phl, struct rtw_wifi_role_t *wrole);

u8
rtw_phl_macid_is_bmc(void *phl, u16 macid);

u8
rtw_phl_macid_is_used(void *phl, u16 macid);



enum rtw_phl_status
rtw_phl_add_key(void *phl, struct rtw_phl_stainfo_t *sta,
			struct phl_sec_param_h *crypt, u8 *keybuf, u8 immediate,
			struct rtw_phl_handler *handler);

enum rtw_phl_status
rtw_phl_del_key(void *phl, struct rtw_phl_stainfo_t *sta,
			struct phl_sec_param_h *crypt, u8 immediate,
			struct rtw_phl_handler *handler);
/* phy msg forwarder functions*/
enum rtw_phl_status rtw_phl_msg_hub_register_recver(void* phl,
		struct phl_msg_receiver* ctx, enum phl_msg_recver_layer layer);
enum rtw_phl_status rtw_phl_msg_hub_update_recver_mask(void* phl,
		enum phl_msg_recver_layer layer, u8* mdl_id, u32 len, u8 clr);
enum rtw_phl_status rtw_phl_msg_hub_deregister_recver(void* phl,
					enum phl_msg_recver_layer layer);
enum rtw_phl_status rtw_phl_msg_hub_send(void* phl,
		struct phl_msg_attribute* attr, struct phl_msg* msg);


u8 rtw_phl_trans_sec_mode(u8 unicast, u8 multicast);

u8 rtw_phl_get_sec_cam_idx(void *phl, struct rtw_phl_stainfo_t *sta,
			u8 keyid, u8 key_type);

/* Test module section */
void rtw_phl_test_init(struct rtw_phl_com_t* phl_com, void *buf);
void rtw_phl_test_deinit(struct rtw_phl_com_t* phl_com, void *buf);
void rtw_phl_test_cmd_process(struct rtw_phl_com_t* phl_com, void *buf,
								u32 buf_len);
void rtw_phl_test_get_submodule_rpt(struct rtw_phl_com_t* phl_com, void *buf,
								u32 buf_len);
void rtw_phl_test_get_rpt(struct rtw_phl_com_t* phl_com, void *buf,
								u32 buf_len);
void rtw_phl_test_txtb_cfg(struct rtw_phl_com_t* phl_com, void *buf,
	u32 buf_len, u8 *cfg_bssid, u16 cfg_aid, u8 cfg_bsscolor);
/* command dispatcher module section*/
enum rtw_phl_status rtw_phl_register_module(void *phl, u8 band_idx,
					enum phl_module_id id,
					struct phl_bk_module_ops* ops);
enum rtw_phl_status rtw_phl_deregister_module(void *phl,u8 band_idx,
					enum phl_module_id id);
/* opt: refer to enum phl_msg_opt */
enum rtw_phl_status rtw_phl_send_msg_to_dispr(void *phl, u8 band_idx, struct phl_msg* msg,
					      struct phl_msg_attribute* attr, u32* msg_hdl);
enum rtw_phl_status rtw_phl_cancel_dispr_msg(void *phl, u8 band_idx, u32* msg_hdl);

enum rtw_phl_status rtw_phl_add_cmd_token_req(void *phl, u8 band_idx,
				struct phl_cmd_token_req* req, u32* req_hdl);
enum rtw_phl_status rtw_phl_cancel_cmd_token(void *phl, u8 band_idx, u32* req_hdl);
enum rtw_phl_status rtw_phl_query_cur_cmd_info(void *phl, u8 band_idx,
					       struct phl_module_op_info* op_info);

enum rtw_phl_status rtw_phl_free_cmd_token(void *phl, u8 band_idx, u32* req_hdl);
enum rtw_phl_status rtw_phl_set_bk_module_info(void *phl, u8 band_idx,
		enum phl_module_id id,	struct phl_module_op_info* op_info);
enum rtw_phl_status rtw_phl_query_bk_module_info(void *phl, u8 band_idx,
		enum phl_module_id id,	struct phl_module_op_info* op_info);

/* BA session management */
void rtw_phl_stop_rx_ba_session(void *phl, struct rtw_phl_stainfo_t *sta,
				u16 tid);
enum rtw_phl_status
rtw_phl_start_rx_ba_session(void *phl, struct rtw_phl_stainfo_t *sta,
			    u8 dialog_token, u16 timeout, u16 start_seq_num,
			    u16 ba_policy, u16 tid, u16 buf_size);
void rtw_phl_rx_bar(void *phl, struct rtw_phl_stainfo_t *sta, u8 tid, u16 seq);
#ifdef RTW_PHL_BCN
enum rtw_phl_status rtw_phl_add_beacon(void *phl, struct rtw_bcn_info_cmn *bcn_cmn);
enum rtw_phl_status rtw_phl_update_beacon(void *phl, u8 bcn_id);
enum rtw_phl_status rtw_phl_free_bcn_entry(void *phl, struct rtw_wifi_role_t *wrole);
#endif

enum rtw_phl_status
rtw_phl_set_ch_bw(struct rtw_wifi_role_t *wifi_role,
	u8 chan, enum channel_width bw, enum chan_offset offset, bool do_rfk);
u8 rtw_phl_get_cur_ch(struct rtw_wifi_role_t *wifi_role);
u8 rtw_phl_get_center_ch(u8 ch,
	enum channel_width bw, enum chan_offset offset);
enum rtw_phl_status
rtw_phl_dfs_hw_tx_pause(struct rtw_wifi_role_t *wifi_role,bool tx_pause);

/*
 * export API from sw cap module
 */
void rtw_phl_final_cap_decision(void *phl);


enum rtw_phl_status
rtw_phl_get_dft_proto_cap(void *phl, u8 hw_band, enum role_type rtype,
				struct protocol_cap_t *role_proto_cap);
enum rtw_phl_status
rtw_phl_get_dft_cap(void *phl, u8 hw_band, struct role_cap_t *role_cap);

void rtw_phl_mac_dbg_status_dump(void *phl, u32 *val, u8 *en);

#ifdef CONFIG_DBCC_SUPPORT
enum rtw_phl_status
rtw_phl_dbcc_test(void *phl, enum dbcc_test_id id, void *param);
#endif
enum rtw_phl_status rtw_phl_force_usb_switch(void *phl, u32 speed);
/* refer enum rtw_usb_speed for definition of speed */
enum rtw_phl_status rtw_phl_get_cur_usb_speed(void *phl, u32 *speed);
/* refer enum phl_usb_ability for definition of ability */
enum rtw_phl_status rtw_phl_get_usb_support_ability(void *phl, u32 *ability);
/*
 * API for config channel info CR
 */
#ifdef CONFIG_PHL_CHANNEL_INFO
enum rtw_phl_status rtw_phl_cfg_chinfo(void *phl, struct rtw_phl_stainfo_t *sta);
#endif /* CONFIG_PHL_CHANNEL_INFO */

void rtw_phl_set_edcca_mode(void *phl, enum rtw_edcca_mode mode);
enum rtw_edcca_mode rtw_phl_get_edcca_mode(void *phl);

bool rtw_phl_valid_regulation_domain(u8 domain);
bool rtw_phl_regulation_set_domain(void *phl, u8 domain,
				       	enum regulation_rsn reason);
bool rtw_phl_regulation_set_country(void *phl, char *country,
					enum regulation_rsn reason);
bool rtw_phl_regulation_set_capability(void *phl,
		enum rtw_regulation_capability capability);
bool rtw_phl_regulation_query_chplan(
			void *phl, enum rtw_regulation_query type,
			struct rtw_chlist *filter,
			struct rtw_regulation_chplan *plan);
bool rtw_phl_query_specific_chplan(u8 domain,
			struct rtw_regulation_chplan *plan);
bool rtw_phl_query_country_chplan(char *country,
			struct rtw_regulation_country_chplan *country_chplan);
bool rtw_phl_generate_scan_instance(struct instance_strategy *strategy,
				struct rtw_regulation_chplan *chplan,
				struct instance *inst);
bool rtw_phl_scan_instance_insert_ch(void *phl, struct instance *inst,
					u8 channel, u8 strategy_period);
bool rtw_phl_regulation_valid_channel(void *phl, u16 channel, u8 reject);
bool rtw_phl_regulation_dfs_channel(void *phl, u16 channel, bool *dfs);
bool rtw_phl_query_regulation_info(void *phl, struct rtw_regulation_info *info);
bool rtw_phl_regulation_query_ch(void *phl, u8 channel,
				struct rtw_regulation_channel *ch);

enum rtw_phl_status rtw_phl_get_mac_addr_efuse(void* phl, u8 *addr);

/**
 * rtw_phl_usb_tx_ep_id - query  USB tx end point index
 * identified by macid, tid and band
 * @macid: input target macid is 0 ~ 127
* @tid: input target tid, range is 0 ~ 7
 * @band: input target band, 0 for band 0 / 1 for band 1
 *
 * returns corresponding end point idx of a specific tid
 */
u8 rtw_phl_usb_tx_ep_id(void *phl, u16 macid, u8 tid, u8 band);

enum rtw_phl_status
rtw_phl_cfg_trx_path(void* phl, enum rf_path tx, u8 tx_nss,
		     enum rf_path rx, u8 rx_nss);

void rtw_phl_reset_stat_ma_rssi(struct rtw_phl_com_t *phl_com);

u8
rtw_phl_get_ma_rssi(struct rtw_phl_com_t *phl_com,
		    enum rtw_rssi_type rssi_type);

bool rtw_phl_chanctx_chk(void *phl, struct rtw_wifi_role_t *wifi_role,
		u8 chan, enum channel_width bw, enum chan_offset offset);
bool rtw_phl_chanctx_add(void *phl, struct rtw_wifi_role_t *wifi_role,
		u8 *chan, enum channel_width *bw, enum chan_offset *offset);
int rtw_phl_chanctx_del(void *phl, struct rtw_wifi_role_t *wifi_role,
						struct rtw_chan_def *chan_def);
enum rtw_phl_status rtw_phl_chanctx_del_no_self(void *phl, struct rtw_wifi_role_t *wifi_role);
int rtw_phl_mr_get_chanctx_num(void *phl, struct rtw_wifi_role_t *wifi_role);
enum rtw_phl_status rtw_phl_mr_get_chandef(void *phl, struct rtw_wifi_role_t *wifi_role,
							struct rtw_chan_def *chandef);

u8 rtw_phl_mr_dump_mac_addr(void *phl,
					struct rtw_wifi_role_t *wifi_role);
u8 rtw_phl_mr_buddy_dump_mac_addr(void *phl,
					struct rtw_wifi_role_t *wifi_role);
enum rtw_phl_status
rtw_phl_mr_rx_filter(void *phl, struct rtw_wifi_role_t *wrole);

enum rtw_phl_status
rtw_phl_mr_state_upt(void *phl, struct rtw_wifi_role_t *wrole);

enum rtw_phl_status
rtw_phl_mr_offch_hdl(void *phl,
		     struct rtw_wifi_role_t *wrole,
		     bool off_ch,
		     void *obj_priv,
		     u8 (*issue_null_data)(void *priv, u8 ridx, bool ps),
		     struct rtw_chan_def *chandef);

void rtw_phl_mr_ops_init (void *phl, struct rtw_phl_mr_ops *mr_ops);

#ifdef	PHL_MR_PROC_CMD
void rtw_phl_mr_dump_info(void *phl, bool show_caller);
void rtw_phl_mr_dump_band_ctl(void *phl, bool show_caller);
bool rtw_phl_chanctx_test(void *phl, struct rtw_wifi_role_t *wifi_role, bool is_add,
		u8 *chan, enum channel_width *bw, enum chan_offset *offset);
#endif
void rtw_phl_sta_dump_info(void *phl, bool show_caller, struct rtw_wifi_role_t *wr, u8 mode);

void rtw_phl_mr_dump_cur_chandef(void *phl, struct rtw_wifi_role_t *wifi_role);

void rtw_phl_led_set_ctrl_mode(void *phl, enum rtw_led_id led_id,
			       enum rtw_led_ctrl_mode ctrl_mode);
void rtw_phl_led_set_action(void *phl, enum rtw_led_event event,
			    enum rtw_led_state state_condition, enum rtw_led_id led_id,
			    enum rtw_led_action led_action, u32 *intervals,
			    u8 intervals_len);
void rtw_phl_led_control(void *phl, enum rtw_led_event led_event);

#ifdef CONFIG_RTW_ACS
u16 rtw_phl_acs_get_channel_by_idx(void *phl, u8 ch_idx);
u8 rtw_phl_acs_get_clm_ratio_by_idx(void *phl, u8 ch_idx);
s8  rtw_phl_noise_query_by_idx(void *phl, u8 ch_idx);
#endif /* CONFIG_RTW_ACS */

#ifdef RTW_WKARD_DYNAMIC_BFEE_CAP
enum rtw_phl_status
rtw_phl_bfee_ctrl(void *phl, struct rtw_wifi_role_t *wrole, bool ctrl);
#endif

enum rtw_phl_status
rtw_phl_snd_init_ops_send_ndpa(void *phl, void *snd_send_ndpa);

u8 rtw_phl_snd_chk_in_progress(void *phl);

enum rtw_phl_status
rtw_phl_sound_start(void *phl, u8 wrole_idx, u8 st_dlg_tkn, u8 period, u8 test_flag);

enum rtw_phl_status
rtw_phl_sound_abort(void *phl);


void rtw_phl_init_ppdu_sts_para(struct rtw_phl_com_t *phl_com,
				bool en_psts_per_pkt, bool psts_ampdu,
				u8 rx_fltr);

enum rtw_phl_status rtw_phl_radio_on(void *phl);
enum rtw_phl_status rtw_phl_radio_off(void *phl);


enum rtw_phl_status
rtw_phl_beacon_stop(void *phl, struct rtw_wifi_role_t *wrole, u8 stop);

#endif /*_PHL_API_H_*/

