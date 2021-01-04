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
#define _PHL_SEC_C_
#include "phl_headers.h"

#define RTW_PHL_EXT_KEY_LEN 32
#define RTW_SEC_KEY_TYPE_NUM 3

enum rtw_phl_status
rtw_phl_add_key(void *phl, struct rtw_phl_stainfo_t *sta,
			struct phl_sec_param_h *crypt, u8 *keybuf, u8 immediate,
			struct rtw_phl_handler *handler)
{
    struct phl_info_t *phl_info = (struct phl_info_t *)phl;
    enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

    if (immediate) {
		PHL_INFO("rtw_phl_add_key enc_type(%d) key_id(%d) key_type(%d)\n", 
					crypt->enc_type, crypt->keyid, crypt->key_type);
        hal_status = rtw_hal_set_key(phl_info->hal, sta,
                        crypt->enc_type,
                        (crypt->key_len==RTW_PHL_EXT_KEY_LEN)?1:0,
                        crypt->spp, crypt->keyid, crypt->key_type, keybuf);
    } else {
        /* cmd obj */
    }

    /* ToDo: callback function after h2c */
    if (hal_status == RTW_HAL_STATUS_SUCCESS) {
            if (handler)
                handler->callback((void *)&handler->os_handler);
    }
    return (enum rtw_phl_status)hal_status;
}

enum rtw_phl_status
rtw_phl_del_key(void *phl, struct rtw_phl_stainfo_t *sta,
			struct phl_sec_param_h *crypt, u8 immediate,
			struct rtw_phl_handler *handler)
{
    struct phl_info_t *phl_info = (struct phl_info_t *)phl;
    enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

    if (immediate) {
        hal_status = rtw_hal_set_key(phl_info->hal, sta,
                        crypt->enc_type,
                        (crypt->key_len==RTW_PHL_EXT_KEY_LEN)?1:0,
                        crypt->spp, crypt->keyid, crypt->key_type, NULL);
    } else {
        /* cmd obj */
    }

    /* ToDo: callback function after h2c */
    if (hal_status == RTW_HAL_STATUS_SUCCESS) {
        if (handler)
            handler->callback((void *)&handler->os_handler);
    }
    return (enum rtw_phl_status)hal_status;
}

u8 rtw_phl_trans_sec_mode(u8 unicast, u8 multicast)
{
	u8	ret = RTW_SEC_ENT_MODE_0;

	if (RTW_ENC_NONE == unicast && RTW_ENC_NONE == multicast) {
		ret = RTW_SEC_ENT_MODE_0;
	} else if ((RTW_ENC_WEP40 == unicast && RTW_ENC_WEP40 == multicast) ||
		(RTW_ENC_WEP104 == unicast && RTW_ENC_WEP104 == multicast)) {
		ret = RTW_SEC_ENT_MODE_1;
	} else if (RTW_ENC_WEP40 == multicast || RTW_ENC_WEP104 == multicast) {
		ret = RTW_SEC_ENT_MODE_3;
	} else {
		ret = RTW_SEC_ENT_MODE_2;
	}

	return ret;
}

u8 rtw_phl_get_sec_cam_idx(void *phl, struct rtw_phl_stainfo_t *sta,
			u8 keyid, u8 key_type)
{
	u8 ret = 0, i = 0;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	ret = (u8) rtw_hal_search_key_idx(phl_info->hal, sta, keyid, key_type);

	return ret;
}
