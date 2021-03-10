/******************************************************************************
 *
 * Copyright(c) 2020 Realtek Corporation.
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
#ifndef _PHL_REGULATION_DEF_H_
#define _PHL_REGULATION_DEF_H_

#define RSVD_DOMAIN 0x1a

#define MAX_CH_NUM_2GHZ 14

#define MAX_CH_NUM_BAND1 4 /* 36, 40, 44, 48 */
#define MAX_CH_NUM_BAND2 4 /* 52, 56, 60, 64 */
#define MAX_CH_NUM_BAND3 12 /* 100, 104, 108, 112,
				116, 120, 124, 128,
				132, 136, 140, 144 */
#define MAX_CH_NUM_BAND4 8 /* 149, 153, 157, 161, 165, 169, 173, 177 */
#define MAX_CH_NUM_5GHZ (MAX_CH_NUM_BAND1 + MAX_CH_NUM_BAND2 +\
				MAX_CH_NUM_BAND3 + MAX_CH_NUM_BAND4)

#define CH_2GHZ(_ch_) ((_ch_ <= 14) ? true : false)
#define CH_5GHZ(_ch_) ((_ch_ > 14) ? true : false)
#define CH_5GHZ_BAND1(_ch_) (((_ch_ >= 36) && (_ch_ <= 48)) ? true : false)
#define CH_5GHZ_BAND2(_ch_) (((_ch_ >= 52) && (_ch_ <= 64)) ? true : false)
#define CH_5GHZ_BAND3(_ch_) (((_ch_ >= 100) && (_ch_ <= 144)) ? true : false)
#define CH_5GHZ_BAND4(_ch_) (((_ch_ >= 149) && (_ch_ <= 177)) ? true : false)

#define SUPPORT_11A BIT(0)
#define SUPPORT_11B BIT(1)
#define SUPPORT_11G BIT(2)
#define SUPPORT_11N BIT(3)
#define SUPPORT_11AC BIT(4)
#define SUPPORT_11AX BIT(5)

enum regulation_rsn {
	REGU_RSN_DEFAULT = 0x0,
	REGU_RSN_SMBIOS,
	REGU_RSN_EFUSE,
	REGU_RSN_11D,
	REGU_RSN_REGISTRY,
	REGU_RSN_LOCATION,
	REGU_RSN_MANUAL,
	REGU_RSN_MAX
};

enum rtw_regulation_capability {
	CAPABILITY_2GHZ = BIT(0),
	CAPABILITY_5GHZ = BIT(1),
	CAPABILITY_DFS = BIT(2)
};

enum rtw_regulation_query {
	REGULQ_CHPLAN_FULL = 0x0,
	REGULQ_CHPLAN_2GHZ,
	REGULQ_CHPLAN_5GHZ_ALL,
	REGULQ_CHPLAN_5GHZ_BAND1,
	REGULQ_CHPLAN_5GHZ_BAND2,
	REGULQ_CHPLAN_5GHZ_BAND3,
	REGULQ_CHPLAN_5GHZ_BAND4
};

enum ch_property {
	CH_PASSIVE = BIT(0),
	CH_DFS = BIT(1)
};

struct rtw_regulation_channel {
	u8 channel;
	u8 property;
};

struct rtw_regulation_chplan {
	u32 cnt;
	struct rtw_regulation_channel ch[MAX_CH_NUM_2GHZ + MAX_CH_NUM_5GHZ];
};

struct rtw_chlist {
	u32 cnt;
	u8 ch[MAX_CH_NUM_2GHZ + MAX_CH_NUM_5GHZ];
};

struct rtw_regulation_info {
	u8 domain_code;
	u8 domain_reason;
	char country[2];
	u8 support_mode;
	u8 regulation_2g;
	u8 regulation_5g;
	u8 chplan_ver;
	u8 country_ver;
	u16 capability;
};

struct rtw_regulation_country_chplan {
	u8 domain_code;
	u8 support_mode;
	/*
	* bit0: accept 11a
	* bit1: accept 11b
	* bit2: accept 11g
	* bit3: accept 11n
	* bit4: accept 11ac
	* bit5: accept 11ax
	*/
};

struct rtw_user_def_chplan {
	u16 ch2g; /* bit0 ~ bit13 : ch1~ch14 */
	u16 passive2g; /* bit0 ~ bit13 : ch1~ch14, if value = 1, means passive for that channel */

	/* 5g channels.
	 * bit0~7 : ch 36/40/44/48/52/56/60/64
	 * bit8~15 : ch 100/104/108/112/116/120/124/128
	 * bit16~23 : ch 132/136/140/144/149/153/157/161
	 * bit24~27 : ch 165/169/173/177
	 */
	u32 ch5g;
	u32 passive5g;
	u32 dfs5g;
};

/*
 * NOTE:
 * 	This api prototype will be removed after hal related API/header is added
 * 	for halrf.
 */
bool rtw_phl_query_regulation_info(void *phl, struct rtw_regulation_info *info);

#endif /* _PHL_REGULATION_DEF_H_ */
