/******************************************************************************
 *
 * Copyright(c) 2019 - 2020 Realtek Corporation.
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
#ifndef __RTW_PHL_CMD__
#define __RTW_PHL_CMD__
u32 rtw_enqueue_phl_cmd(struct cmd_obj *pcmd);

#ifdef CONFIG_IOCTL_CFG80211
int rtw_phl_remain_on_ch_cmd(_adapter *padapter, u64 cookie, struct wireless_dev *wdev,
	struct ieee80211_channel *ch, u8 ch_type, unsigned int duration,
	struct back_op_param *bkop_parm, u8 is_p2p);
#endif

int rtw_phl_cancel_remain_on_ch_cmd(_adapter *padapter);

#endif /* __RTW_PHL_CMD__ */

