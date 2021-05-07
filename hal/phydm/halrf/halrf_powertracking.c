/******************************************************************************
 *
 * Copyright(c) 2007 - 2017  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/

/*@************************************************************
 * include files
 * ************************************************************
 */
#include "mp_precomp.h"
#include "phydm_precomp.h"

boolean
odm_check_power_status(void *dm_void)
{
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	PADAPTER *adapter = dm->adapter;

	RT_RF_POWER_STATE rt_state;
	MGNT_INFO *mgnt_info = &((PADAPTER)adapter)->MgntInfo;

	/* 2011/07/27 MH We are not testing ready~~!! We may fail to get correct value when init sequence. */
	if (mgnt_info->init_adpt_in_progress == true) {
		RF_DBG(dm, DBG_RF_INIT,
		       "check_pow_status Return true, due to initadapter\n");
		return true;
	}

	/*
	 *	2011/07/19 MH We can not execute tx pwoer tracking/ LLC calibrate or IQK.
	 */
	((PADAPTER)adapter)->HalFunc.GetHwRegHandler((PADAPTER)adapter, HW_VAR_RF_STATE, (u8 *)(&rt_state));
	if (((PADAPTER)adapter)->bDriverStopped || ((PADAPTER)adapter)->bDriverIsGoingToPnpSetPowerSleep || rt_state == eRfOff) {
		RF_DBG(dm, DBG_RF_INIT,
		       "check_pow_status Return false, due to %d/%d/%d\n",
		       ((PADAPTER)adapter)->bDriverStopped,
		       ((PADAPTER)adapter)->bDriverIsGoingToPnpSetPowerSleep,
		       rt_state);
		return false;
	}
#endif
	return true;
}

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN | ODM_CE))
void halrf_update_pwr_track(void *dm_void, u8 rate)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	RF_DBG(dm, DBG_RF_TX_PWR_TRACK, "Pwr Track Get rate=0x%x\n", rate);

	dm->tx_rate = rate;
}
#endif


void halrf_set_pwr_track(void *dm_void, u8 enable)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_rf_calibration_struct *cali_info = &dm->rf_calibrate_info;
	struct _hal_rf_ *rf = &(dm->rf_table);
	struct txpwrtrack_cfg c;
	u8 i;

	configure_txpower_track(dm, &c);
	if (enable) {
		rf->rf_supportability = rf->rf_supportability | HAL_RF_TX_PWR_TRACK;
		if (cali_info->txpowertrack_control == 1 || cali_info->txpowertrack_control == 3)
			halrf_do_tssi(dm);

	} else {
		rf->rf_supportability = rf->rf_supportability & ~HAL_RF_TX_PWR_TRACK;
		odm_clear_txpowertracking_state(dm);
		halrf_do_tssi(dm);
		halrf_calculate_tssi_codeword(dm);
		halrf_set_tssi_codeword(dm);
		
		for (i = 0; i < c.rf_path_count; i++)
			(*c.odm_tx_pwr_track_set_pwr)(dm, CLEAN_MODE, i, 0);
	}

	if (cali_info->txpowertrack_control == 2 ||
		cali_info->txpowertrack_control == 3 ||
		cali_info->txpowertrack_control == 4 ||
		cali_info->txpowertrack_control == 5)
		halrf_txgapk_reload_tx_gain(dm);
}

