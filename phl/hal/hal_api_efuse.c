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
#define _HAL_API_EFUSE_C_
#include "hal_headers.h"
#include "efuse/hal_efuse_export.h"

#if 0 // NEO : TODO : mark off first

/*WIFI Efuse*/
enum rtw_hal_status
rtw_hal_efuse_shadow_load(struct hal_info_t *hal_info, bool is_limit)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;

	status = rtw_efuse_shadow_load(hal_info->efuse, is_limit);

	return status;
}

enum rtw_hal_status
rtw_hal_efuse_shadow_update(struct hal_info_t *hal_info, bool is_limit)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_EFUSE_PG_FAIL;

	status = rtw_efuse_shadow_update(hal_info->efuse, is_limit);

	return status;
}

enum rtw_hal_status
rtw_hal_efuse_shadow_read(struct hal_info_t *hal_info, u8 byte_count,
						  u16 offset, u32 *value, bool is_limit)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;

	status = rtw_efuse_shadow_read(hal_info->efuse, byte_count, offset, value,
								   is_limit);

	return status;
}

enum rtw_hal_status
rtw_hal_efuse_shadow_write(struct hal_info_t *hal_info, u8 byte_count,
						   u16 offset, u32 value, bool is_limit)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;

	status = rtw_efuse_shadow_write(hal_info->efuse, byte_count, offset, value,
									is_limit);
	return status;
}

enum rtw_hal_status
rtw_hal_efuse_shadow2buf(struct hal_info_t *hal_info, u8 *pbuf, u16 buflen)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_SUCCESS;

	status = rtw_efuse_shadow2buf(hal_info->efuse, pbuf, buflen);

	return status;
}

enum rtw_hal_status rtw_hal_efuse_file_map_load(
	struct hal_info_t *hal_info, char *file_path, u8 is_limit)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

	hal_status = rtw_efuse_file_map_load(hal_info->efuse, file_path ,is_limit);

	return hal_status;
}

enum rtw_hal_status rtw_hal_efuse_file_mask_load(
	struct hal_info_t *hal_info, char *file_path, u8 is_limit)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

	hal_status = rtw_efuse_file_mask_load(hal_info->efuse, file_path, is_limit);

	return hal_status;
}

/* usage = used percentage(1 Byte) + used bytes(2 Bytes) */
enum rtw_hal_status rtw_hal_efuse_get_usage(struct hal_info_t *hal_info,
	u32 *usage)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;

	status = rtw_efuse_get_usage(hal_info->efuse, usage);

	return status;
}

enum rtw_hal_status rtw_hal_efuse_get_logical_size(struct hal_info_t *hal_info,
	u32 *size)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;

	status = rtw_efuse_get_logical_size(hal_info->efuse, size, true);

	return status;
}

enum rtw_hal_status rtw_hal_efuse_get_size(struct hal_info_t *hal_info,
	u32 *size)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;

	status = rtw_efuse_get_size(hal_info->efuse, size);

	return status;
}

enum rtw_hal_status rtw_hal_efuse_get_avl(struct hal_info_t *hal_info,
	u32 *size)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;

	status = rtw_efuse_get_avl(hal_info->efuse, size);

	return status;
}

enum rtw_hal_status rtw_hal_efuse_get_shadowmap_from(struct hal_info_t *hal_info,
	u8 *val)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;

	status = rtw_efuse_get_shadowmap_from(hal_info->efuse, val);

	return status;
}

enum rtw_hal_status rtw_hal_efuse_get_offset_mask(struct hal_info_t *hal_info,
	u16 offset, u8 *mask)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;

	status = rtw_efuse_get_offset_mask(hal_info->efuse, offset, mask);

	return status;
}


enum rtw_hal_status
rtw_hal_efuse_get_info(struct rtw_hal_com_t *hal_com,
		       enum rtw_efuse_info info_type,
		       void *value,
		       u8 size)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;
	struct hal_info_t *hal_info = hal_com->hal_priv;

	status = rtw_efuse_get_info(hal_info->efuse, info_type, value, size);
	

	return status;
}

#endif // if 0 NEO

/* API export to PHL : rtw_hal_get_efuse_info */
enum rtw_hal_status
rtw_hal_get_efuse_info(void *hal,
		       enum rtw_efuse_info info_type,
		       void *value,
		       u8 size)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;

	// NEO : TODO : mark off first
	//status = rtw_efuse_get_info(hal_info->efuse, info_type, value, size);

	return status;
}

#if 0 // NEO : TODO : mark off first

void rtw_hal_efuse_process(struct hal_info_t *hal_info, char *ic_name)
{
	rtw_efuse_process(hal_info->efuse, ic_name);
}

#endif // NEO

enum rtw_hal_status rtw_hal_efuse_init(struct rtw_phl_com_t *phl_com,
					struct hal_info_t *hal_info)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;
	struct rtw_hal_com_t *hal_com = hal_info->hal_com;

	status = rtw_efuse_init(phl_com, hal_com, &(hal_info->efuse));

	return status;
}

#if 0 // NEO : TODO : mark off first

void rtw_hal_efuse_deinit(struct rtw_phl_com_t *phl_com,
					struct hal_info_t *hal_info)
{
	struct rtw_hal_com_t *hal_com = hal_info->hal_com;

	rtw_efuse_deinit(hal_com, hal_info->efuse);
}


enum rtw_hal_status
rtw_hal_efuse_bt_shadow_load(struct hal_info_t *hal_info)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;

	status = rtw_efuse_bt_shadow_load(hal_info->efuse);

	return status;
}

enum rtw_hal_status
rtw_hal_efuse_bt_shadow_update(struct hal_info_t *hal_info)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_EFUSE_PG_FAIL;

	status = rtw_efuse_bt_shadow_update(hal_info->efuse);

	return status;
}

enum rtw_hal_status
rtw_hal_efuse_bt_shadow_read(struct hal_info_t *hal_info, u8 byte_count,
						  u16 offset, u32 *value)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;

	status = rtw_efuse_bt_shadow_read(hal_info->efuse, byte_count, offset, value);

	return status;
}

enum rtw_hal_status
rtw_hal_efuse_bt_shadow_write(struct hal_info_t *hal_info, u8 byte_count,
						   u16 offset, u32 value)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;

	status = rtw_efuse_bt_shadow_write(hal_info->efuse, byte_count, offset, value);
	return status;
}


enum rtw_hal_status
rtw_hal_efuse_bt_shadow2buf(struct hal_info_t *hal_info, u8 *pbuf, u16 buflen)
{
   enum rtw_hal_status status = RTW_HAL_STATUS_SUCCESS;

   status = rtw_efuse_bt_shadow2buf(hal_info->efuse, pbuf, buflen);

   return status;
}

enum rtw_hal_status rtw_hal_efuse_bt_file_map_load(
	struct hal_info_t *hal_info, char *file_path)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

	hal_status = rtw_efuse_bt_file_map_load(hal_info->efuse, file_path);

	return hal_status;
}


enum rtw_hal_status rtw_hal_efuse_bt_file_mask_load(
	struct hal_info_t *hal_info, char *file_path)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

	hal_status = rtw_efuse_bt_file_mask_load(hal_info->efuse, file_path);

	return hal_status;
}

/* usage = used percentage(1 Byte) + used bytes(2 Bytes) */
enum rtw_hal_status rtw_hal_efuse_bt_get_usage(struct hal_info_t *hal_info,
	u32 *usage)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;

	status = rtw_efuse_get_usage(hal_info->efuse, usage);

	return status;
}

enum rtw_hal_status rtw_hal_efuse_bt_get_logical_size(struct hal_info_t *hal_info,
	u32 *size)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;

	status = rtw_efuse_bt_get_logical_size(hal_info->efuse, size);

	return status;
}

enum rtw_hal_status rtw_hal_efuse_bt_get_size(struct hal_info_t *hal_info,
	u32 *size)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;

	status = rtw_efuse_bt_get_size(hal_info->efuse, size);

	return status;
}

enum rtw_hal_status rtw_hal_efuse_bt_get_avl(struct hal_info_t *hal_info,
	u32 *size)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;

	status = rtw_efuse_bt_get_avl(hal_info->efuse, size);

	return status;
}


enum rtw_hal_status rtw_hal_efuse_bt_get_offset_mask(struct hal_info_t *hal_info,
	u16 offset, u8 *mask)
{
	enum rtw_hal_status status = RTW_HAL_STATUS_FAILURE;

	status = rtw_efuse_bt_get_offset_mask(hal_info->efuse, offset, mask);

	return status;
}

enum rtw_hal_status rtw_hal_efuse_bt_read_hidden(
	struct hal_info_t *hal_info, u32 addr, u32 size, u8 *val)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

	hal_status = rtw_efuse_bt_read_hidden(hal_info->efuse, addr, size, val);

	return hal_status;
}

enum rtw_hal_status rtw_hal_efuse_bt_write_hidden(
	struct hal_info_t *hal_info, u32 addr, u8 val)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

	hal_status = rtw_efuse_bt_write_hidden(hal_info->efuse, addr, val);

	return hal_status;
}

#endif // if 0 NEO
