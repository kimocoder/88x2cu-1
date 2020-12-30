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
#ifndef _PHL_API_DRV_H_
#define _PHL_API_DRV_H_

void *rtw_phl_get_txbd_buf(struct rtw_phl_com_t *phl_com);
void *rtw_phl_get_rxbd_buf(struct rtw_phl_com_t *phl_com);

/**
 * rtw_phl_query_h2c_pkt - provide h2c buffer for halmac
 * @phl_com: see struct rtw_phl_com_t
 * @type: the type of h2c buf
 *
 * returns struct rtw_h2c_pkt*
 */
struct rtw_h2c_pkt *rtw_phl_query_h2c_pkt(struct rtw_phl_com_t *phl_com,
										  enum rtw_h2c_pkt_type type);

/**
 * rtw_phl_pltfm_tx - h2c platform transmit
 * @phl_com: see struct rtw_phl_com_t
 * @pkt: the h2c pkt
 *
 * returns enum RTW_PHL_STATUS
 */
enum rtw_phl_status rtw_phl_pltfm_tx(struct rtw_phl_com_t *phl_com,
									 struct rtw_h2c_pkt *pkt);

enum rtw_phl_status rtw_phl_msg_hub_hal_send(struct rtw_phl_com_t *phl_com,
						struct phl_msg_attribute* attr, struct phl_msg* msg);

struct rtw_phl_stainfo_t *
rtw_phl_get_stainfo_self(void *phl, struct rtw_wifi_role_t *wrole);
struct rtw_phl_stainfo_t *
rtw_phl_get_stainfo_by_macid(void *phl, u16 macid);


/* For hal wow use */
#define RTW_PHL_PKT_OFLD_REQ(_phl, _macid, _type, _seq, _buf)	\
	rtw_phl_pkt_ofld_request(_phl, _macid, _type, _seq, __func__, _buf)

enum rtw_phl_status rtw_phl_pkt_ofld_request(void *phl, u8 macid, u8 type,
						u32 *token, const char *req_name,
						void *buf);
enum rtw_phl_status rtw_phl_pkt_ofld_cancel(void *phl, u8 macid,
						u8 type, u32 *token);
u8 rtw_phl_pkt_ofld_get_id(void *phl, u8 macid, u8 type);

bool rtw_phl_query_regulation_info(void *phl, struct rtw_regulation_info *info);
bool rtw_phl_regulation_query_ch(void *phl, u8 channel,
				struct rtw_regulation_channel *ch);
u8 rtw_phl_get_center_ch(u8 ch,
	enum channel_width bw, enum chan_offset offset);
void rtw_phl_pkt_ofld_reset_all_entry(struct rtw_phl_com_t *phl_com);

#endif /* _PHL_API_DRV_H_ */

