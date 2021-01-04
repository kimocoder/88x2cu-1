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
#ifndef _PHL_CUSTOM_H_
#define _PHL_CUSTOM_H_

#ifdef CONFIG_PHL_CUSTOM_FEATURE
enum phl_customer_feature_id {
	PHL_CUS_ID_NONE = 0,
	PHL_CUS_ID_FB = 1,
	PHL_CUS_ID_MAX
};
struct phl_custom_evt_rpt {
        u32 evt_id;
        u32 customer_id;
        u32 len;
        u8 val[1];
};

struct phl_custom_cmd {
        u32 evt_id;
        u32 customer_id;
        u32 len;
        u8 val[1];
};

enum rtw_phl_status
phl_register_custom_module(struct phl_info_t *phl_info, u8 band_idx);
enum rtw_phl_status phl_custom_prepare_evt_rpt(void *custom_ctx, u32 evt_id,
                               u32 customer_id, u8 *rpt, u32 rpt_len);
#else
#define phl_register_custom_module(_phl_info, _band_idx) (RTW_PHL_STATUS_SUCCESS)
#endif

#endif  /*_PHL_CUSTOMIZE_FEATURE_H_*/

