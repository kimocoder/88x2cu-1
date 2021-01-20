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
#ifndef _HAL_TRX_8822C_H_
#define _HAL_TRX_8822C_H_

#define RX_DESC_S_SIZE_8822C 24 


/* wifi packet(RXD.RPKT_TYPE = 0x0) = 32 bytes, otherwise 16 bytes */

#define RX_DESC_L_SIZE_8822C 24
#define RX_BD_INFO_SIZE 4

#if 0 //NEO : mark off first
#define RX_PPDU_MAC_INFO_SIZE_8852A 4

#define ACH0_QUEUE_IDX_8852A 0x0
#define ACH1_QUEUE_IDX_8852A 0x1
#define ACH2_QUEUE_IDX_8852A 0x2
#define ACH3_QUEUE_IDX_8852A 0x3
#define ACH4_QUEUE_IDX_8852A 0x4
#define ACH5_QUEUE_IDX_8852A 0x5
#define ACH6_QUEUE_IDX_8852A 0x6
#define ACH7_QUEUE_IDX_8852A 0x7
#define MGQ_B0_QUEUE_IDX_8852A 0x8
#define HIQ_B0_QUEUE_IDX_8852A 0x9
#define MGQ_B1_QUEUE_IDX_8852A 0xa
#define HIQ_B1_QUEUE_IDX_8852A 0xb
#define FWCMD_QUEUE_IDX_8852A 0xc

#endif // if 0 NEO


#define GET_RX_DESC_SWDEC(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x00, 27, 1)
#define GET_RX_DESC_PHYST(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x00, 26, 1)
#define GET_RX_DESC_SHIFT(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x00, 24, 2)
#define GET_RX_DESC_QOS(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x00, 23, 1)
#define GET_RX_DESC_SECURITY(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x00, 20, 3)
#define GET_RX_DESC_DRV_INFO_SIZE(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x00, 16, 4)
#define GET_RX_DESC_ICV_ERR(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x00, 15, 1)
#define GET_RX_DESC_CRC32(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x00, 14, 1)
#define GET_RX_DESC_PKT_LEN(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x00, 0, 14)

#define GET_RX_DESC_C2H(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x08, 28, 1)
#define GET_RX_DESC_AMSDU(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x04, 13, 1)
#define GET_RX_DESC_MF(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x04, 27, 1)
#define GET_RX_DESC_MD(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x04, 26, 1)
#define GET_RX_DESC_PWR(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x04, 25, 1)

#define GET_RX_DESC_FRAG(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x08, 12, 4)
#define GET_RX_DESC_SEQ(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x08, 0, 12)
#define GET_RX_DESC_PAGGR(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x04, 15, 1)

#define GET_RX_DESC_RX_SCRAMBLER(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x10, 9, 7)
#define GET_RX_DESC_RX_EOF(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x10, 8, 1)

#define GET_RX_DESC_PPDU_CNT(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x08, 29, 2)

/*RXDESC_WORD1*/

#define GET_RX_DESC_BC(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x04, 31, 1)
#define GET_RX_DESC_MC(rxdesc) LE_BITS_TO_4BYTE(rxdesc + 0x04, 30, 1)

/*RXDESC_WORD0*/

#define GET_RX_DESC_EOR_8822C(rxdesc) GET_RX_DESC_EOR(rxdesc)
#define GET_RX_DESC_PHYPKTIDC_8822C(rxdesc) GET_RX_DESC_PHYPKTIDC(rxdesc)
#define GET_RX_DESC_SWDEC_8822C(rxdesc) GET_RX_DESC_SWDEC(rxdesc)
#define GET_RX_DESC_PHYST_8822C(rxdesc) GET_RX_DESC_PHYST(rxdesc)
#define GET_RX_DESC_SHIFT_8822C(rxdesc) GET_RX_DESC_SHIFT(rxdesc)
#define GET_RX_DESC_QOS_8822C(rxdesc) GET_RX_DESC_QOS(rxdesc)
#define GET_RX_DESC_SECURITY_8822C(rxdesc) GET_RX_DESC_SECURITY(rxdesc)
#define GET_RX_DESC_DRV_INFO_SIZE_8822C(rxdesc)                                \
	GET_RX_DESC_DRV_INFO_SIZE(rxdesc)
#define GET_RX_DESC_ICV_ERR_8822C(rxdesc) GET_RX_DESC_ICV_ERR(rxdesc)
#define GET_RX_DESC_CRC32_8822C(rxdesc) GET_RX_DESC_CRC32(rxdesc)
#define GET_RX_DESC_PKT_LEN_8822C(rxdesc) GET_RX_DESC_PKT_LEN(rxdesc)

/*RXDESC_WORD1*/

#define GET_RX_DESC_BC_8822C(rxdesc) GET_RX_DESC_BC(rxdesc)
#define GET_RX_DESC_MC_8822C(rxdesc) GET_RX_DESC_MC(rxdesc)
#define GET_RX_DESC_TY_PE_8822C(rxdesc) GET_RX_DESC_TY_PE(rxdesc)
#define GET_RX_DESC_MF_8822C(rxdesc) GET_RX_DESC_MF(rxdesc)
#define GET_RX_DESC_MD_8822C(rxdesc) GET_RX_DESC_MD(rxdesc)
#define GET_RX_DESC_PWR_8822C(rxdesc) GET_RX_DESC_PWR(rxdesc)
#define GET_RX_DESC_PAM_8822C(rxdesc) GET_RX_DESC_PAM(rxdesc)
#define GET_RX_DESC_CHK_VLD_8822C(rxdesc) GET_RX_DESC_CHK_VLD(rxdesc)
#define GET_RX_DESC_RX_IS_TCP_UDP_8822C(rxdesc)                                \
	GET_RX_DESC_RX_IS_TCP_UDP(rxdesc)
#define GET_RX_DESC_RX_IPV_8822C(rxdesc) GET_RX_DESC_RX_IPV(rxdesc)
#define GET_RX_DESC_CHKERR_8822C(rxdesc) GET_RX_DESC_CHKERR(rxdesc)
#define GET_RX_DESC_PAGGR_8822C(rxdesc) GET_RX_DESC_PAGGR(rxdesc)
#define GET_RX_DESC_RXID_MATCH_8822C(rxdesc) GET_RX_DESC_RXID_MATCH(rxdesc)
#define GET_RX_DESC_AMSDU_8822C(rxdesc) GET_RX_DESC_AMSDU(rxdesc)
#define GET_RX_DESC_MACID_VLD_8822C(rxdesc) GET_RX_DESC_MACID_VLD(rxdesc)
#define GET_RX_DESC_TID_8822C(rxdesc) GET_RX_DESC_TID(rxdesc)
#define GET_RX_DESC_MACID_8822C(rxdesc) GET_RX_DESC_MACID(rxdesc)

/*RXDESC_WORD2*/

#define GET_RX_DESC_FCS_OK_8822C(rxdesc) GET_RX_DESC_FCS_OK(rxdesc)
#define GET_RX_DESC_PPDU_CNT_8822C(rxdesc) GET_RX_DESC_PPDU_CNT(rxdesc)
#define GET_RX_DESC_C2H_8822C(rxdesc) GET_RX_DESC_C2H(rxdesc)
#define GET_RX_DESC_HWRSVD_8822C(rxdesc) GET_RX_DESC_HWRSVD(rxdesc)
#define GET_RX_DESC_WLANHD_IV_LEN_8822C(rxdesc)                                \
	GET_RX_DESC_WLANHD_IV_LEN(rxdesc)
#define GET_RX_DESC_RX_STATISTICS_8822C(rxdesc)                                \
	GET_RX_DESC_RX_STATISTICS(rxdesc)
#define GET_RX_DESC_RX_IS_QOS_8822C(rxdesc) GET_RX_DESC_RX_IS_QOS(rxdesc)
#define GET_RX_DESC_FRAG_8822C(rxdesc) GET_RX_DESC_FRAG(rxdesc)
#define GET_RX_DESC_SEQ_8822C(rxdesc) GET_RX_DESC_SEQ(rxdesc)

/*RXDESC_WORD3*/

#define GET_RX_DESC_MAGIC_WAKE_8822C(rxdesc) GET_RX_DESC_MAGIC_WAKE(rxdesc)
#define GET_RX_DESC_UNICAST_WAKE_8822C(rxdesc) GET_RX_DESC_UNICAST_WAKE(rxdesc)
#define GET_RX_DESC_PATTERN_MATCH_8822C(rxdesc)                                \
	GET_RX_DESC_PATTERN_MATCH(rxdesc)
#define GET_RX_DESC_RXPAYLOAD_MATCH_8822C(rxdesc)                              \
	GET_RX_DESC_RXPAYLOAD_MATCH(rxdesc)
#define GET_RX_DESC_RXPAYLOAD_ID_8822C(rxdesc) GET_RX_DESC_RXPAYLOAD_ID(rxdesc)
#define GET_RX_DESC_DMA_AGG_NUM_8822C(rxdesc) GET_RX_DESC_DMA_AGG_NUM(rxdesc)
#define GET_RX_DESC_BSSID_FIT_1_0_8822C(rxdesc)                                \
	GET_RX_DESC_BSSID_FIT_1_0(rxdesc)
#define GET_RX_DESC_EOSP_8822C(rxdesc) GET_RX_DESC_EOSP(rxdesc)
#define GET_RX_DESC_HTC_8822C(rxdesc) GET_RX_DESC_HTC(rxdesc)
#define GET_RX_DESC_BSSID_FIT_4_2_8822C(rxdesc)                                \
	GET_RX_DESC_BSSID_FIT_4_2(rxdesc)
#define GET_RX_DESC_RX_RATE_8822C(rxdesc) GET_RX_DESC_RX_RATE(rxdesc)

/*RXDESC_WORD4*/

#define GET_RX_DESC_A1_FIT_8822C(rxdesc) GET_RX_DESC_A1_FIT(rxdesc)
#define GET_RX_DESC_MACID_RPT_BUFF_8822C(rxdesc)                               \
	GET_RX_DESC_MACID_RPT_BUFF(rxdesc)
#define GET_RX_DESC_RX_PRE_NDP_VLD_8822C(rxdesc)                               \
	GET_RX_DESC_RX_PRE_NDP_VLD(rxdesc)
#define GET_RX_DESC_RX_SCRAMBLER_8822C(rxdesc) GET_RX_DESC_RX_SCRAMBLER(rxdesc)
#define GET_RX_DESC_RX_EOF_8822C(rxdesc) GET_RX_DESC_RX_EOF(rxdesc)
#define GET_RX_DESC_PATTERN_IDX_8822C(rxdesc) GET_RX_DESC_PATTERN_IDX(rxdesc)

/*RXDESC_WORD5*/

#define GET_RX_DESC_TSFL_8822C(rxdesc) GET_RX_DESC_TSFL(rxdesc)

/*
0000: WIFI packet
0110: TX report
1010: C2H packet */
#define RX_8822C_DESC_PKT_T_WIFI 0
#define RX_8822C_DESC_PKT_T_TX_RPT 6
#define RX_8822C_DESC_PKT_T_C2H 10

#if 0 // NEO

/*
0000: WIFI packet
0001: PPDU status
0010: channel info
0011: BB scope mode
0100: F2P TX CMD report
0101: SS2FW report
0110: TX report
0111: TX payload release to host
1000: DFS report
1001: TX payload release to WLCPU
1010: C2H packet */
#define RX_8852A_DESC_PKT_T_WIFI 0
#define RX_8852A_DESC_PKT_T_PPDU_STATUS 1
#define RX_8852A_DESC_PKT_T_CHANNEL_INFO 2
#define RX_8852A_DESC_PKT_T_BB_SCOPE 3
#define RX_8852A_DESC_PKT_T_F2P_TX_CMD_RPT 4
#define RX_8852A_DESC_PKT_T_SS2FW_RPT 5
#define RX_8852A_DESC_PKT_T_TX_RPT 6
#define RX_8852A_DESC_PKT_T_TX_PD_RELEASE_HOST 7
#define RX_8852A_DESC_PKT_T_DFS_RPT 8
#define RX_8852A_DESC_PKT_T_TX_PD_RELEASE_WLCPU 9
#define RX_8852A_DESC_PKT_T_C2H 10


#define RX_8852A_DESC_PPDU_T_LCCK 0
#define RX_8852A_DESC_PPDU_T_SCCK 1
#define RX_8852A_DESC_PPDU_T_OFDM 2
#define RX_8852A_DESC_PPDU_T_HT 3
#define RX_8852A_DESC_PPDU_T_HTGF 4
#define RX_8852A_DESC_PPDU_T_VHT_SU 5
#define RX_8852A_DESC_PPDU_T_VHT_MU 6
#define RX_8852A_DESC_PPDU_T_HE_SU 7
#define RX_8852A_DESC_PPDU_T_HE_ERSU 8
#define RX_8852A_DESC_PPDU_T_HE_MU 9
#define RX_8852A_DESC_PPDU_T_HE_TB 10
#define RX_8852A_DESC_PPDU_T_UNKNOWN 15


struct rx_ppdu_status{
	u32 mac_info_length;
	u32 phy_info_length;
	//struct mac_info macinfo;
	//struct phy_info phyinfo;
};

#endif // if 0 NEO

enum rtw_hal_status
hal_handle_rx_buffer_8822c(struct rtw_phl_com_t *phl_com,
				struct hal_info_t *hal,
				u8 *buf, u32 buf_len,
				struct rtw_phl_rx_pkt *phl_rx);


#endif /*_HAL_TRX_8822C_H_*/
