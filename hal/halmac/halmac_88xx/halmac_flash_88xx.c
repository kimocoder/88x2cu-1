/******************************************************************************
 *
 * Copyright(c) 2017 - 2019 Realtek Corporation. All rights reserved.
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

#include "halmac_flash_88xx.h"
#include "halmac_88xx_cfg.h"
#include "halmac_common_88xx.h"

#if HALMAC_88XX_SUPPORT

/**
 * erase_flash_88xx() -erase flash data
 * @adapter : the adapter of halmac
 * @erase_cmd : erase command
 * @addr : flash start address where fw should be erased
 * Author : Pablo Chiu
 * Return : enum halmac_ret_status
 * More details of status code can be found in prototype document
 */
enum halmac_ret_status
erase_flash_88xx(struct halmac_adapter *adapter, u8 erase_cmd, u32 addr)
{
	enum halmac_ret_status status;
	struct halmac_h2c_header_info hdr_info;
	struct halmac_api *api = (struct halmac_api *)adapter->halmac_api;
	u8 value8;
	u8 h2c_buf[H2C_PKT_SIZE_88XX] = {0};
	u16 seq_num = 0;
	u32 cnt;

	/* Construct H2C Content */
	DOWNLOAD_FLASH_SET_SPI_CMD(h2c_buf, erase_cmd);
	DOWNLOAD_FLASH_SET_LOCATION(h2c_buf, 0);
	DOWNLOAD_FLASH_SET_START_ADDR(h2c_buf, addr);
	DOWNLOAD_FLASH_SET_SIZE(h2c_buf, 0);

	value8 = HALMAC_REG_R8(REG_MCUTST_I);
	value8 |= BIT(0);
	HALMAC_REG_W8(REG_MCUTST_I, value8);

	/* Fill in H2C Header */
	hdr_info.sub_cmd_id = SUB_CMD_ID_DOWNLOAD_FLASH;
	hdr_info.content_size = 16;
	hdr_info.ack = 1;
	set_h2c_pkt_hdr_88xx(adapter, h2c_buf, &hdr_info, &seq_num);

	/* Send H2C Cmd Packet */
	status = send_h2c_pkt_88xx(adapter, h2c_buf);

	if (status != HALMAC_RET_SUCCESS)
		PLTFM_MSG_ERR("[ERR]send h2c!!\n");

	cnt = 5000;
	while (((HALMAC_REG_R8(REG_MCUTST_I)) & BIT(0)) != 0 && cnt != 0) {
		PLTFM_DELAY_US(1000);
		cnt--;
	}

	if (cnt == 0)
		return HALMAC_RET_FAIL;
	else
		return HALMAC_RET_SUCCESS;
}

#endif /* HALMAC_88XX_SUPPORT */
