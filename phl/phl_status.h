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
#ifndef _PHL_STATUS_H_
#define _PHL_STATUS_H_

enum rtw_phl_status {
	RTW_PHL_STATUS_SUCCESS, /* 0 */
	RTW_PHL_STATUS_FAILURE, /* 1 */
	RTW_PHL_STATUS_RESOURCE, /* 2 */
	RTW_PHL_STATUS_HAL_INIT_FAILURE, /* 3 */
	RTW_PHL_STATUS_PENDING, /* 4 */
	RTW_PHL_STATUS_FRAME_DROP, /* 5 */
	RTW_PHL_STATUS_INVALID_PARAM, /* 6 */
};

enum phl_mdl_ret_code {
	MDL_RET_SUCCESS = 0,
	MDL_RET_FAIL,
	MDL_RET_IGNORE,
	MDL_RET_PENDING,
};

#endif /*_PHL_STATUS_H_*/

