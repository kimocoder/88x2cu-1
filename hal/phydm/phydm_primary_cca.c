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

/*************************************************************
 * include files
 ************************************************************/
#include "mp_precomp.h"
#include "phydm_precomp.h"
#ifdef PHYDM_PRIMARY_CCA

void phydm_write_dynamic_cca(
	void *dm_void,
	u8 curr_mf_state

	)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct phydm_pricca_struct *pri_cca = &dm->dm_pri_cca;

	if (pri_cca->mf_state == curr_mf_state)
		return;

	pri_cca->mf_state = curr_mf_state;
	PHYDM_DBG(dm, DBG_PRI_CCA, "Set CCA at ((%s SB)), 0xc6c[8:7]=((%d))\n",
		  ((curr_mf_state == MF_USC_LSC) ? "D" :
		  ((curr_mf_state == MF_LSC) ? "L" : "U")), curr_mf_state);
}

void phydm_primary_cca_reset(
	void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct phydm_pricca_struct *pri_cca = &dm->dm_pri_cca;

	PHYDM_DBG(dm, DBG_PRI_CCA, "[PriCCA] Reset\n");
	pri_cca->mf_state = 0xff;
	pri_cca->pre_bw = (enum channel_width)0xff;
	phydm_write_dynamic_cca(dm, MF_USC_LSC);
}

void phydm_primary_cca_11n(
	void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct phydm_pricca_struct *pri_cca = &dm->dm_pri_cca;
	enum channel_width curr_bw = (enum channel_width)*dm->band_width;

	if (!(dm->support_ability & ODM_BB_PRIMARY_CCA))
		return;

	if (!dm->is_linked) {
		PHYDM_DBG(dm, DBG_PRI_CCA, "[PriCCA][No Link!!!]\n");

		if (pri_cca->pri_cca_is_become_linked) {
			phydm_primary_cca_reset(dm);
			pri_cca->pri_cca_is_become_linked = dm->is_linked;
		}
		return;
	} else {
		if (!pri_cca->pri_cca_is_become_linked) {
			PHYDM_DBG(dm, DBG_PRI_CCA, "[PriCCA][Linked !!!]\n");
			pri_cca->pri_cca_is_become_linked = dm->is_linked;
		}
	}

	if (curr_bw != pri_cca->pre_bw) {
		PHYDM_DBG(dm, DBG_PRI_CCA, "[Primary CCA] start ==>\n");
		pri_cca->pre_bw = curr_bw;

		if (curr_bw == CHANNEL_WIDTH_40) {
			if (*dm->sec_ch_offset == SECOND_CH_AT_LSB) {
			/* Primary CH @ upper sideband*/
				PHYDM_DBG(dm, DBG_PRI_CCA,
					  "BW40M, Primary CH at USB\n");
				phydm_write_dynamic_cca(dm, MF_USC);
			} else {
			/*Primary CH @ lower sideband*/
				PHYDM_DBG(dm, DBG_PRI_CCA,
					  "BW40M, Primary CH at LSB\n");
				phydm_write_dynamic_cca(dm, MF_LSC);
			}
		} else {
			PHYDM_DBG(dm, DBG_PRI_CCA, "Not BW40M, USB + LSB\n");
			phydm_primary_cca_reset(dm);
		}
	}
}

boolean
odm_dynamic_primary_cca_dup_rts(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct phydm_pricca_struct *pri_cca = &dm->dm_pri_cca;

	return pri_cca->dup_rts_flag;
}

void phydm_primary_cca_init(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct phydm_pricca_struct *pri_cca = &dm->dm_pri_cca;

}

void phydm_primary_cca(void *dm_void)
{
#ifdef PHYDM_PRIMARY_CCA
	struct dm_struct *dm = (struct dm_struct *)dm_void;


#endif
}
#endif
