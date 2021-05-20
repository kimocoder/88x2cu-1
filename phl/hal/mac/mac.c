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

#include "mac.h"
#include "mac_reg.h"

#define CHIP_ID_HW_DEF_8852A	0x50
#define CHIP_ID_HW_DEF_8852B	0x51
#define CHIP_ID_HW_DEF_8852C	0x52
#define CHIP_ID_HW_DEF_8834A	0x53

#define SDIO_WAIT_CNT		50
#if MAC_SDIO_SUPPORT
static u8 r8_indir_cmd52_sdio(void *drv_adapter,
			      struct mac_ax_pltfm_cb *pltfm_cb, u32 adr);
#endif

#ifndef CONFIG_NEW_HALMAC_INTERFACE
static u8 chk_pltfm_cb(void *drv_adapter, enum mac_intf intf,
		       struct mac_pltfm_cb *pltfm_cb)
{
	if (!pltfm_cb)
		return MACSUCCESS;

	if (!pltfm_cb->msg_print)
		return MACSUCCESS;

#if MAC_SDIO_SUPPORT
	if (!pltfm_cb->sdio_cmd52_r8 || !pltfm_cb->sdio_cmd53_r8 ||
	    !pltfm_cb->sdio_cmd53_r16 || !pltfm_cb->sdio_cmd53_r32 ||
	    !pltfm_cb->sdio_cmd53_rn || !pltfm_cb->sdio_cmd52_w8 ||
	    !pltfm_cb->sdio_cmd53_w8 || !pltfm_cb->sdio_cmd53_w16 ||
	    !pltfm_cb->sdio_cmd53_w32 || !pltfm_cb->sdio_cmd53_wn ||
	    !pltfm_cb->sdio_cmd52_cia_r8) {
		pltfm_cb->msg_print(drv_adapter, "[ERR]CB-SDIO\n");
		return MACSUCCESS;
	}
#endif

#if (MAC_USB_SUPPORT || MAC_PCIE_SUPPORT)
	if (!pltfm_cb->reg_r8 || !pltfm_cb->reg_r16 ||
	    !pltfm_cb->reg_r32 || !pltfm_cb->reg_w8 ||
	    !pltfm_cb->reg_w16 || !pltfm_cb->reg_w32) {
		pltfm_cb->msg_print(drv_adapter, "[ERR]CB-USB or PCIE\n");
		return MACSUCCESS;
	}
#endif
	if (!pltfm_cb->rtl_free || !pltfm_cb->rtl_malloc ||
	    !pltfm_cb->rtl_memcpy || !pltfm_cb->rtl_memset ||
	    !pltfm_cb->rtl_delay_us || !pltfm_cb->rtl_delay_ms ||
	    !pltfm_cb->rtl_mutex_init || !pltfm_cb->rtl_mutex_deinit ||
	    !pltfm_cb->rtl_mutex_lock || !pltfm_cb->rtl_mutex_unlock ||
	    !pltfm_cb->event_notify) {
		pltfm_cb->msg_print(drv_adapter, "[ERR]CB-OS\n");
		return MACSUCCESS;
	}

	return MACPFCB;
}
#endif /*CONFIG_NEW_HALMAC_INTERFACE*/

static u8 chk_pltfm_endian(void)
{
	u32 num = 1;
	u8 *num_ptr = (u8 *)&num;

	if (*num_ptr != PLATFOM_IS_LITTLE_ENDIAN)
		return MACSUCCESS;

	return MACPFCB;
}

#ifdef CONFIG_NEW_HALMAC_INTERFACE
#if MAC_AX_SDIO_SUPPORT
static u8 r8_indir_cmd52_sdio(void *drv_adapter,
			      struct mac_ax_pltfm_cb *pltfm_cb, u32 adr)
{
	u8 tmp;
	u32 cnt;

	PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_ADDR, (u8)adr);
	PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_ADDR + 1, (u8)(adr >> 8));
	PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_ADDR + 2, (u8)(adr >> 16));
	PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_ADDR + 3,
			    (u8)((adr | B_AX_INDIRECT_RDY) >> 24));
	PLTFM_SDIO_CMD52_W8(R_AX_SDIO_INDIRECT_CTRL, (u8)B_AX_INDIRECT_REG_R);

	cnt = SDIO_WAIT_CNT;
	do {
		tmp = PLTFM_SDIO_CMD52_R8(R_AX_SDIO_INDIRECT_ADDR + 3);
		cnt--;
	} while (((tmp & BIT(7)) == 0) && (cnt > 0));

	if (((tmp & BIT(7)) == 0) && cnt == 0)
		PLTFM_MSG_ERR("[ERR]sdio indirect CMD52 read\n");

	return PLTFM_SDIO_CMD52_R8(R_AX_SDIO_INDIRECT_DATA);
}
#endif
static u32 get_chip_info(struct mac_ax_adapter *adapter,
			 struct mac_ax_pltfm_cb *pltfm_cb,
			 enum mac_ax_intf intf, u8 *id, u8 *cv)
{
	u32 cv_temp;

	if (!cv || !id)
		return MACNPTR;

	switch (intf) {
	case MAC_INTF_SDIO:
		break;
	case MAC_INTF_USB:
	case MAC_INTF_PCIE:
		*cv = pltfm_cb->reg_r8(drv_adapter, REG_SYS_CFG1 + 1) >> 4;
		break;
	default:
		return MACINTF;
	}

	*id = MAC_CHIP_ID;

	return MACSUCCESS;
}

u32 mac_ax_ops_init_v1(void *phl_adapter, void *drv_adapter,
		       enum rtw_chip_id chip_id,
		       enum rtw_hci_type hci,
		       struct mac_ax_adapter **mac_adapter,
		       struct mac_ax_ops **mac_ops)
{
	u32 ret;
	u8 cv;
	struct mac_ax_adapter *adapter;
	enum mac_ax_intf intf = MAC_AX_INTF_INVALID;

	if (!chk_pltfm_endian())
		return MACPFED;

	ret = 0;

	if (hci == RTW_HCI_PCIE)
		intf = MAC_AX_INTF_PCIE;
	else if (hci == RTW_HCI_USB)
		intf = MAC_AX_INTF_USB;
	else if (hci == RTW_HCI_SDIO)
		intf = MAC_AX_INTF_SDIO;

	ret = get_chip_info(drv_adapter, NULL, intf, &chip_id, &cv);
	if (ret)
		return ret;

	adapter = get_mac_ax_adapter(intf, chip_id, cv, phl_adapter,
				     drv_adapter, NULL);
	if (!adapter) {
		PLTFM_MSG_PRINT("[ERR]Get MAC adapter\n");
		return MACADAPTER;
	}
	PLTFM_MSG_ALWAYS("MAC_AX_MAJOR_VER = %d\n"
			    "MAC_AX_PROTOTYPE_VER = %d\n"
			    "MAC_AX_SUB_VER = %d\n"
			    "MAC_AX_SUB_INDEX = %d\n",
			    MAC_AX_MAJOR_VER, MAC_AX_PROTOTYPE_VER,
			    MAC_AX_SUB_VER, MAC_AX_SUB_INDEX);

	*mac_adapter = adapter;
	*mac_ops = adapter->ops;

	return MACSUCCESS;
}

#else


#if MAC_SDIO_SUPPORT
static u8 r8_indir_cmd52_sdio(void *drv_adapter,
			      struct mac_ax_pltfm_cb *pltfm_cb, u32 adr)
{
	u8 tmp;
	u32 cnt;

	pltfm_cb->sdio_cmd52_w8(drv_adapter, R_AX_SDIO_INDIRECT_ADDR,
				(u8)adr);
	pltfm_cb->sdio_cmd52_w8(drv_adapter, R_AX_SDIO_INDIRECT_ADDR + 1,
				(u8)(adr >> 8));
	pltfm_cb->sdio_cmd52_w8(drv_adapter, R_AX_SDIO_INDIRECT_ADDR + 2,
				(u8)(adr >> 16));
	pltfm_cb->sdio_cmd52_w8(drv_adapter, R_AX_SDIO_INDIRECT_ADDR + 3,
				(u8)((adr | B_AX_INDIRECT_RDY) >> 24));
	pltfm_cb->sdio_cmd52_w8(drv_adapter, R_AX_SDIO_INDIRECT_CTRL,
				(u8)B_AX_INDIRECT_REG_R);

	cnt = SDIO_WAIT_CNT;
	do {
		tmp = pltfm_cb->sdio_cmd52_r8(drv_adapter,
					      R_AX_SDIO_INDIRECT_ADDR + 3);
		cnt--;
	} while (((tmp & BIT(7)) == 0) && (cnt > 0));

	if (((tmp & BIT(7)) == 0) && cnt == 0)
		pltfm_cb->msg_print(drv_adapter,
				    "[ERR]sdio indirect CMD52 read\n");

	return pltfm_cb->sdio_cmd52_r8(drv_adapter, R_AX_SDIO_INDIRECT_DATA);
}
#endif

static u32 get_chip_info(struct mac_adapter *drv_adapter,
			 struct mac_pltfm_cb *pltfm_cb,
			 enum mac_intf intf, u8 *id, u8 *cv)
{
	u32 cv_temp;

	if (!cv || !id)
		return MACNPTR;

	switch (intf) {
	case MAC_INTF_SDIO:
		break;
	case MAC_INTF_USB:
	case MAC_INTF_PCIE:
		*cv = pltfm_cb->reg_r8(drv_adapter, REG_SYS_CFG1 + 1) >> 4;
		break;
	default:
		return MACINTF;
	}

	*id = MAC_CHIP_ID_8822C;

	return MACSUCCESS;
}

u32 mac_ops_init(void *drv_adapter, struct mac_pltfm_cb *pltfm_cb,
		    enum mac_intf intf,
		    struct mac_adapter **mac_adapter,
		    struct mac_ops **mac_ops)
{
	u32 ret;
	u8 chip_id = 0;
	u8 cv = 0;
	struct mac_adapter *adapter;

	if (!chk_pltfm_cb(drv_adapter, intf, pltfm_cb))
		return MACPFCB;

	if (!chk_pltfm_endian())
		return MACPFED;

	pltfm_cb->msg_print(drv_adapter,
			    "MAC_MAJOR_VER = %d\n"
			    "MAC_PROTOTYPE_VER = %d\n"
			    "MAC_SUB_VER = %d\n"
			    "MAC_SUB_INDEX = %d\n",
			    MAC_MAJOR_VER, MAC_PROTOTYPE_VER,
			    MAC_SUB_VER, MAC_SUB_INDEX);

	ret = get_chip_info(drv_adapter, pltfm_cb, intf, &chip_id, &cv);
	if (ret)
		return ret;

	adapter = get_mac_adapter(intf, chip_id, cv, drv_adapter,
				     pltfm_cb);
	if (!adapter) {
		pltfm_cb->msg_print(drv_adapter, "[ERR]Get MAC adapter\n");
		return MACADAPTER;
	}

	*mac_adapter = adapter;
	*mac_ops = adapter->ops;

#if MAC_AX_FEATURE_HV
	adapter->hv_ops = get_hv_ax_ops(adapter);
#endif

#if MAC_PHL_H2C
#else
	ret = h2cb_init(adapter);
	if (ret)
		return ret;
#endif

	ret = role_tbl_init(adapter);
	if (ret)
		return ret;

	ret = sec_info_tbl_init(adapter);
	if (ret)
		return ret;

	ret = efuse_tbl_init(adapter);
	if (ret)
		return ret;
#if MAC_AX_SDIO_SUPPORT
	ret = sdio_tbl_init(adapter);
	if (ret)
		return ret;
#endif
	return MACSUCCESS;
}
#endif /*CONFIG_NEW_HALMAC_INTERFACE*/

#if MAC_PHL_H2C
u32 mac_phl_init(void *phl_adapter, struct mac_adapter *mac_adapter)
{
	struct mac_adapter *adapter = mac_adapter;

	adapter->phl_adapter = phl_adapter;

	return MACSUCCESS;
}
#endif

u32 mac_ops_exit(struct mac_adapter *adapter)
{
	struct mac_efuse_param *efuse_param = &adapter->efuse_param;
	u32 ret;

	ret = free_sec_info_tbl(adapter);
	if (ret) {
		PLTFM_MSG_ERR("[ERR]sec table exit\n");
		return ret;
	}

	ret = role_tbl_exit(adapter);
	if (ret) {
		PLTFM_MSG_ERR("[ERR]role table exit\n");
		return ret;
	}

	ret = efuse_tbl_exit(adapter);
	if (ret) {
		PLTFM_MSG_ERR("[ERR]efuse table exit\n");
		return ret;
	}
#if MAC_AX_SDIO_SUPPORT
	ret = sdio_tbl_exit(adapter);
	if (ret) {
		PLTFM_MSG_ERR("[ERR]efuse table exit\n");
		return ret;
	}
#endif

#if 0 //NEO
	ret = h2cb_exit(adapter);
	if (ret) {
		PLTFM_MSG_ERR("[ERR]h2c buffer exit\n");
		return ret;
	}
#endif //NEO

	if (efuse_param->efuse_map) {
		PLTFM_FREE(efuse_param->efuse_map,
			   adapter->hw_info->efuse_size);
		efuse_param->efuse_map = (u8 *)NULL;
	}

	if (efuse_param->bt_efuse_map) {
		PLTFM_FREE(efuse_param->bt_efuse_map,
			   adapter->hw_info->bt_efuse_size);
		efuse_param->bt_efuse_map = (u8 *)NULL;
	}

	if (efuse_param->log_efuse_map) {
		PLTFM_FREE(efuse_param->log_efuse_map,
			   adapter->hw_info->log_efuse_size);
		efuse_param->log_efuse_map = (u8 *)NULL;
	}

	if (efuse_param->bt_log_efuse_map) {
		PLTFM_FREE(efuse_param->bt_log_efuse_map,
			   adapter->hw_info->bt_log_efuse_size);
		efuse_param->bt_log_efuse_map = (u8 *)NULL;
	}

	PLTFM_FREE(adapter->hw_info, sizeof(struct mac_hw_info));

	PLTFM_FREE(adapter, sizeof(struct mac_adapter));
	return MACSUCCESS;
}


