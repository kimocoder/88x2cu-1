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


#include "init.h"
//#include "security_cam.h"
//#include "hw.h"

#ifdef CONFIG_NEW_HALMAC_INTERFACE
struct mac_ax_adapter *get_mac_ax_adapter(enum mac_ax_intf intf,
					  u8 chip_id, u8 chip_cut,
					  void *phl_adapter, void *drv_adapter,
					  struct mac_ax_pltfm_cb *pltfm_cb)
{
	struct mac_ax_adapter *adapter = NULL;

	switch (chip_id) {
#if MAC_AX_8852A_SUPPORT
	case MAC_AX_CHIP_ID_8852A:
		adapter = get_mac_8852a_adapter(intf, chip_cut, phl_adapter,
						drv_adapter, pltfm_cb);
		break;
#endif
	default:
		return NULL;
	}

	return adapter;
}
#else
struct mac_adapter *get_mac_adapter(enum mac_intf intf,
					  u8 chip_id, u8 chip_cut,
					  void *drv_adapter,
					  struct mac_pltfm_cb *pltfm_cb)
{
	struct mac_adapter *adapter = NULL;

	switch (chip_id) {
#if MAC_AX_8852A_SUPPORT
	case MAC_CHIP_ID_8852A:
		adapter = get_mac_8852a_adapter(intf, chip_cut, drv_adapter,
						pltfm_cb);
		break;
#endif
#if MAC_AX_8852B_SUPPORT
	case MAC_CHIP_ID_8852B:
		adapter = get_mac_8852b_adapter(intf, chip_cut, drv_adapter,
						pltfm_cb);
		break;
#endif
#if MAC_AC_8822C_SUPPORT
	case MAC_CHIP_ID_8822C:
		adapter = get_mac_8822c_adapter(intf, chip_cut, drv_adapter,
						pltfm_cb);
		break;
#endif

	default:
		return NULL;
	}

	return adapter;
}
#endif

#if 0 // NEO
u32 hci_func_en(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;
	u32 ret = MACSUCCESS;

	val32 = MAC_REG_R32(R_AX_HCI_FUNC_EN) |
		B_AX_HCI_TXDMA_EN | B_AX_HCI_RXDMA_EN;
	MAC_REG_W32(R_AX_HCI_FUNC_EN, val32);

	return ret;
}

u32 dmac_func_en(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
#if MAC_AX_PCIE_SUPPORT
	//enum mac_ax_intf intf = adapter->hw_info->intf;
#endif
	u32 val32;
	u32 ret = 0;

	val32 = (B_AX_MAC_FUNC_EN | B_AX_DMAC_FUNC_EN | B_AX_MAC_SEC_EN |
		 B_AX_DISPATCHER_EN | B_AX_DLE_CPUIO_EN | B_AX_PKT_IN_EN |
		 B_AX_DMAC_TBL_EN | B_AX_PKT_BUF_EN | B_AX_STA_SCH_EN |
		 B_AX_TXPKT_CTRL_EN | B_AX_WD_RLS_EN | B_AX_MPDU_PROC_EN);
	MAC_REG_W32(R_AX_DMAC_FUNC_EN, val32);

	val32 = (B_AX_MAC_SEC_CLK_EN | B_AX_DISPATCHER_CLK_EN |
		 B_AX_DLE_CPUIO_CLK_EN | B_AX_PKT_IN_CLK_EN |
		 B_AX_STA_SCH_CLK_EN | B_AX_TXPKT_CTRL_CLK_EN |
		 B_AX_WD_RLS_CLK_EN);
	MAC_REG_W32(R_AX_DMAC_CLK_EN, val32);

	return ret;
}

u32 cmac_func_en(struct mac_ax_adapter *adapter, u8 band, u8 en)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32_func_en = 0;
	u32 val32_ck_en = 0;
	u32 val32_c1pc_en = 0;
	u32 addrl_func_en[] = {R_AX_CMAC_FUNC_EN, R_AX_CMAC_FUNC_EN_C1};
	u32 addrl_ck_en[] = {R_AX_CK_EN, R_AX_CK_EN_C1};

	val32_func_en = B_AX_CMAC_EN | B_AX_CMAC_TXEN | B_AX_CMAC_RXEN |
			B_AX_PHYINTF_EN | B_AX_CMAC_DMA_EN | B_AX_PTCLTOP_EN |
			B_AX_SCHEDULER_EN | B_AX_TMAC_EN | B_AX_RMAC_EN;
	val32_ck_en = B_AX_CMAC_CKEN | B_AX_PHYINTF_CKEN | B_AX_CMAC_DMA_CKEN |
		      B_AX_PTCLTOP_CKEN | B_AX_SCHEDULER_CKEN | B_AX_TMAC_CKEN |
		      B_AX_RMAC_CKEN;
	val32_c1pc_en = B_AX_R_SYM_WLCMAC1_PC_EN |
			B_AX_R_SYM_WLCMAC1_P1_PC_EN |
			B_AX_R_SYM_WLCMAC1_P2_PC_EN |
			B_AX_R_SYM_WLCMAC1_P3_PC_EN |
			B_AX_R_SYM_WLCMAC1_P4_PC_EN;

	if (band >= MAC_AX_BAND_NUM) {
		PLTFM_MSG_ERR("band %d invalid\n", band);
		return MACFUNCINPUT;
	}

	if (en) {
		if (band == MAC_AX_BAND_1) {
			MAC_REG_W32(R_AX_AFE_CTRL1,
				    MAC_REG_R32(R_AX_AFE_CTRL1) |
				    val32_c1pc_en);
			MAC_REG_W32(R_AX_SYS_ISO_CTRL_EXTEND,
				    MAC_REG_R32(R_AX_SYS_ISO_CTRL_EXTEND) &
				    ~B_AX_R_SYM_ISO_CMAC12PP);
			MAC_REG_W32(R_AX_SYS_ISO_CTRL_EXTEND,
				    MAC_REG_R32(R_AX_SYS_ISO_CTRL_EXTEND) |
				    B_AX_CMAC1_FEN);
		}
		MAC_REG_W32(addrl_ck_en[band],
			    MAC_REG_R32(addrl_ck_en[band]) | val32_ck_en);
		MAC_REG_W32(addrl_func_en[band],
			    MAC_REG_R32(addrl_func_en[band]) | val32_func_en);
	} else {
		MAC_REG_W32(addrl_func_en[band],
			    MAC_REG_R32(addrl_func_en[band]) & ~val32_func_en);
		MAC_REG_W32(addrl_ck_en[band],
			    MAC_REG_R32(addrl_ck_en[band]) & ~val32_ck_en);
		if (band == MAC_AX_BAND_1) {
			MAC_REG_W32(R_AX_SYS_ISO_CTRL_EXTEND,
				    MAC_REG_R32(R_AX_SYS_ISO_CTRL_EXTEND) &
				    ~B_AX_CMAC1_FEN);
			MAC_REG_W32(R_AX_SYS_ISO_CTRL_EXTEND,
				    MAC_REG_R32(R_AX_SYS_ISO_CTRL_EXTEND) |
				    B_AX_R_SYM_ISO_CMAC12PP);
			MAC_REG_W32(R_AX_AFE_CTRL1,
				    MAC_REG_R32(R_AX_AFE_CTRL1) &
				    ~val32_c1pc_en);
		}
	}
//Reset BACAM chunchu
//RMAC reg0x3C[1:0] = 2'b10
	return MACSUCCESS;
}

u32 chip_func_en(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	u32 val32;
	u32 ret = MACSUCCESS;

	if (is_chip_id(adapter, MAC_AX_CHIP_ID_8852A) ||
	    is_chip_id(adapter, MAC_AX_CHIP_ID_8852B)) {
		/* patch for OCP */
		val32 = MAC_REG_R32(R_AX_SPSLDO_ON_CTRL0);
		val32 |= SET_WOR2(B_AX_OCP_L1_MSK, B_AX_OCP_L1_SH,
				  B_AX_OCP_L1_MSK);
		MAC_REG_W32(R_AX_SPSLDO_ON_CTRL0, val32);
	}

	return ret;
}

u32 mac_sys_init(struct mac_ax_adapter *adapter)
{
	u32 ret;

	ret = dmac_func_en(adapter);
	if (ret)
		return ret;

	ret = cmac_func_en(adapter, MAC_AX_BAND_0, MAC_AX_FUNC_EN);
	if (ret)
		return ret;

	ret = chip_func_en(adapter);
	if (ret)
		return ret;

	return ret;
}

u32 mac_hal_init(struct mac_ax_adapter *adapter,
		 struct mac_ax_trx_info *trx_info,
		 struct mac_ax_fwdl_info *fwdl_info,
		 struct mac_ax_intf_info *intf_info)
{
	struct mac_ax_ops *ops = adapter_to_mac_ops(adapter);
	struct mac_ax_intf_ops *intf_ops = adapter_to_intf_ops(adapter);
	struct mac_ax_hw_info *hw_info = adapter->hw_info;
	u32 ret;
	u32 rom_addr;

	PLTFM_MUTEX_INIT(&adapter->fw_info.seq_lock);
	PLTFM_MUTEX_INIT(&adapter->hw_info->ind_access_lock);

	ret = ops->pwr_switch(adapter, 1);
	if (ret == MACALRDYON) {
		ret = ops->pwr_switch(adapter, 0);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]pwr_switch 0 fail %d\n", ret);
			return ret;
		}
		ret = ops->pwr_switch(adapter, 1);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]pwr_switch 0->1 fail %d\n", ret);
			return ret;
		}
	}
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]pwr_switch 1 fail %d\n", ret);
		return ret;
	}

	ret = hci_func_en(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]hci_func_en %d\n", ret);
		return ret;
	}

	ret = intf_ops->intf_pre_init(adapter, intf_info);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]intf_pre_init %d\n", ret);
		return ret;
	}

	if (fwdl_info->fw_en) {
		if (fwdl_info->dlrom_en || fwdl_info->dlram_en) {
			ret = fwdl_pre_init(adapter, trx_info->qta_mode);
			if (ret != MACSUCCESS) {
				PLTFM_MSG_ERR("[ERR]fwdl_pre_init %d\n", ret);
				return ret;
			}
		}
		if (fwdl_info->dlrom_en) {
			switch (hw_info->chip_id) {
			case MAC_AX_CHIP_ID_8852A:
				rom_addr = RTL8852A_ROM_ADDR;
				break;
			case MAC_AX_CHIP_ID_8852B:
				rom_addr = RTL8852B_ROM_ADDR;
				break;
			default:
				PLTFM_MSG_ERR("[ERR]chip id\n");
				return MACNOITEM;
			}
			ret = ops->romdl(adapter,
					 fwdl_info->rom_buff,
					 rom_addr,
					 fwdl_info->rom_size);
			if (ret != MACSUCCESS) {
				PLTFM_MSG_ERR("[ERR]romdl %d\n", ret);
				return ret;
			}
		}

		if (fwdl_info->dlram_en) {
			if (fwdl_info->fw_from_hdr) {
				ret = ops->enable_fw(adapter,
						     fwdl_info->fw_cat);
				if (ret != MACSUCCESS) {
					PLTFM_MSG_ERR("[ERR]enable_fw %d\n",
						      ret);
					return ret;
				}
			} else {
				ret = ops->enable_cpu(adapter, 0,
						      fwdl_info->dlram_en);
				if (ret != MACSUCCESS) {
					PLTFM_MSG_ERR("[ERR]enable_cpu %d\n",
						      ret);
					return ret;
				}

				ret = ops->fwdl(adapter,
						fwdl_info->ram_buff,
						fwdl_info->ram_size);
				if (ret != MACSUCCESS) {
					PLTFM_MSG_ERR("[ERR]fwdl %d\n", ret);
					return ret;
				}
			}
		}
	}

	ret = set_enable_bb_rf(adapter, MAC_AX_FUNC_EN);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]set_enable_bb_rf %d\n", ret);
		return ret;
	}

	ret = ops->sys_init(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]sys_init %d\n", ret);
		return ret;
	}

	ret = ops->trx_init(adapter, trx_info);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]trx_init %d\n", ret);
		return ret;
	}

	ret = intf_ops->intf_init(adapter, intf_info);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]intf_init %d\n", ret);
		return ret;
	}

	return ret;
}

u32 mac_hal_deinit(struct mac_ax_adapter *adapter)
{
	struct mac_ax_ops *ops = adapter_to_mac_ops(adapter);
	struct mac_ax_intf_ops *intf_ops = adapter_to_intf_ops(adapter);
	u32 ret;

	ret = free_sec_info_tbl(adapter);
	if (ret) {
		PLTFM_MSG_ERR("[ERR]remove security info tbl\n");
		return ret;
	}

	ret = mac_remove_role_by_band(adapter, MAC_AX_BAND_0, 1);
	if (ret) {
		PLTFM_MSG_ERR("[ERR]remove band0 role fail\n");
		return ret;
	}

	ret = mac_remove_role_by_band(adapter, MAC_AX_BAND_1, 1);
	if (ret) {
		PLTFM_MSG_ERR("[ERR]remove band0 role fail\n");
		return ret;
	}

	ret = intf_ops->intf_deinit(adapter, NULL);
	if (ret) {
		PLTFM_MSG_ERR("[ERR]intf deinit\n");
		return ret;
	}

	ret = ops->pwr_switch(adapter, 0);
	if (ret) {
		PLTFM_MSG_ERR("[ERR]pwr switch off\n");
		return ret;
	}

	PLTFM_MUTEX_DEINIT(&adapter->fw_info.seq_lock);
	PLTFM_MUTEX_DEINIT(&adapter->hw_info->ind_access_lock);

	return ret;
}

u32 mac_hal_fast_init(struct mac_ax_adapter *adapter,
		      struct mac_ax_trx_info *trx_info,
		      struct mac_ax_fwdl_info *fwdl_info,
		      struct mac_ax_intf_info *intf_info)
{
	struct mac_ax_ops *ops = adapter_to_mac_ops(adapter);
	struct mac_ax_intf_ops *intf_ops = adapter_to_intf_ops(adapter);
	struct mac_ax_hw_info *hw_info = adapter->hw_info;
	u32 rom_addr;
	u32 ret;

	PLTFM_MUTEX_INIT(&adapter->fw_info.seq_lock);

	ret = ops->pwr_switch(adapter, 1);
	if (ret == MACALRDYON) {
		ret = ops->pwr_switch(adapter, 0);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]pwr_switch 0 fail %d\n", ret);
			return ret;
		}
		ret = ops->pwr_switch(adapter, 1);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]pwr_switch 0->1 fail %d\n", ret);
			return ret;
		}
	}
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]pwr_switch 1 fail %d\n", ret);
		return ret;
	}

	ret = hci_func_en(adapter);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]hci_func_en %d\n", ret);
		return ret;
	}

	ret = intf_ops->intf_pre_init(adapter, intf_info);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]intf_pre_init %d\n", ret);
		return ret;
	}

	if (fwdl_info->fw_en) {
		if (fwdl_info->dlrom_en || fwdl_info->dlram_en) {
			ret = fwdl_pre_init(adapter, trx_info->qta_mode);
			if (ret != MACSUCCESS) {
				PLTFM_MSG_ERR("[ERR]fwdl_pre_init %d\n", ret);
				return ret;
			}
		}
		if (fwdl_info->dlrom_en) {
			switch (hw_info->chip_id) {
			case MAC_AX_CHIP_ID_8852A:
				rom_addr = RTL8852A_ROM_ADDR;
				break;
			case MAC_AX_CHIP_ID_8852B:
				rom_addr = RTL8852B_ROM_ADDR;
				break;
			default:
				PLTFM_MSG_ERR("[ERR]chip id\n");
				return MACNOITEM;
			}
			ret = ops->romdl(adapter,
					 fwdl_info->rom_buff,
					 rom_addr,
					 fwdl_info->rom_size);
			if (ret != MACSUCCESS) {
				PLTFM_MSG_ERR("[ERR]romdl %d\n", ret);
				return ret;
			}
		}

		if (fwdl_info->dlram_en) {
			if (fwdl_info->fw_from_hdr) {
				ret = ops->enable_fw(adapter,
						     fwdl_info->fw_cat);
				if (ret != MACSUCCESS) {
					PLTFM_MSG_ERR("[ERR]enable_fw %d\n",
						      ret);
					return ret;
				}
			} else {
				ret = ops->enable_cpu(adapter, 0,
						      fwdl_info->dlram_en);
				if (ret != MACSUCCESS) {
					PLTFM_MSG_ERR("[ERR]enable_cpu %d\n",
						      ret);
					return ret;
				}

				ret = ops->fwdl(adapter,
						fwdl_info->ram_buff,
						fwdl_info->ram_size);
				if (ret != MACSUCCESS) {
					PLTFM_MSG_ERR("[ERR]fwdl %d\n", ret);
					return ret;
				}
			}
		}
	}

	return ret;
}

u32 mac_ax_init_state(struct mac_ax_adapter *adapter)
{
	struct mac_ax_state_mach sm = MAC_AX_DFLT_SM;

	adapter->sm = sm;
	adapter->fw_info.h2c_seq = 0;
	adapter->fw_info.rec_seq = 0;

	return MACSUCCESS;
}

#endif // if 0
