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
#define _PHL_CHAN_C_
#include "phl_headers.h"

#if 0 // NEO TODO

const char *const _band_str[] = {
	"BAND_ON_24G",
	"BAND_ON_5G",
	"BAND_ON_6G",
	"BAND_UNKNOWN"
};
#define _get_band_str(band) (((band) >= BAND_MAX) ? _band_str[BAND_MAX] : _band_str[(band)])

const char *const _bw_str[] = {
	"BW_20M",
	"BW_40M",
	"BW_80M",
	"BW_160M",
	"BW_80_80M",
	"BW_5M",
	"BW_10M",
	"BW_UNKNOWN"
};
#define _get_bw_str(bw) (((bw) >= CHANNEL_WIDTH_MAX) ? _bw_str[CHANNEL_WIDTH_MAX] : _bw_str[((bw))])

#ifdef DBG_PHL_CHAN
void phl_chan_dump_chandef(const char *caller, const int line, bool show_caller,
				struct rtw_chan_def *chandef)
{
	if (show_caller)
		PHL_INFO("###### FUN - %s LINE - %d #######\n", caller, line);

	PHL_INFO("\t[CH] band:%s\n", _get_band_str(chandef->band));
	PHL_INFO("\t[CH] chan:%d\n", chandef->chan);
	PHL_INFO("\t[CH] center_ch:%d\n", chandef->center_ch);
	PHL_INFO("\t[CH] bw:%s\n", _get_bw_str(chandef->bw));
	PHL_INFO("\t[CH] offset:%d\n", chandef->offset);

	PHL_INFO("\t[CH] center_freq1:%d\n", chandef->center_freq1);
	PHL_INFO("\t[CH] center_freq2:%d\n", chandef->center_freq2);
	PHL_INFO("\t[CH] hw_value:%d\n", chandef->hw_value);

	if (show_caller)
		PHL_INFO("#################################\n");
}
#endif

#ifdef CONFIG_PHL_DFS
static enum rtw_phl_status
phl_radar_detect_hdl(struct phl_info_t *phl_info,
	u8 channel, enum channel_width bwmode, enum chan_offset offset)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct rtw_dfs_t *dfs_info = &phl_com->dfs_info;
	enum rtw_phl_status rst = RTW_PHL_STATUS_FAILURE;
	bool overlap_radar_range;

	overlap_radar_range = rtw_hal_in_radar_domain(phl_info->hal,
						channel, bwmode);
	if (overlap_radar_range)
		PHL_INFO("chan in DFS domain ch:%d,bw:%d\n", channel, bwmode);


	if (overlap_radar_range && !dfs_info->dfs_enabled) {
		/*radar_detect_enable*/
		if (rtw_hal_radar_detect_cfg(phl_info->hal, true) ==
			RTW_HAL_STATUS_SUCCESS) {
			dfs_info->dfs_enabled = true;
			PHL_INFO("[DFS] chan(%d) in radar range, enable dfs\n",
				channel);
			rst = RTW_PHL_STATUS_SUCCESS;
		}
		else {
			PHL_ERR("[DFS] chan(%d) in radar range, enable dfs failed\n",
				channel);
		}

	} else if (!overlap_radar_range && dfs_info->dfs_enabled) {
		/*radar_detect_disable*/
		if (rtw_hal_radar_detect_cfg(phl_info->hal, false) ==
			RTW_HAL_STATUS_SUCCESS) {
			dfs_info->dfs_enabled = false;
			PHL_INFO("[DFS] chan(%d) not in radar range, disable dfs\n",
				channel);
			rst = RTW_PHL_STATUS_SUCCESS;
		}
		else {
			PHL_ERR("[DFS] chan(%d) not in radar range, disable dfs failed\n",
				channel);
		}
	}
	return rst;
}
#endif /*CONFIG_PHL_DFS*/
#endif // if 0 NEO

enum rtw_phl_status
rtw_phl_set_ch_bw(struct rtw_wifi_role_t *wifi_role,
		  u8 chan, enum channel_width bw, enum chan_offset offset, bool do_rfk)
{
	struct phl_info_t *phl_info = wifi_role->phl_com->phl_priv;
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;

#ifdef CONFIG_PHL_DFS
	phl_radar_detect_hdl(phl_info, chan, bw, offset);
#endif

	hstatus = rtw_hal_set_ch_bw(phl_info->hal, wifi_role->hw_band,
				    chan, bw, offset, do_rfk);
	if (RTW_HAL_STATUS_SUCCESS != hstatus)
		PHL_ERR("%s rtw_hal_set_ch_bw: statuts = %u\n", __func__, hstatus);

	return RTW_PHL_STATUS_SUCCESS;
}

#if 0 // NEO mark off first

u8 rtw_phl_get_cur_ch(struct rtw_wifi_role_t *wifi_role)
{
	struct phl_info_t *phl_info = wifi_role->phl_com->phl_priv;

	return rtw_hal_get_cur_ch(phl_info->hal, wifi_role->hw_band);
}

enum rtw_phl_status
rtw_phl_dfs_hw_tx_pause(struct rtw_wifi_role_t *wifi_role, bool tx_pause)
{

	struct phl_info_t *phl_info = wifi_role->phl_com->phl_priv;
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;

	hstatus = rtw_hal_dfs_pause_tx(phl_info->hal, wifi_role->hw_band, tx_pause);

	if (RTW_HAL_STATUS_SUCCESS == hstatus) {
		return RTW_PHL_STATUS_SUCCESS;
	} else {
		PHL_ERR("%s Failure :%u\n",__func__, hstatus);
		return RTW_PHL_STATUS_FAILURE;
	}
}
#ifdef CONFIG_DBCC_SUPPORT
enum rtw_phl_status
rtw_phl_dbcc_test(void *phl, enum dbcc_test_id id, void *param)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_hal_status hsts = RTW_HAL_STATUS_FAILURE;

	switch (id){
	case DBCC_PRE_CFG :
	{
		bool dbcc_en = *(bool *)param;

		PHL_INFO("[DBCC] PRE_CFG :%s\n", (dbcc_en) ? "EN" : "DIS");
		hsts = rtw_hal_dbcc_pre_cfg(phl_info->hal, phl_info->phl_com, dbcc_en);
	}
	break;

	case DBCC_CFG :
	{
		bool dbcc_en = *(bool *)param;

		PHL_INFO("[DBCC] CFG :%s\n", (dbcc_en) ? "EN" : "DIS");
		hsts = rtw_hal_dbcc_cfg(phl_info->hal, phl_info->phl_com, dbcc_en);
	}
	break;
	case DBCC_CLEAN_TXQ :
		hsts = rtw_hal_clean_tx_queue(phl_info->hal);
		break;
	default :
		PHL_ERR("%s unknown DBCC Test ID:%d\n",__func__, id);
		break;
	}

	return RTW_PHL_STATUS_SUCCESS;
}
#endif

#define MAX_CHANCTX_QUEUE_NUM	2


static enum rtw_phl_status
_phl_chanctx_add(struct phl_info_t *phl_info,
			struct phl_queue *chan_ctx_queue,
			struct rtw_chan_ctx *chanctx)
{
	if (!chanctx)
		return RTW_PHL_STATUS_FAILURE;

	list_add_tail(&chanctx->list, &chan_ctx_queue->queue);
	chan_ctx_queue->cnt++;
	if (chan_ctx_queue->cnt > MAX_CHANCTX_QUEUE_NUM) {
		PHL_ERR("%s chan_ctx_queue cnt(%d) > 2\n", __func__, chan_ctx_queue->cnt);
		_os_warn_on(1);
	}

	return RTW_PHL_STATUS_SUCCESS;
}

static enum rtw_phl_status
_phl_chanctx_add_with_lock(struct phl_info_t *phl_info,
			struct phl_queue *chan_ctx_queue,
			struct rtw_chan_ctx *chanctx)
{
	void *drv = phl_to_drvpriv(phl_info);
	/*_os_spinlockfg sp_flags;*/

	if (!chanctx)
		return RTW_PHL_STATUS_FAILURE;

	_os_spinlock(drv, &chan_ctx_queue->lock, _ps, NULL);
	_phl_chanctx_add(phl_info, chan_ctx_queue, chanctx);
	_os_spinunlock(drv, &chan_ctx_queue->lock, _ps, NULL);
	return RTW_PHL_STATUS_SUCCESS;
}

static enum rtw_phl_status
_phl_chanctx_del(struct phl_info_t *phl_info,
			struct phl_queue *chan_ctx_queue,
			struct rtw_chan_ctx *chanctx)
{
	if (!chanctx)
		return RTW_PHL_STATUS_FAILURE;

	/*if (!list_empty(&chan_ctx_queue->queue)) {*/
	if (chan_ctx_queue->cnt) {
		list_del(&chanctx->list);
		chan_ctx_queue->cnt--;
		if (chan_ctx_queue->cnt < 0) {
			PHL_ERR("%s chan_ctx_queue cnt(%d) < 0\n", __func__, chan_ctx_queue->cnt);
			_os_warn_on(1);
		}
	}
	return RTW_PHL_STATUS_SUCCESS;
}

static enum rtw_phl_status
_phl_chanctx_del_with_lock(struct phl_info_t *phl_info,
			struct phl_queue *chan_ctx_queue,
			struct rtw_chan_ctx *chanctx)
{
	void *drv = phl_to_drvpriv(phl_info);
	/*_os_spinlockfg sp_flags;*/

	if (!chanctx)
		return RTW_PHL_STATUS_FAILURE;

	_os_spinlock(drv, &chan_ctx_queue->lock, _ps, NULL);
	_phl_chanctx_del(phl_info, chan_ctx_queue, chanctx);
	_os_spinunlock(drv, &chan_ctx_queue->lock, _ps, NULL);
	return RTW_PHL_STATUS_SUCCESS;
}

static inline enum rtw_phl_status
_phl_chanctx_rmap_set(struct phl_info_t *phl_info,
			struct rtw_wifi_role_t *wifi_role,
			struct phl_queue *chan_ctx_queue,
			struct rtw_chan_ctx *chanctx)
{
	u8 ridx = wifi_role->id;

	if (!chanctx)
		return RTW_PHL_STATUS_FAILURE;

	#ifdef DBG_CHCTX_RMAP
	if (chanctx->role_map & BIT(ridx))
		PHL_ERR("wifi_role idx(%d) has in chanctx->role_map(0x%02x)\n",
				ridx, chanctx->role_map);
	#endif
	chanctx->role_map |= BIT(ridx);
	wifi_role->chanctx = chanctx;
	return RTW_PHL_STATUS_SUCCESS;
}

static enum rtw_phl_status
_phl_chanctx_rmap_set_with_lock(struct phl_info_t *phl_info,
			struct rtw_wifi_role_t *wifi_role,
			struct phl_queue *chan_ctx_queue,
			struct rtw_chan_ctx *chanctx)
{
	void *drv = phl_to_drvpriv(phl_info);
	/*_os_spinlockfg sp_flags;*/

	if (!chanctx)
		return RTW_PHL_STATUS_FAILURE;

	_os_spinlock(drv, &chan_ctx_queue->lock, _ps, NULL);
	_phl_chanctx_rmap_set(phl_info, wifi_role, chan_ctx_queue, chanctx);
	_os_spinunlock(drv, &chan_ctx_queue->lock, _ps, NULL);
	return RTW_PHL_STATUS_SUCCESS;
}

static inline enum rtw_phl_status
_phl_chanctx_rmap_clr(struct phl_info_t *phl_info,
			struct rtw_wifi_role_t *wifi_role,
			struct phl_queue *chan_ctx_queue,
			struct rtw_chan_ctx *chanctx)
{
	u8 ridx = wifi_role->id;

	if (!chanctx)
		return RTW_PHL_STATUS_FAILURE;

	#ifdef DBG_CHCTX_RMAP
	if (!(chanctx->role_map & BIT(ridx)))
		PHL_ERR("ridx(%d) hasn't in chanctx->role_map(0x%02x)\n", ridx, chanctx->role_map);
	#endif
	wifi_role->chanctx = NULL;
	chanctx->role_map &= ~BIT(ridx);

	return RTW_PHL_STATUS_SUCCESS;
}

static enum rtw_phl_status
_phl_chanctx_rmap_clr_with_lock(struct phl_info_t *phl_info,
			struct rtw_wifi_role_t *wifi_role,
			struct phl_queue *chan_ctx_queue,
			struct rtw_chan_ctx *chanctx)
{
	void *drv = phl_to_drvpriv(phl_info);
	/*_os_spinlockfg sp_flags;*/

	if (!chanctx)
		return RTW_PHL_STATUS_FAILURE;

	_os_spinlock(drv, &chan_ctx_queue->lock, _ps, NULL);
	_phl_chanctx_rmap_clr(phl_info, wifi_role, chan_ctx_queue, chanctx);
	_os_spinunlock(drv, &chan_ctx_queue->lock, _ps, NULL);
	return RTW_PHL_STATUS_SUCCESS;
}

#endif // if 0 NEO

u8 phl_chanctx_get_rnum(struct phl_info_t *phl_info,
					struct phl_queue *chan_ctx_queue,
					struct rtw_chan_ctx *chanctx)
{
	u8 i;
	u8 role_num = 0;

	for (i = 0; i < MAX_WIFI_ROLE_NUMBER; i++)
		if (chanctx->role_map & BIT(i))
			role_num++;
	return role_num;
}

u8 phl_chanctx_get_rnum_with_lock(struct phl_info_t *phl_info,
			struct phl_queue *chan_ctx_queue,
			struct rtw_chan_ctx *chanctx)
{
	void *drv = phl_to_drvpriv(phl_info);
	u8 role_num = 0;

	if (!chanctx)
		return role_num;

	_os_spinlock(drv, &chan_ctx_queue->lock, _ps, NULL);
	role_num = phl_chanctx_get_rnum(phl_info, chan_ctx_queue, chanctx);
	_os_spinunlock(drv, &chan_ctx_queue->lock, _ps, NULL);
	return role_num;
}

#if 0 // NEO TODO
/**
 * _phl_is_chbw_grouped - test if the two ch settings can be grouped together
 * @ch_a: ch of set a
 * @bw_a: bw of set a
 * @offset_a: offset of set a
 * @ch_b: ch of set b
 * @bw_b: bw of set b
 * @offset_b: offset of set b
 */
static bool _phl_is_chbw_grouped(u8 ch_a, enum channel_width bw_a, enum chan_offset offset_a
			 , u8 ch_b, enum channel_width bw_b, enum chan_offset offset_b)
{
	bool is_grouped = false;

	if (ch_a != ch_b) {
		/* ch is different */
		goto exit;
	} else if ((bw_a == CHANNEL_WIDTH_40 || bw_a == CHANNEL_WIDTH_80)
		   && (bw_b == CHANNEL_WIDTH_40 || bw_b == CHANNEL_WIDTH_80)
		  ) {
		if (offset_a != offset_b)
			goto exit;
	}

	is_grouped = true;

exit:
	return is_grouped;
}


static inline bool
_phl_feature_check(struct rtw_phl_com_t *phl_com, u8 flg)
{
	return (phl_com->dev_cap.hw_sup_flags & flg) ? true : false;
}

static u8 _phl_get_offset_by_chbw(u8 ch, enum channel_width bw, enum chan_offset *r_offset)
{
	u8 valid = 1;
	enum chan_offset offset = CHAN_OFFSET_NO_EXT;

	if (bw == CHANNEL_WIDTH_20)
		goto exit;

	if (bw >= CHANNEL_WIDTH_80 && ch <= 14) {
		valid = 0;
		goto exit;
	}

	if (ch >= 1 && ch <= 4)
		offset = CHAN_OFFSET_UPPER;
	else if (ch >= 5 && ch <= 9) {
		if (*r_offset == CHAN_OFFSET_UPPER || *r_offset == CHAN_OFFSET_LOWER)
			offset = *r_offset; /* both lower and upper is valid, obey input value */
		else
			offset = CHAN_OFFSET_LOWER; /* default use upper */
	} else if (ch >= 10 && ch <= 13)
		offset = CHAN_OFFSET_LOWER;
	else if (ch == 14) {
		valid = 0; /* ch14 doesn't support 40MHz bandwidth */
		goto exit;
	} else if (ch >= 36 && ch <= 177) {
		switch (ch) {
		case 36:
		case 44:
		case 52:
		case 60:
		case 100:
		case 108:
		case 116:
		case 124:
		case 132:
		case 140:
		case 149:
		case 157:
		case 165:
		case 173:
			offset = CHAN_OFFSET_UPPER;
			break;
		case 40:
		case 48:
		case 56:
		case 64:
		case 104:
		case 112:
		case 120:
		case 128:
		case 136:
		case 144:
		case 153:
		case 161:
		case 169:
		case 177:
			offset = CHAN_OFFSET_LOWER;
			break;
		default:
			valid = 0;
			break;
		}
	} else
		valid = 0;

exit:
	if (valid && r_offset)
		*r_offset = offset;
	return valid;
}

/**
 * _phl_adjust_chandef - obey g_ch, adjust g_bw, g_offset, bw, offset
 * @req_ch: pointer of the request ch, may be modified further
 * @req_bw: pointer of the request bw, may be modified further
 * @req_offset: pointer of the request offset, may be modified further
 * @g_ch: pointer of the ongoing group ch
 * @g_bw: pointer of the ongoing group bw, may be modified further
 * @g_offset: pointer of the ongoing group offset, may be modified further
 */
static void _phl_adjust_chandef(u8 *req_ch, enum channel_width *req_bw, enum chan_offset *req_offset,
		   u8 *g_ch, enum channel_width *g_bw, enum chan_offset *g_offset)
{

	*req_ch = *g_ch;

	if (*req_bw == CHANNEL_WIDTH_80 && *g_ch <= 14) {
		/*2.4G ch, downgrade to 40Mhz */
		*req_bw = CHANNEL_WIDTH_40;
	}

	switch (*req_bw) {
	case CHANNEL_WIDTH_80:
		if (*g_bw == CHANNEL_WIDTH_40 || *g_bw == CHANNEL_WIDTH_80)
			*req_offset = *g_offset;
		else if (*g_bw == CHANNEL_WIDTH_20)
			_phl_get_offset_by_chbw(*req_ch, *req_bw, req_offset);

		if (*req_offset == CHAN_OFFSET_NO_EXT) {
			PHL_ERR("%s req 80MHz BW without offset, down to 20MHz\n", __func__);
			_os_warn_on(1);
			*req_bw = CHANNEL_WIDTH_20;
		}
		break;
	case CHANNEL_WIDTH_40:
		if (*g_bw == CHANNEL_WIDTH_40 || *g_bw == CHANNEL_WIDTH_80)
			*req_offset = *g_offset;
		else if (*g_bw == CHANNEL_WIDTH_20)
			_phl_get_offset_by_chbw(*req_ch, *req_bw, req_offset);

		if (*req_offset == CHAN_OFFSET_NO_EXT) {
			PHL_ERR("%s req 40MHz BW without offset, down to 20MHz\n", __func__);
			_os_warn_on(1);
			*req_bw = CHANNEL_WIDTH_20;
		}
		break;
	case CHANNEL_WIDTH_20:
		*req_offset = CHAN_OFFSET_NO_EXT;
		break;
	default:
		PHL_ERR("%s req unsupported BW:%u\n", __func__, *req_bw);
		_os_warn_on(1);
	}

	if (*req_bw > *g_bw) {
		*g_bw = *req_bw;
		*g_offset = *req_offset;
	}
}

static enum rtw_phl_status
_phl_chanctx_create(struct phl_info_t *phl_info,
		struct rtw_wifi_role_t *wifi_role,
		enum band_type band, u8 chan,
		enum channel_width bw,	enum chan_offset offset)
{
	enum rtw_phl_status phl_sts = RTW_PHL_STATUS_FAILURE;
	void *drv = phl_to_drvpriv(phl_info);
	struct rtw_chan_ctx *chanctx = NULL;
	struct mr_ctl_t *mr_ctl = phlcom_to_mr_ctrl(phl_info->phl_com);
	struct hw_band_ctl_t *band_ctrl = &(mr_ctl->band_ctrl[wifi_role->hw_band]);

	chanctx = _os_kmem_alloc(drv, sizeof(struct rtw_chan_ctx));
	if (chanctx == NULL) {
		PHL_ERR("alloc chanctx failed\n");
		goto _exit;
	}

	chanctx->chan_def.band = band;
	chanctx->chan_def.chan = chan;
	chanctx->chan_def.bw = bw;
	chanctx->chan_def.offset = offset;
	chanctx->chan_def.center_ch = rtw_phl_get_center_ch(chan, bw, offset);
	phl_sts = _phl_chanctx_add_with_lock(phl_info, &band_ctrl->chan_ctx_queue, chanctx);

	if (phl_sts == RTW_PHL_STATUS_SUCCESS)
		_phl_chanctx_rmap_set_with_lock(phl_info, wifi_role,
					&band_ctrl->chan_ctx_queue, chanctx);
_exit:
	return phl_sts;
}
bool phl_chanctx_add(struct phl_info_t *phl_info, struct rtw_wifi_role_t *wifi_role,
		u8 *chan, enum channel_width *bw, enum chan_offset *offset)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	enum rtw_phl_status phl_sts = RTW_PHL_STATUS_FAILURE;
	void *drv = phl_to_drvpriv(phl_info);
	struct mr_ctl_t *mr_ctl = phlcom_to_mr_ctrl(phl_com);
	struct hw_band_ctl_t *band_ctrl = &(mr_ctl->band_ctrl[wifi_role->hw_band]);
	struct rtw_chan_ctx *chanctx = NULL;
	struct rtw_chan_def *chandef = NULL;
	_os_list *chan_ctx_list = &band_ctrl->chan_ctx_queue.queue;
	bool is_ch_grouped = false;
	enum band_type band = (*chan > 14) ? BAND_ON_5G : BAND_ON_24G;
	int chanctx_num = 0;

	if (wifi_role == NULL) {
		PHL_ERR("%s wrole == NULL\n", __func__);
		goto _exit;
	}

	PHL_INFO("%s - hw_band_idx:%d, chan:%d, bw:%d, offset:%d\n",
		__func__, wifi_role->hw_band, *chan, *bw, *offset);

	chanctx_num = phl_mr_get_chanctx_num(phl_info, band_ctrl);
	if (chanctx_num == 0) {
		phl_sts = _phl_chanctx_create(phl_info, wifi_role,
						band, *chan, *bw, *offset);
		if (phl_sts != RTW_PHL_STATUS_SUCCESS) {
			PHL_ERR("%s failed\n", __func__);
			_os_warn_on(1);
		}
		else {
			is_ch_grouped = true;
		}
	}
	else {
		_os_spinlock(drv, &band_ctrl->chan_ctx_queue.lock, _ps, NULL);
		phl_list_for_loop(chanctx, struct rtw_chan_ctx, chan_ctx_list, list) {
			chandef = &chanctx->chan_def;
			is_ch_grouped = _phl_is_chbw_grouped(
					chandef->chan, chandef->bw, chandef->offset,
					*chan, *bw, *offset);
			if (is_ch_grouped) {
				_phl_adjust_chandef(chan, bw, offset,
					&chandef->chan, &chandef->bw, &chandef->offset);
				_phl_chanctx_rmap_set(phl_info, wifi_role,
					&band_ctrl->chan_ctx_queue, chanctx);
				break;
			}
		}
		_os_spinunlock(drv, &band_ctrl->chan_ctx_queue.lock, _ps, NULL);

		if (is_ch_grouped == false) { /*MCC or DBCC*/
			PHL_INFO("%s chan:%d, bw:%d, offset:%d could not grouped\n",
				__func__, *chan, *bw, *offset);

			#ifdef CONFIG_MCC_SUPPORT
			if (phl_com->dev_cap.mcc_sup == false) {
				PHL_ERR("%s don't support MCC\n", __func__);
				goto _exit;
			}

			if (chanctx_num >= 2) {
				PHL_ERR("chan_ctx cnt(%d) >= 2\n", chanctx_num);
				/*DBCC ?*/
				goto _exit;
			}
			if (band == chandef->band) { /*MCC*/
				phl_sts = _phl_chanctx_create(phl_info, wifi_role,
							band, *chan, *bw, *offset);
				if (phl_sts == RTW_PHL_STATUS_SUCCESS)
					is_ch_grouped = true;
			} else {
				/*DBCC*/
				#ifdef CONFIG_DBCC_SUPPORT
				if (phl_com->dev_cap.dbcc_sup == true) {
					PHL_INFO("%s support DBC\n", __func__);
					goto _exit;
				}
				#endif
				/*MCC*/
				phl_sts = _phl_chanctx_create(phl_info, wifi_role,
							band, *chan, *bw, *offset);
				if (phl_sts == RTW_PHL_STATUS_SUCCESS)
					is_ch_grouped = true;

			}
			#endif
		}
	}

_exit:
	return is_ch_grouped;
}

#endif // if 0 NEO

enum rtw_phl_status
phl_chanctx_free(struct phl_info_t *phl_info, struct hw_band_ctl_t *band_ctl)
{
	int chanctx_num = 0;
	struct rtw_chan_ctx *chanctx = NULL;
	struct phl_queue *chan_ctx_queue = &band_ctl->chan_ctx_queue;
	void *drv = phl_to_drvpriv(phl_info);

	chanctx_num = phl_mr_get_chanctx_num(phl_info, band_ctl);
	if (chanctx_num == 0)
		return RTW_PHL_STATUS_SUCCESS;

	PHL_INFO("%s band_idx:%d chctx_num:%d\n", __func__, band_ctl->id, chanctx_num);
	do {
		_os_spinlock(drv, &band_ctl->chan_ctx_queue.lock, _ps, NULL);
		if (list_empty(&chan_ctx_queue->queue)) {
			chanctx = NULL;
		} else {
			chanctx = list_first_entry(&chan_ctx_queue->queue,
						struct rtw_chan_ctx, list);
			list_del(&chanctx->list);
			chan_ctx_queue->cnt--;
		}
		_os_spinunlock(drv, &band_ctl->chan_ctx_queue.lock, _ps, NULL);

		if (chanctx) {
			_os_kmem_free(drv, chanctx, sizeof(struct rtw_chan_ctx));
		}
	} while (chanctx != NULL);
	return RTW_PHL_STATUS_SUCCESS;
}

#if 0 // NEO TODO

bool rtw_phl_chanctx_chk(void *phl, struct rtw_wifi_role_t *wifi_role,
		u8 chan, enum channel_width bw, enum chan_offset offset)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	void *drv = phl_to_drvpriv(phl_info);
	struct mr_ctl_t *mr_ctl = phlcom_to_mr_ctrl(phl_com);
	u8 band_idx = wifi_role->hw_band;
	bool is_ch_group = false;
	struct hw_band_ctl_t *band_ctrl = &(mr_ctl->band_ctrl[band_idx]);
	int chanctx_num = 0;
	struct rtw_chan_ctx *chanctx = NULL;
	struct rtw_chan_def *chandef = NULL;

	if (chan == 0) {
		PHL_ERR("%s req chan = 0 \n", __func__);
		goto _exit;
	}

	/*status check*/
	if (mr_ctl->is_sb) {
		if (band_idx == 1) {
			PHL_ERR("wrole:%d in band_idx:%d\n", wifi_role->id, band_idx);
			_os_warn_on(1);
			goto _exit;
		}
	}

	chanctx_num = phl_mr_get_chanctx_num(phl_info, band_ctrl);

	if (chanctx_num > 0) {
		_os_spinlock(drv, &band_ctrl->chan_ctx_queue.lock, _ps, NULL);
		phl_list_for_loop(chanctx, struct rtw_chan_ctx, &band_ctrl->chan_ctx_queue.queue, list) {
			chandef = &chanctx->chan_def;
			is_ch_group = _phl_is_chbw_grouped(
					chandef->chan, chandef->bw, chandef->offset,
					chan, bw, offset);
			if (is_ch_group)
				break;
		}
		_os_spinunlock(drv, &band_ctrl->chan_ctx_queue.lock, _ps, NULL);
	} else {
		is_ch_group = true;
	}
_exit:
	PHL_DUMP_MR_EX(phl_info);
	return is_ch_group;
}

bool rtw_phl_chanctx_add(void *phl, struct rtw_wifi_role_t *wifi_role,
		u8 *chan, enum channel_width *bw, enum chan_offset *offset)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	void *drv = phl_to_drvpriv(phl_info);
	struct mr_ctl_t *mr_ctl = phlcom_to_mr_ctrl(phl_com);
	u8 band_idx = wifi_role->hw_band;
	bool is_ch_grouped = false;
	struct hw_band_ctl_t *band_ctrl = &(mr_ctl->band_ctrl[band_idx]);
	int chanctx_num = 0;
	u8 chctx_role_num = 0;

	if(!chan || !bw || !offset)
		goto _exit;

	if (*chan == 0) {
		PHL_ERR("%s req chan = 0 \n", __func__);
		goto _exit;
	}

	/*status check*/
	if (mr_ctl->is_sb) {
		if (band_idx == 1) {
			PHL_ERR("wrole:%d in band_idx:%d\n", wifi_role->id, band_idx);
			goto _exit;
		}
	}


	is_ch_grouped = phl_chanctx_add(phl_info, wifi_role, chan, bw, offset);
	if (is_ch_grouped) {
		chanctx_num = phl_mr_get_chanctx_num(phl_info, band_ctrl);
		if (chanctx_num == 2) {
			band_ctrl->op_mode = MR_OP_MCC;
		} else if (chanctx_num == 1) {
			struct rtw_chan_ctx *chanctx = NULL;
			struct phl_queue *chan_ctx_queue = &band_ctrl->chan_ctx_queue;

			_os_spinlock(drv, &chan_ctx_queue->lock, _ps, NULL);
			chanctx = list_first_entry(&chan_ctx_queue->queue,
							struct rtw_chan_ctx, list);
			chctx_role_num = phl_chanctx_get_rnum(phl_info, chan_ctx_queue, chanctx);
			if (chctx_role_num >= 2)
				band_ctrl->op_mode = MR_OP_SCC;
			else
				band_ctrl->op_mode = MR_OP_NON;
			_os_spinunlock(drv, &chan_ctx_queue->lock, _ps, NULL);
		}
	}
	#ifdef CONFIG_DBCC_SUPPORT
	else {
		if ((phl_com->dev_cap.hw_sup_flags & HW_SUP_DBCC) && (phl_com->dev_cap.dbcc_sup)) {
			/*TODO - info core layer */
		}
	}
	#endif
_exit:
	PHL_DUMP_MR_EX(phl_info);
	return is_ch_grouped;
}

enum rtw_phl_status
rtw_phl_chanctx_del_no_self(void *phl, struct rtw_wifi_role_t *wifi_role)
{
	enum rtw_phl_status phl_sts = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	void *drv = phl_to_drvpriv(phl_info);
	struct mr_ctl_t *mr_ctl = phlcom_to_mr_ctrl(phl_com);
	struct hw_band_ctl_t *band_ctrl = &(mr_ctl->band_ctrl[wifi_role->hw_band]);
	struct rtw_chan_ctx *chanctx = NULL;
	int chctx_num = 0;

	chctx_num = phl_mr_get_chanctx_num(phl_info, band_ctrl);
	if (chctx_num > 2) {
		PHL_ERR("%s ERR - chanctx_num(%d) > 2\n", __func__, chctx_num);
		_os_warn_on(1);
		goto _exit;
	}

	if (chctx_num == 0) {
		phl_sts = RTW_PHL_STATUS_SUCCESS;
		PHL_INFO("%s - chctx_num = 0\n", __func__);
		goto _exit;
	}
	else if (chctx_num == 1) { /*SCC*/
		_os_spinlock(drv, &band_ctrl->chan_ctx_queue.lock, _ps, NULL);
		if (!list_empty(&band_ctrl->chan_ctx_queue.queue)) {
			chanctx = list_first_entry(&band_ctrl->chan_ctx_queue.queue,
							struct rtw_chan_ctx, list);
			phl_sts = _phl_chanctx_del(phl_info, &band_ctrl->chan_ctx_queue, chanctx);
			if (phl_sts != RTW_PHL_STATUS_SUCCESS) {
				PHL_ERR("_phl_chanctx_del failed\n");
				_os_warn_on(1);
			}
		}
		_os_spinunlock(drv, &band_ctrl->chan_ctx_queue.lock, _ps, NULL);
		_os_kmem_free(drv, chanctx, sizeof(struct rtw_chan_ctx));
	}
	else if (chctx_num == 2) { /*MCC*/
	}

_exit:
	PHL_DUMP_MR_EX(phl_info);
	return phl_sts;
}

int rtw_phl_chanctx_del(void *phl, struct rtw_wifi_role_t *wifi_role,
						struct rtw_chan_def *chan_def)
{
	enum rtw_phl_status phl_sts = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	void *drv = phl_to_drvpriv(phl_info);
	struct mr_ctl_t *mr_ctl = phlcom_to_mr_ctrl(phl_com);
	struct hw_band_ctl_t *band_ctrl = &(mr_ctl->band_ctrl[wifi_role->hw_band]);
	struct phl_queue *chan_ctx_queue = &band_ctrl->chan_ctx_queue;
	struct rtw_chan_ctx *target_chanctx = NULL;
	struct rtw_chan_ctx *chanctx = NULL;
	int chctx_num = 0;
	u8 chctx_role_num = 0;
	u8 band_role_num = 0;

	if (wifi_role == NULL) {
		PHL_ERR("%s wifi_role == NULL!!\n", __func__);
		/*_os_warn_on(1);*/
		goto _exit;
	}

	target_chanctx = wifi_role->chanctx;
	if (target_chanctx == NULL) {
		PHL_ERR("%s wifi_role->chanctx == NULL\n", __func__);
		/*_os_warn_on(1);*/
		goto _exit;
	}
	/*init chan_def*/
	if (chan_def)
		chan_def->chan = 0;

	chctx_num = phl_mr_get_chanctx_num(phl_info, band_ctrl);
	band_role_num = phl_mr_get_role_num(phl_info, band_ctrl);

	chctx_role_num = phl_chanctx_get_rnum_with_lock(phl_info, chan_ctx_queue, target_chanctx);

	if (chctx_num == 0 || chctx_role_num == 0) {
		PHL_ERR("%s ERR - chanctx_num(%d), role_num(%d)\n", __func__, chctx_num, chctx_role_num);
		_os_warn_on(1);
		goto _exit;
	}
	if (chctx_num > 2) {
		PHL_ERR("%s ERR - chanctx_num(%d) > 2\n", __func__, chctx_num);
		_os_warn_on(1);
		goto _exit;
	}

	if (chctx_role_num == 1) { /*single role on this chctx*/
		_os_spinlock(drv, &chan_ctx_queue->lock, _ps, NULL);
		phl_sts = _phl_chanctx_rmap_clr(phl_info, wifi_role,
						chan_ctx_queue, target_chanctx);
		if (phl_sts != RTW_PHL_STATUS_SUCCESS)
			PHL_ERR("_phl_chanctx_rmap_clr failed\n");

		phl_sts = _phl_chanctx_del(phl_info, chan_ctx_queue, target_chanctx);
		if (phl_sts != RTW_PHL_STATUS_SUCCESS)
			PHL_ERR("_phl_chanctx_del failed\n");
		_os_spinunlock(drv, &chan_ctx_queue->lock, _ps, NULL);

		_os_kmem_free(drv, target_chanctx, sizeof(struct rtw_chan_ctx));

	} else { /*multi roles on this chctx*/
		phl_sts = _phl_chanctx_rmap_clr_with_lock(phl_info, wifi_role,
						chan_ctx_queue, target_chanctx);
		if (phl_sts != RTW_PHL_STATUS_SUCCESS)
			PHL_ERR("_phl_chanctx_rmap_clr_with_lock failed\n");

		phl_sts = phl_mr_chandef_upt(phl_info, band_ctrl, target_chanctx);
		if (phl_sts != RTW_PHL_STATUS_SUCCESS) {
			PHL_ERR("phl_mr_chandef_upt failed\n");
			_os_warn_on(1);
			goto _exit;
		}
	}

	chctx_num = phl_mr_get_chanctx_num(phl_info, band_ctrl);
	if (chctx_num == 0) {
		band_ctrl->op_mode = MR_OP_NON;
	}
	else if (chctx_num == 1) {
		_os_spinlock(drv, &chan_ctx_queue->lock, _ps, NULL);
		chanctx = list_first_entry(&chan_ctx_queue->queue,
						struct rtw_chan_ctx, list);
		chctx_role_num = phl_chanctx_get_rnum(phl_info, chan_ctx_queue, chanctx);
		if (chan_def)
			_os_mem_cpy(drv, chan_def, &chanctx->chan_def, sizeof(struct rtw_chan_def));
		_os_spinunlock(drv, &chan_ctx_queue->lock, _ps, NULL);

		#ifdef DBG_PHL_MR
		if (chctx_role_num == 0) {
			PHL_ERR("chctx_num=1, chctx_role_num=0\n");
			_os_warn_on(1);
		}
		#endif
		band_ctrl->op_mode = (chctx_role_num == 1) ? MR_OP_NON : MR_OP_SCC;
	} else if (chctx_num == 2) {
		if (chan_def)
			_os_mem_cpy(drv, chan_def, &target_chanctx->chan_def, sizeof(struct rtw_chan_def));
		band_ctrl->op_mode = MR_OP_MCC;
	}

	phl_sts = RTW_PHL_STATUS_SUCCESS;
	PHL_INFO("%s - Bidx(%d) - Total role_num:%d, chctx_num:%d, target-chctx rnum:%d, op_mode:%d\n",
		__func__, band_ctrl->id, band_role_num, chctx_num, chctx_role_num, band_ctrl->op_mode);

_exit:
	PHL_DUMP_MR_EX(phl_info);
	return chctx_num;
}

#ifdef	PHL_MR_PROC_CMD
bool rtw_phl_chanctx_test(void *phl, struct rtw_wifi_role_t *wifi_role, bool is_add,
		u8 *chan, enum channel_width *bw, enum chan_offset *offset)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	bool rst = true;
	int chanctx_num = 0;
	struct rtw_chan_def chan_def = {0};

	if (is_add) {
		rst = rtw_phl_chanctx_add(phl, wifi_role, chan, bw, offset);
	}
	else {
		chanctx_num = rtw_phl_chanctx_del(phl, wifi_role, &chan_def);
		PHL_ERR("%s chctx_num = %d\n", __func__, chanctx_num);
		PHL_DUMP_CHAN_DEF(&chan_def);
	}
	return rst;
}
#endif

u8 rtw_phl_get_center_ch(u8 ch,
			enum channel_width bw, enum chan_offset offset)
{
	u8 cch = ch;

	if (bw == CHANNEL_WIDTH_160) {
		if (ch % 4 == 0) {
			if (ch >= 36 && ch <= 64)
				cch = 50;
			else if (ch >= 100 && ch <= 128)
				cch = 114;
		} else if (ch % 4 == 1) {
			if (ch >= 149 && ch <= 177)
				cch = 163;
		}

	} else if (bw == CHANNEL_WIDTH_80) {
		if (ch <= 14)
			cch = 7; /* special case for 2.4G */
		else if (ch % 4 == 0) {
			if (ch >= 36 && ch <= 48)
				cch = 42;
			else if (ch >= 52 && ch <= 64)
				cch = 58;
			else if (ch >= 100 && ch <= 112)
				cch = 106;
			else if (ch >= 116 && ch <= 128)
				cch = 122;
			else if (ch >= 132 && ch <= 144)
				cch = 138;
		} else if (ch % 4 == 1) {
			if (ch >= 149 && ch <= 161)
				cch = 155;
			else if (ch >= 165 && ch <= 177)
				cch = 171;
		}

	} else if (bw == CHANNEL_WIDTH_40) {
		if (offset == CHAN_OFFSET_UPPER)
			cch = ch + 2;
		else if (offset == CHAN_OFFSET_LOWER)
			cch = ch - 2;

	} else if (bw == CHANNEL_WIDTH_20
		|| bw == CHANNEL_WIDTH_10
		|| bw == CHANNEL_WIDTH_5) {
		; /* the same as ch */
	}
	else {
		PHL_ERR("%s failed\n", __func__);
	}
	return cch;
}

#endif // if 0 NEO
