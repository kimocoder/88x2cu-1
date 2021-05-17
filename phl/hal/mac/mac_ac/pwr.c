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
#include "pwr.h"
#include "../mac_reg_ac.h"
#if 0 //NEO
#include "coex.h"

static void restore_flr_lps(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);

	MAC_REG_W32(R_AX_WCPU_FW_CTRL, 0);

	MAC_REG_W32(R_AX_AFE_CTRL1, MAC_REG_R32(R_AX_AFE_CTRL1) &
		    ~B_AX_CMAC_CLK_SEL);

	MAC_REG_W32(R_AX_GPIO0_15_EECS_EESK_LED1_PULL_LOW_EN,
		    MAC_REG_R32(R_AX_GPIO0_15_EECS_EESK_LED1_PULL_LOW_EN) &
		    ~(B_AX_GPIO8_PULL_LOW_EN | B_AX_LED1_PULL_LOW_EN));
}

static void clr_aon_int(struct mac_ax_adapter *adapter)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32;

	val32 = MAC_REG_R32(R_AX_FWS0IMR);
	val32 &= ~B_AX_FS_GPIOA_INT_EN;
	MAC_REG_W32(R_AX_FWS0IMR, val32);

	val32 = MAC_REG_R32(R_AX_FWS0ISR);
	val32 |= B_AX_FS_GPIOA_INT;
	MAC_REG_W32(R_AX_FWS0ISR, val32);
}

static u32 _patch_aon_int_leave_lps(struct mac_ax_adapter *adapter)
{
	struct mac_ax_ops *mac_ops = adapter_to_mac_ops(adapter);
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val32, cnt, ret = MACSUCCESS;

	restore_flr_lps(adapter);

	val32 = MAC_REG_R32(R_AX_FWS0IMR);
	val32 |= B_AX_FS_GPIOA_INT_EN;
	MAC_REG_W32(R_AX_FWS0IMR, val32);

	ret = mac_ops->set_gpio_func(adapter, RTW_MAC_GPIO_SW_IO,
				     LPS_LEAVE_GPIO);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]set gpio fail %d\n", ret);
		return ret;
	}

	val32 = MAC_REG_R32(R_AX_GPIO0_15_EECS_EESK_LED1_PULL_HIGH_EN);
	val32 |= B_AX_GPIO10_PULL_HIGH_EN;
	MAC_REG_W32(R_AX_GPIO0_15_EECS_EESK_LED1_PULL_HIGH_EN, val32);

	val32 = MAC_REG_R32(R_AX_GPIO_EXT_CTRL);
	val32 |= (BIT10 | BIT18 | BIT26);
	MAC_REG_W32(R_AX_GPIO_EXT_CTRL, val32);
	val32 &= ~BIT10;
	MAC_REG_W32(R_AX_GPIO_EXT_CTRL, val32);
	val32 |= BIT10;
	MAC_REG_W32(R_AX_GPIO_EXT_CTRL, val32);

	cnt = LPS_POLL_CNT;
	while (cnt && (GET_FIELD(MAC_REG_R32(R_AX_IC_PWR_STATE), B_AX_WLMAC_PWR_STE) ==
		       MAC_AX_MAC_LPS)) {
		cnt--;
		PLTFM_DELAY_US(LPS_POLL_DLY_US);
	}

	if (!cnt) {
		PLTFM_MSG_ERR("[ERR]Polling MAC state timeout! 0x3F0 = %X\n",
			      MAC_REG_R32(R_AX_IC_PWR_STATE));
		return MACPOLLTO;
	}

	return MACSUCCESS;
}
#endif //NEO

static u32 pwr_cmd_poll(struct mac_adapter *adapter, struct mac_pwr_cfg *seq)
{
	u8 val = 0;
	u32 addr;
	u32 cnt;
	struct mac_intf_ops *ops = adapter_to_intf_ops(adapter);

	cnt = PWR_POLL_CNT;
	addr = seq->addr;

	while (cnt--) {
		val = MAC_REG_R8(addr);
		val &= seq->msk;
		if (val == (seq->val & seq->msk))
			return MACSUCCESS;
		PLTFM_DELAY_US(PWR_POLL_DLY_US);
	}

	PLTFM_MSG_ERR("[ERR] Polling timeout\n");
	PLTFM_MSG_ERR("[ERR] addr: %X, %X\n", addr, seq->addr);
	PLTFM_MSG_ERR("[ERR] val: %X, %X\n", val, seq->val);

	return MACPOLLTO;
}


static u32 sub_pwr_seq_start(struct mac_adapter *adapter,
			     u8 cv_msk, u8 intf_msk, struct mac_pwr_cfg *seq)
{
	u8 val;
	u32 addr;
	u32 ret;
	struct mac_intf_ops *ops = adapter_to_intf_ops(adapter);

	while (seq->cmd != PWR_CMD_END) {
		if (!(seq->intf_msk & intf_msk) || !(seq->cut_msk & cv_msk))
			goto next_seq;

		switch (seq->cmd) {
		case PWR_CMD_WRITE:
			addr = seq->addr;
			val = MAC_REG_R8(addr);
			val &= ~(seq->msk);
			val |= (seq->val & seq->msk);
			MAC_REG_W8(addr, val);
			break;
		case PWR_CMD_POLL:
			ret = pwr_cmd_poll(adapter, seq);
			if (ret != MACSUCCESS) {
				PLTFM_MSG_ERR("[ERR]pwr cmd poll %d\n", ret);
				return ret;
			}
			break;
		case PWR_CMD_DELAY:
			if (seq->val == PWR_DELAY_US)
				PLTFM_DELAY_US(seq->addr);
			else
				PLTFM_DELAY_US(seq->addr * 1000);
			break;
		case PWR_CMD_READ:
			break;
		default:
			PLTFM_MSG_ERR("[ERR]unknown pwr seq cmd %d\n",
				      seq->cmd);
			return MACNOITEM;
		}
next_seq:
		seq++;
	}

	return MACSUCCESS;
}

u32 pwr_seq_start(struct mac_adapter *adapter,
		  struct mac_pwr_cfg **seq)
{
	u8 cv;
	u8 intf;
	u32 ret;
	struct mac_hw_info *hw_info = adapter->hw_info;
	struct mac_pwr_cfg *sub_seq = *seq;
	u32 val = 0;

	cv = PWR_CDV_MSK;

	intf = PWR_INTF_MSK_USB2;

	while (sub_seq) {
		ret = sub_pwr_seq_start(adapter, cv, intf, sub_seq);
		if (ret != MACSUCCESS) {
			PLTFM_MSG_ERR("[ERR]sub pwr seq %d\n", ret);
			return ret;
		}
		seq++;
		sub_seq = *seq;
	}

	return MACSUCCESS;
}

u32 mac_pwr_switch(struct mac_adapter *adapter, u8 on)
{
	u32 ret = MACSUCCESS;
	u32 ret_end;
	struct mac_pwr_cfg **pwr_seq;
	struct mac_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 (*intf_pwr_switch)(void *vadapter, u8 pre_switch, u8 on);
	u32 (*pwr_func)(void *vadapter);
	u32 val32;
	u8 rpwm, val8;
	enum mac_pwr_st mac_pwr;
	struct rtw_hal_com_t *hal_com = (struct rtw_hal_com_t *)adapter->drv_adapter;
	struct dvobj_priv *dvobj = hal_com->drv_priv;
	_adapter *tadapter = dvobj_get_primary_adapter(dvobj);

	pr_info("%s NEO TODO\n", __func__);


	/* Check FW still exist or not */
	if (MAC_REG_R16(REG_MCUFW_CTRL) == 0xC078) {
		/* Leave 32K */
		rpwm = (u8)((rpwm ^ BIT(7)) & 0x80);
		MAC_REG_W8(0xFE58, rpwm);
	}

	val8 = MAC_REG_R8(REG_CR);
	if (val8 == 0xEA) {
		mac_pwr = MAC_PWR_OFF;
	} else {
		if (BIT(0) == (MAC_REG_R8(REG_SYS_STATUS1 + 1) & BIT(0)))
			mac_pwr = MAC_PWR_OFF;
		else
			mac_pwr = MAC_PWR_ON;
	}


	if (on) {
		/*Check if power switch is needed*/
		if (mac_pwr == MAC_PWR_ON) {
			PLTFM_MSG_WARN("[WARN]power state unchange!!\n");
			ret = MACALRDYON;
			goto END;
		}

		pwr_seq = adapter->hw_info->pwr_on_seq;


		/* pre_init_system_cfg_8822c */
		MAC_REG_W8(REG_RSV_CTRL, 0);
		if (MAC_REG_R8(REG_SYS_CFG2 + 3) == 0x20)
			MAC_REG_W8(0xFE5B, MAC_REG_R8(0xFE5B) | BIT(4));

		/* CONFIG PIN MUX */
		val32 = MAC_REG_R32(REG_PAD_CTRL1);
		val32 |= BIT(28) | BIT(29);
		MAC_REG_W32(REG_PAD_CTRL1, val32);

		val32 = MAC_REG_R32(REG_LED_CFG);
		val32 &= ~(BIT(25) | BIT(26));
		MAC_REG_W32(REG_LED_CFG, val32);

		val32 = MAC_REG_R32(REG_GPIO_MUXCFG);
		val32 |= BIT(2);
		MAC_REG_W32(REG_GPIO_MUXCFG, val32);

		/* disable BB/RF */
		val8 = MAC_REG_R8(REG_SYS_FUNC_EN);
		val8 &= ~(BIT(0) | BIT(1));
		MAC_REG_W8(REG_SYS_FUNC_EN, val8);

		val8 = MAC_REG_R8(REG_RF_CTRL);
		val8 &= ~(BIT(0) | BIT(1) | BIT(2));
		MAC_REG_W8(REG_RF_CTRL, val8);


		val32 = MAC_REG_R32(REG_WLRF1);
		val32 &= ~(BIT(24) | BIT(25) | BIT(26));
		MAC_REG_W32(REG_WLRF1, val32);
		
	} else {
		pwr_seq = adapter->hw_info->pwr_off_seq;
	}


	ret = pwr_seq_start(adapter, pwr_seq);
	if (ret != MACSUCCESS) {
		PLTFM_MSG_ERR("[ERR]pwr seq start %d\n", ret);
		goto END;
	}

	if (on) {
		MAC_REG_W8_CLR(REG_SYS_STATUS1 + 1, BIT(0));

		/* init system cfg 8822c */
		val32 = MAC_REG_R32(REG_CPU_DMEM_CON);
		val32 |= BIT(16) | BIT(8);
		MAC_REG_W32(REG_CPU_DMEM_CON, val32);

		val8 = MAC_REG_R8(REG_SYS_FUNC_EN + 1);
		val8 |= 0xD8;
		MAC_REG_W8(REG_SYS_FUNC_EN + 1, val8);

		/* PHY_REQ_DELAY reg 0x1100[27:24] = 0x0C */
		val8 = MAC_REG_R8(REG_CR_EXT + 3) & 0xF0;
		val8 |= 0x0C;
		MAC_REG_W8(REG_CR_EXT + 3, val8);

		/* disable boot-from-flash for driver's DL FW */
		val32 = MAC_REG_R32(REG_MCUFW_CTRL);
		if (val32 & BIT(20)) {
			MAC_REG_W32(REG_MCUFW_CTRL, val32 & (~(BIT(20))));
			val32 = MAC_REG_R32(REG_GPIO_MUXCFG) & (~(BIT(19)));
			MAC_REG_W32(REG_GPIO_MUXCFG, val32);
		}
	}
END:
	return ret;
}
