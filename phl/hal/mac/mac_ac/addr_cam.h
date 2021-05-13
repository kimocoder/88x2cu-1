/** @file */
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

#ifndef _MAC_ADDR_CAM_H_
#define _MAC_ADDR_CAM_H_

#include "../type.h"
#include "fwcmd.h"
//NEO
//#include "../fw_ax/inc_hdr/fwcmd_intf.h"

#define ADDR_CAM_ENT_LONG_SIZE  0x40
#define ADDR_CAM_ENT_SHORT_SIZE  0x20

#define BSSID_CAM_ENT_SIZE 0x08
#define ETH_ALEN 6
#define DEFAULT_HIT_MACID 0x0

/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup ADDRCAM
 * @{
 */
/**
 * @brief addr_cam_init
 *
 * @param *adapter
 * @param *info
 * @param *fw_addrcam
 * @return Please Place Description here.
 * @retval u32
 */
u32 addr_cam_init(struct mac_adapter *adapter,
		  enum mac_band band);

#if 0 //NEO

/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup ADDRCAM
 * @{
 */
/**
 * @brief addr_cam_init
 *
 * @param *adapter
 * @param *info
 * @param *fw_addrcam
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_get_cfg_addr_cam(struct mac_adapter *adapter,
			 struct mac_ax_addrcam_ctrl_t *opt,
			 enum mac_band band);

/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup ADDRCAM
 * @{
 */
/**
 * @brief addr_cam_init
 *
 * @param *adapter
 * @param *info
 * @param *fw_addrcam
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_get_cfg_addr_cam_dis(struct mac_adapter *adapter,
			     struct mac_ax_addrcam_dis_ctrl_t *opt,
			     enum mac_band band);

/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup ADDRCAM
 * @{
 */
/**
 * @brief mac_cfg_addrcam
 *
 * @param *adapter
 * @param *info
 * @param *fw_addrcam
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_cfg_addr_cam(struct mac_adapter *adapter,
		     struct mac_ax_addrcam_ctrl_t *ctl_opt,
		     struct mac_ax_addrcam_ctrl_t *ctl_msk,
		     enum mac_band band);

/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup ADDRCAM
 * @{
 */
/**
 * @brief mac_cfg_addrcam_dis
 *
 * @param *adapter
 * @param *info
 * @param *fw_addrcam
 * @return Please Place Description here.
 * @retval u32
 */
u32 mac_cfg_addr_cam_dis(struct mac_adapter *adapter,
			 struct mac_ax_addrcam_dis_ctrl_t *ctl_opt,
			 struct mac_ax_addrcam_dis_ctrl_t *ctl_msk,
			 enum mac_band band);

/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup ADDRCAM
 * @{
 */
/**
 * @brief fill_addr_cam_info
 *
 * @param *adapter
 * @param *info
 * @param *fw_addrcam
 * @return Please Place Description here.
 * @retval u32
 */
u32 fill_addr_cam_info(struct mac_adapter *adapter,
		       struct mac_ax_role_info *info,
		       struct fwcmd_addrcam_info *fw_addrcam);
/**
 * @}
 * @}
 */
/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup ADDRCAM
 * @{
 */

/**
 * @brief fill_bssid_cam_info
 *
 * @param *adapter
 * @param *info
 * @param *fw_addrcam
 * @return Please Place Description here.
 * @retval u32
 */

u32 fill_bssid_cam_info(struct mac_adapter *adapter,
			struct mac_ax_role_info *info,
			struct fwcmd_addrcam_info *fw_addrcam);
/**
 * @}
 * @}
 */
/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup ADDRCAM
 * @{
 */

/**
 * @brief init_addr_cam_info
 *
 * @param *adapter
 * @param *info
 * @param *fw_addrcam
 * @return Please Place Description here.
 * @retval u32
 */

u32 init_addr_cam_info(struct mac_adapter *adapter,
		       struct mac_ax_role_info *info,
		       struct fwcmd_addrcam_info *fw_addrcam);
/**
 * @}
 * @}
 */
/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup ADDRCAM
 * @{
 */

/**
 * @brief change_addr_cam_info
 *
 * @param *adapter
 * @param *info
 * @param *fw_addrcam
 * @return Please Place Description here.
 * @retval u32
 */

u32 change_addr_cam_info(struct mac_adapter *adapter,
			 struct mac_ax_role_info *info,
			 struct fwcmd_addrcam_info *fw_addrcam);
/**
 * @}
 * @}
 */
/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup ADDRCAM
 * @{
 */

/**
 * @brief mac_upd_addr_cam
 *
 * @param *adapter
 * @param *info
 * @param change_role
 * @return Please Place Description here.
 * @retval u32
 */

u32 mac_upd_addr_cam(struct mac_adapter *adapter,
		     struct mac_role_info *info,
		     enum mac_role_opmode op);

#endif //NEO

/**
 * @}
 * @}
 */

/**
 * @}
 * @}
 */
/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup ADDRCAM
 * @{
 */

/**
 * @brief set_mac_resp_ack
 *
 * The function could contrl MAC resp ACK or not.
 *
 * @param *adapter
 * @param ack
 * @return 0 for success. Others are fail.
 * @retval u32
 */

u32 set_mac_resp_ack(struct mac_adapter *adapter, u32 *ack);
/**
 * @}
 * @}
 */

/**
 * @}
 * @}
 */
/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup ADDRCAM
 * @{
 */

/**
 * @brief get_mac_resp_ack
 *
 * The function could get MAC resp ACK ability
 *
 * @param *adapter
 * @param ack
 * @return 0 for success. Others are fail.
 * @retval u32
 */

u32 get_mac_resp_ack(struct mac_adapter *adapter, u32 *ack);
/**
 * @}
 * @}
 */

/**
 * @}
 * @}
 */
/**
 * @addtogroup Basic_TRX
 * @{
 * @addtogroup ADDRCAM
 * @{
 */

/**
 * @brief get_addr_cam_size
 *
 * Get the entry size of address CAM
 *
 * @param *adapter
 * @return ret the size of address CAM.
 * @retval u32
 */

u8 get_addr_cam_size(struct mac_adapter *adapter);
/**
 * @}
 * @}
 */


#endif
