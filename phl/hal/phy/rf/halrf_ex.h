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

#ifndef __HALRF_EX_H__
#define __HALRF_EX_H__

struct rf_info;

/**************halrf.c**************/
void halrf_watchdog(void *rf_void);
enum rtw_hal_status halrf_chl_rfk_trigger(void *rf_void,
			   enum phl_phy_idx phy_idx,
			   bool force);
enum rtw_hal_status halrf_dack_trigger(void *rf_void, bool force);
enum rtw_hal_status halrf_rx_dck_trigger(void *rf_void,
				enum phl_phy_idx phy_idx, bool is_afe);
enum rtw_hal_status halrf_iqk_trigger(void *rf_void,
		       enum phl_phy_idx phy_idx,
		       bool force);
void halrf_lck_trigger(void *rf_void);
enum rtw_hal_status halrf_dpk_trigger(void *rf_void,
		       enum phl_phy_idx phy_idx,
		       bool force);
enum rtw_hal_status halrf_tssi_trigger(void *rf_void, enum phl_phy_idx phy_idx);
void halrf_do_tssi_scan(void *rf_void, enum phl_phy_idx phy_idx);
void halrf_tssi_enable(void *rf_void, enum phl_phy_idx phy_idx);
void halrf_tssi_disable(void *rf_void, enum phl_phy_idx phy_idx);
enum rtw_hal_status halrf_gapk_trigger(void *rf_void,
			enum phl_phy_idx phy_idx, bool force);
enum rtw_hal_status halrf_gapk_enable(void *rf_void,
			enum phl_phy_idx phy_idx);
enum rtw_hal_status halrf_gapk_disable(void *rf_void,
			enum phl_phy_idx phy_idx);
void halrf_rck_trigger(void *rf_void, enum phl_phy_idx phy_idx);
void halrf_iqk_onoff(void *rf_void, bool is_enable);
void halrf_dpk_onoff(void *rf_void, bool is_enable);
void halrf_dpk_track_onoff(void *rf_void, bool is_enable);
void halrf_dpk_switch(void *rf_void, bool is_enable);
void halrf_dpk_init(void *rf_void);
void halrf_rx_dck_onoff(void *rf_void, bool is_enable);
void halrf_gapk_onoff(void *rf_void, bool is_enable);
void halrf_dack_onoff(void *rf_void, bool is_enable);
void halrf_tssi_onoff(void *rf_void, bool is_enable);
bool halrf_get_iqk_onoff(void *rf_void);
bool halrf_get_dpk_onoff(void *rf_void);
bool halrf_get_dpk_track_onoff(void *rf_void);
bool halrf_get_gapk_onoff(void *rf_void);
bool halrf_get_dack_onoff(void *rf_void);
bool halrf_get_tssi_onoff(void *rf_void);
void halrf_lo_test(void *rf_void, bool is_on, enum rf_path path);
s8 halrf_get_power_by_rate(struct rf_info *rf,
	enum phl_phy_idx phy,
	u8 rf_path, u16 rate, u8 dcm, u8 offset);
s8 halrf_get_power_by_rate_band(struct rf_info *rf,
	enum phl_phy_idx phy, u16 rate, u8 dcm, u8 offset, u32 band);
s8 halrf_get_power_limit(struct rf_info *rf,
	enum phl_phy_idx phy, u8 rf_path, u16 rate, u8 bandwidth,
	u8 beamforming, u8 tx_num, u8 channel);
s8 halrf_get_power_limit_ru(struct rf_info *rf,
	enum phl_phy_idx phy, u8 rf_path, u16 rate, u8 bandwidth,
	u8 tx_num, u8 channel);
s16 halrf_get_power(void *rf_void,
	u8 rf_path, u16 rate, u8 dcm, u8 offset, u8 bandwidth,
	u8 beamforming, u8 channel);
u8 halrf_get_thermal(void *rf_void, u8 rf_path);
u32 halrf_get_tssi_de(void *rf_void, enum phl_phy_idx phy_idx, u8 path);
s32 halrf_get_online_tssi_de(void *rf_void, enum phl_phy_idx phy_idx, u8 path, s32 dbm, s32 puot);
void halrf_set_tssi_de_for_tx_verify(void *rf_void, enum phl_phy_idx phy_idx, u32 tssi_de, u8 path);
void halrf_set_tssi_de_offset(void *rf_void, enum phl_phy_idx phy_idx, u32 tssi_de_offset, u8 path);
void halrf_set_tssi_avg_mp(void *rf_void, enum phl_phy_idx phy_idx, s32 xdbm);

void halrf_set_rx_gain_offset_for_rx_verify(void *rf_void, enum phl_phy_idx phy,
					s8 rx_gain_offset, u8 path);
void halrf_set_power_track(void *rf_void, enum phl_phy_idx phy_idx, u8 value);
u8 halrf_get_power_track(void *rf_void);
void halrf_tssi_get_efuse_ex(void *rf_void, enum phl_phy_idx phy_idx);
bool halrf_tssi_check_efuse_data(void *rf_void, enum phl_phy_idx phy_idx);
void halrf_set_ref_power_to_struct(void *rf_void, enum phl_phy_idx phy_idx);
void halrf_bf_config_rf(void *rf_void);
bool halrf_rfc_reg_check_fail(void *rf_void, bool backup);
bool halrf_rfk_reg_check_fail(void *rf_void, bool backup);
bool halrf_dack_reg_check_fail(void *rf_void);
bool halrf_rfk_chl_thermal(void *rf_void, u8 chl_index, u8 ther_index);
void halrf_rfk_recovery_chl_thermal(void *rf_void, u8 chl_index);
u8 halrf_fcs_get_thermal_index(void *rf_void);

/**************halrf_init.c**************/
enum rtw_hal_status halrf_dm_init(void *rf_void);
enum rtw_hal_status  g6_halrf_init(struct rtw_phl_com_t *phl_com,
				struct rtw_hal_com_t *hal_com, void **rf_out);
void g6_halrf_deinit(struct rtw_phl_com_t *phl_com,
		  struct rtw_hal_com_t *hal_com, void *rf);
/**************halrf_hw_cfg.c**************/
bool halrf_init_reg_by_hdr(void *rf_void);
bool halrf_nctl_init_reg_by_hdr(void *rf_void);
bool halrf_config_radio_a_reg(void *rf_void, bool is_form_folder,
				u32 folder_len, u32 *folder_array);
bool halrf_config_radio_b_reg(void *rf_void, bool is_form_folder,
				u32 folder_len, u32 *folder_array);
bool halrf_config_store_power_by_rate(void *rf_void,
		bool is_form_folder, u32 folder_len, u32 *folder_array);
bool halrf_config_store_power_limit(void *rf_void,
		bool is_form_folder, u32 folder_len, u32 *folder_array);
bool halrf_config_store_power_limit_ru(void *rf_void,
		bool is_form_folder, u32 folder_len, u32 *folder_array);
bool halrf_config_store_power_track(void *rf_void,
		bool is_form_folder, u32 folder_len, u32 *folder_array);
void halrf_gapk_save_tx_gain(struct rf_info *rf);
void halrf_gapk_reload_tx_gain(struct rf_info *rf);
/*******************************************/
void halrf_dack_recover(void *rf_void,
			u8 offset,
			enum rf_path path,
			u32 val,
			bool reload);

bool halrf_set_power(struct rf_info *rf, enum phl_phy_idx phy,
	enum phl_pwr_table pwr_table);

bool halrf_get_efuse_power_table_switch(struct rf_info *rf);

void halrf_set_power_table_switch(struct rf_info *rf,
	enum phl_phy_idx phy, u8 pwr_by_rate, u8 pwr_limt);

bool halrf_get_efuse_info(void *rf_void, u8 *efuse_map,
		       enum rtw_efuse_info id, void *value, u32 length,
		       u8 autoload_status);

bool halrf_set_dbcc(void *rf_void, bool dbcc_en);

bool halrf_wl_tx_power_control(void *rf_void, u32 tx_power_val);

void halrf_get_efuse_rx_gain_k(void *rf_void, enum phl_phy_idx phy_idx);

void halrf_get_efuse_trim(void *rf_void, enum phl_phy_idx phy_idx);

void halrf_do_rx_gain_k(void *rf_void, enum phl_phy_idx phy_idx);

enum rtw_hal_status halrf_dpk_tracking(void *rf_void);

enum rtw_hal_status halrf_tssi_tracking(void *rf_void);

u8 halrf_get_default_rfe_type(void *rf_void);

u8 halrf_get_default_xtal(void *rf_void);

void halrf_power_limit_set_ext_pwr_limit_table(struct rf_info *rf,
						enum phl_phy_idx phy);

void halrf_power_limit_set_ext_pwr_limit_ru_table(struct rf_info *rf,
						enum phl_phy_idx phy);
enum rtw_hal_status halrf_iqk_tracking(void *rf_void);

bool halrf_iqk_get_ther_rek(void *rf_void);

void halrf_psd_init(void *rf_void, enum phl_phy_idx phy,
			u8 path, u8 iq_path, u32 avg, u32 fft);
			
void halrf_psd_restore(void *rf_void, enum phl_phy_idx phy);	
			
u32 halrf_psd_get_point_data(void *rf_void, enum phl_phy_idx phy, s32 point);
			
void halrf_psd_query(void *rf_void, enum phl_phy_idx phy,
			u32 point, u32 start_point, u32 stop_point, u32 *outbuf);

void halrf_config_radio_to_fw(void *rf_void);

void halrf_pwr_by_rate_info(struct rf_info *rf,
		char input[][16], u32 *_used, char *output, u32 *_out_len);

void halrf_pwr_limit_info(struct rf_info *rf,
		char input[][16], u32 *_used, char *output, u32 *_out_len);

void halrf_pwr_limit_ru_info(struct rf_info *rf,
		char input[][16], u32 *_used, char *output, u32 *_out_len);

void halrf_disconnect_notify(void *rf_void, struct rtw_chan_def *chandef);
bool  halrf_check_mcc_ch(void *rf_void, struct rtw_chan_def *chandef );
void halrf_ctl_bw(void *rf_void, enum channel_width bw);
void halrf_ctl_ch(void *rf_void, u8 central_ch);
void  halrf_fw_ntfy(void *rf_void, enum phl_phy_idx phy_idx);
u32 halrf_get_nctl_reg_ver(struct rf_info *rf);
u32 halrf_get_radio_reg_ver(struct rf_info *rf);
void halrf_config_nctl_reg(struct rf_info *rf);
void halrf_set_gpio(void *rf_void, enum phl_phy_idx phy, u8 band);
bool halrf_check_efem(void *rf_void, enum phl_phy_idx phy_idx);
void halrf_tssi_default_txagc(void *rf_void,
				enum phl_phy_idx phy_idx, bool enable);
void halrf_tssi_set_avg(void *rf_void,
			enum phl_phy_idx phy_idx, bool enable);
void halrf_wifi_event_notify(void *rf_void,
			enum phl_msg_evt_id event, enum phl_phy_idx phy_idx);
void halrf_2g_rxant(void *rf_void, enum halrf_ant ant);
s8 halrf_get_ther_protected_threshold(void *rf_void);

void halrf_tssi_set_efuse_to_de(void *rf_void,
					enum phl_phy_idx phy_idx);

void halrf_tssi_scan_ch(void *rf_void, enum rf_path path);

bool halrf_mac_set_pwr_reg(void *rf_void, enum phl_phy_idx phy,
	u32 addr, u32 mask, u32 val);

u32 halrf_mac_get_pwr_reg(void *rf_void, enum phl_phy_idx phy,
	u32 addr, u32 mask);

#endif
