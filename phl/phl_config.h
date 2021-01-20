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
#ifndef _PHL_CONFIG_H_
#define _PHL_CONFIG_H_

/* Define correspoding PHL Feature based on information from the Core */
#ifdef PHL_PLATFORM_AP
#define PHL_FEATURE_AP
#elif defined(PHL_PLATFORM_LINUX) || defined(PHL_PLATFORM_WINDOWS)
#define PHL_FEATURE_NIC
#else
#define PHL_FEATURE_NONE
#endif

#ifdef PHL_FEATURE_NONE/* enable compile flag for phl only compilation check */
#ifndef CONFIG_PHL_TEST_SUITE
#define CONFIG_PHL_TEST_SUITE
#endif
#define CONFIG_DFS 1
//#define CONFIG_USB_TX_AGGREGATION
//#define CONFIG_USB_RX_AGGREGATION
#define CONFIG_USB_TX_PADDING_CHK
//#define CONFIG_LOAD_PHY_PARA_FROM_FILE

#define CONFIG_WOW
#define CONFIG_POWER_SAVE
#define CONFIG_WPA3_SUITEB_SUPPORT

#define CONFIG_MR_SUPPORT
#ifdef CONFIG_MR_SUPPORT
#define CONFIG_SCC_SUPPORT
//#define CONFIG_MCC_SUPPORT
#ifdef CONFIG_MCC_SUPPORT
#define MCC_ROLE_NUM 2
#endif /*CONFIG_MCC_SUPPORT*/
//#define CONFIG_DBCC_SUPPORT

#define DBG_PHL_CHAN
#define DBG_PHL_MR
#define PHL_MR_PROC_CMD
#define DBG_CHCTX_RMAP
#endif /*CONFIG_MR_SUPPORT*/

// NEO mark off first
//#define DBG_PHL_MAC_REG_RW

#define CONFIG_RTW_ACS
#define CONFIG_RX_PSTS_PER_PKT

#define CONFIG_PHL_TXSC
#define RTW_PHL_BCN
#define CONFIG_PHL_SDIO_RX_NETBUF_ALLOC_IN_PHL
#endif

/************ Feature flags ************/
// NEO mark off first
//#define DBG_PHL_MAC_REG_RW


#ifdef CONFIG_PHL_TEST_SUITE
#define CONFIG_PHL_TEST_MP
#define CONFIG_PHL_TEST_VERIFY
#endif

#ifdef PHL_PLATFORM_WINDOWS
#define CONFIG_WIN_HANDLE_INTERRUPT
#endif

#ifdef CONFIG_WOW
#define CONFIG_WOWLAN
#define RTW_WKARD_WOW_BDRAM
#define RTW_WKARD_WOW_SKIP_AOAC_RPT
#define RTW_WKARD_WOW_SKIP_WOW_CAM_CONFIG
#define RTW_WKARD_WOW_L2_PWR
#define RTW_WKARD_WOW_DELAY_WAIT_FUNC_DISABLE_DONE
#endif

#ifdef CONFIG_POWER_SAVE
#define CONFIG_PS_PM
#define CONFIG_PS_IPS
#define CONFIG_PS_LPS
#define CONFIG_PS
#endif

#ifdef CONFIG_CONCURRENT_MODE
#define CONFIG_MR_SUPPORT
#endif

/*CONFIG_IFACE_NUMBER*/
#ifdef CONFIG_IFACE_NUMBER
#define MAX_WIFI_ROLE_NUMBER CONFIG_IFACE_NUMBER
#else
#define MAX_WIFI_ROLE_NUMBER 5
#endif

#ifdef CONFIG_MR_SUPPORT
#define CONFIG_SCC_SUPPORT
//#define CONFIG_MCC_SUPPORT
#ifdef CONFIG_MCC_SUPPORT
#define MCC_ROLE_NUM 2
#endif /*CONFIG_MCC_SUPPORT*/
/*#define CONFIG_DBCC_SUPPORT*/

#define DBG_PHL_CHAN
#define DBG_PHL_MR
#define PHL_MR_PROC_CMD
#define DBG_CHCTX_RMAP
#endif

#define CONFIG_PHL_CMD_SCAN

#define DBG_PHL_STAINFO
#define PHL_MAX_STA_NUM 128


#define CONFIG_CMD_DISP

#ifdef DISABLE_CMD_DISPR
#undef CONFIG_CMD_DISP
#endif

#ifdef CONFIG_CMD_DISP
/* enable SOLO mode define to create seperated background thread per dispatcher,
 * otherwise, all dispatcher would share single background thread, which is in share mode.
*/
//#define CONFIG_CMD_DISP_SOLO_MODE
#endif

#ifndef CONFIG_CMD_DISP
#undef CONFIG_PHL_CUSTOM_FEATURE
#undef CONFIG_PHL_CUSTOM_FEATURE_FB
#undef CONFIG_PHL_CMD_SCAN
#endif

#define CONFIG_GEN_GIT_INFO 0
/*#define CONFIG_NEW_HALMAC_INTERFACE*/

#define CONFIG_BTCOEX

#ifdef CONFIG_USB_TX_PADDING_CHK
#define CONFIG_PHL_USB_TX_PADDING_CHK
#endif

#ifdef CONFIG_USB_TX_AGGREGATION
#define CONFIG_PHL_USB_TX_AGGREGATION
#endif

#ifdef CONFIG_USB_RX_AGGREGATION
#define CONFIG_PHL_USB_RX_AGGREGATION
#endif

#if CONFIG_DFS
#define CONFIG_PHL_DFS
#endif

#ifdef CONFIG_PHL_DFS
/*#define CONFIG_PHL_DFS_REGD_FCC*/
/*#define CONFIG_PHL_DFS_REGD_JAP*/
#define CONFIG_PHL_DFS_REGD_ETSI
#endif

#ifdef CONFIG_WPP
#define CONFIG_PHL_WPP
#endif

#ifdef CONFIG_RX_PSTS_PER_PKT
#define CONFIG_PHL_RX_PSTS_PER_PKT
#endif

#ifdef CONFIG_SDIO_RX_NETBUF_ALLOC_IN_PHL
#define CONFIG_PHL_SDIO_RX_NETBUF_ALLOC_IN_PHL
#endif
/************ WKARD flags ************/
#define RTW_WKARD_PHY_CAP

#define RTW_WKARD_LAMODE

#define RTW_WKARD_TXSC

#define RTW_WKARD_BB_C2H

/* Workaround for doing hal reset in changing MP mode will lost the mac entry */
#define RTW_WKARD_MP_MODE_CHANGE

/*
 * One workaround of EFUSE operation
 *  1. Dump EFUSE with FW fail
 */
#define RTW_WKARD_EFUSE_OPERATION

#define RTW_WKARD_ACUT_DISABLE_CQI_FB	/* 8852A_ACUT BFee reply CQI rpt will cause HW Hang */

/* 8852A_ACUT DAC temp workaround for recover mechanism */
#define RTW_WKARD_ACUT_DAC

#define RTW_WKARD_STA_BCN_INTERVAL

#define RTW_WKARD_SER_L1_EXPIRE
#define RTW_WKARD_SER_L2_IGNORED_WHILE_L1

//NEO
//#define RTW_WKARD_SER_USB_POLLING_EVENT

/* #define RTW_WKARD_SER_USB_DISABLE_L1_RCVY_FLOW */

#define RTW_WKARD_BTC_RFETYPE

#define RTW_WKARD_TXBD_UPD_LMT 	/* 8852AE/8852BE txbd index update limitation */

#ifdef CONFIG_WPA3_SUITEB_SUPPORT
#define RTW_WKARD_HW_MGNT_GCMP_256_DISABLE
#endif

/* Workaround for cmac table config
 * - Default is disabled until halbb is ready
 * - This workaround will be removed once fw handles this cfg
 */
/*#define RTW_WKARD_DEF_CMACTBL_CFG*/

/* Workaround for efuse read hidden report
 * - Default is disabled until halmac is ready
 */

/**
 * 8852A acut, HW search BF Entry from Idx1,
 * and HW MAC CR default value is 0, it will cause
 * macid 0 alway have sound status = fail, and cannot Tx BF pkt.
 * Workaround: set all of BF Entry's default macid to 0xFF in BFer_init,
 * make HW can search correct available Idx0 for TxBF */
#define RTW_WKARD_BFER_INIT

#define RTW_WKARD_PRELOAD_TRX_RESET

/* Workaround for cmac table config
 * - This workaround will be removed once fw handles this cfg
 */
#define RTW_WKARD_DEF_CMACTBL_CFG

#define RTW_WKARD_USB_TXAGG_BULK_END_WD
#ifdef CONFIG_HOMOLOGATION
#define CONFIG_PHL_HOMOLOGATION
#endif

#ifdef RTW_WKARD_TX_DISABLE_BFEE
#define RTW_WKARD_DYNAMIC_BFEE_CAP
#endif

#ifdef RTW_WKARD_PHY_INFO_NTFY
#define CONFIG_PHY_INFO_NTFY
#endif

/* Initialize flow should be refined */
#define	RTW_WKARD_RADIO_IPS_FLOW

/* LPS should disable other role */
#define RTW_WKARD_LPS_ROLE_CONFIG

#ifdef PHL_PLATFORM_WINDOWS
#define RTW_WKARD_WIN_TRX_BALANCE
#endif

/*
 * Workaround for MRC bk module call phl_mr_offch_hdl with scan_issue_null_data
 * ops, this should be replaced with phl issue null data function.
 */
#define RTW_WKARD_MRC_ISSUE_NULL_WITH_SCAN_OPS

/*
 * Workaround for phl_mr_offch_hdl sleep after issue null data,
 * - This workaround will be removed once tx report is ready
 */
#define RTW_WKARD_ISSUE_NULL_SLEEP_PROTECTION

/*
 * Workaround for ap mode he rate adaptation
*  This workaround will be removed once bb confirm ltf_gi is ready 
 */
#define RTW_WKARD_AP_RA

/*
 * Workaround for blocking scan during MCC
 */
#ifdef CONFIG_MCC_SUPPORT
#define RTW_WKARD_SKIP_SCAN_IN_MCC
#endif

#ifdef RTW_WKARD_LPS_IQK_TWICE
#define RTW_WKARD_PHL_LPS_IQK_TWICE
#endif

#define RTW_WKARD_BUSCAP_IN_HALSPEC

#endif /*_PHL_CONFIG_H_*/
