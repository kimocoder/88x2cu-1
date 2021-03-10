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
#define _PHL_TX_C_
#include "phl_headers.h"

/**
 * this function will be used in read / write pointer mechanism and
 * return the number of available read pointer
 * @rptr: input, the read pointer
 * @wptr: input, the write pointer
 * @bndy: input, the boundary of read / write pointer mechanism
 */
u16 phl_calc_avail_rptr(u16 rptr, u16 wptr, u16 bndy)
{
	u16 avail_rptr = 0;

	if (wptr >= rptr)
		avail_rptr = wptr - rptr;
	else if (rptr > wptr)
		avail_rptr = wptr + (bndy - rptr);

	return avail_rptr;
}


/**
 * this function will be used in read / write pointer mechanism and
 * return the number of available write pointer
 * @rptr: input, the read pointer
 * @wptr: input, the write pointer
 * @bndy: input, the boundary of read / write pointer mechanism
 */
u16 phl_calc_avail_wptr(u16 rptr, u16 wptr, u16 bndy)
{
	u16 avail_wptr = 0;

	if (rptr > wptr)
		avail_wptr = rptr - wptr - 1;
	else if (wptr >= rptr)
		avail_wptr = rptr + (bndy - wptr) - 1;

	return avail_wptr;
}

void phl_dump_sorted_ring(_os_list *sorted_ring)
{
	struct phl_ring_status *ring_sts;
	u16 i = 0;

	PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_, "==dump sorted ring==\n");

	phl_list_for_loop(ring_sts, struct phl_ring_status, sorted_ring,
			       list) {
		i++;
		PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_, "==ring %d==\n", i);
		PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_, "ring_sts->macid = %d\n",
			  ring_sts->macid);
		PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_, "ring_sts->req_busy = %d\n",
			  ring_sts->req_busy);
		PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_, "ring_sts->ring_ptr->tid = %d\n",
			  ring_sts->ring_ptr->tid);
	}
}

void phl_dump_tx_plan(_os_list *sta_list)
{
	struct phl_tx_plan *tx_plan;
	u16 i = 0;

	PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_, "==dump tx plan==\n");

	phl_list_for_loop(tx_plan, struct phl_tx_plan, sta_list,
			       list) {
		i++;
		PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_, "==tx plan %d==\n", i);
		PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_, "tx_plan->sleep = %d\n",
			  tx_plan->sleep);
		PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_, "tx_plan->has_mgnt = %d\n",
			  tx_plan->has_mgnt);
		PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_, "tx_plan->has_hiq = %d\n",
			  tx_plan->has_hiq);
		phl_dump_sorted_ring(&tx_plan->sorted_ring);
	}
}

void phl_dump_t_fctrl_result(_os_list *t_fctrl_result)
{
	struct phl_ring_status *ring_sts;
	u16 i = 0;

	PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_, "==dump tx flow control result==\n");

	phl_list_for_loop(ring_sts, struct phl_ring_status, t_fctrl_result,
						   list) {
		i++;
		PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_, "==ring %d==\n", i);
		PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_, "ring_sts->macid = %d\n",
			  ring_sts->macid);
		PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_, "ring_sts->req_busy = %d\n",
			  ring_sts->req_busy);
		PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_, "ring_sts->ring_ptr->tid = %d\n",
			  ring_sts->ring_ptr->tid);
	}
}

void phl_dump_tx_stats(struct rtw_stats *stats)
{
	PHL_TRACE(COMP_PHL_XMIT, _PHL_DEBUG_,
		  "Dump Tx statistics\n"
		  "tx_byte_uni = %lld\n"
		  "tx_byte_total = %lld\n"
		  "tx_tp_kbits = %d\n"
		  "last_tx_time_ms = %d\n",
		  stats->tx_byte_uni,
		  stats->tx_byte_total,
		  stats->tx_tp_kbits,
		  stats->last_tx_time_ms);
}

void phl_dump_h2c_pool_stats(struct phl_h2c_pkt_pool *h2c_pkt_pool)
{
	PHL_INFO("[h2c_stats] idle cmd %d, idle data %d, idle ldata %d, busy h2c %d.\n",
				h2c_pkt_pool->idle_h2c_pkt_cmd_list.cnt,
				h2c_pkt_pool->idle_h2c_pkt_data_list.cnt,
				h2c_pkt_pool->idle_h2c_pkt_ldata_list.cnt,
				h2c_pkt_pool->busy_h2c_pkt_list.cnt);
}

void phl_reset_tx_stats(struct rtw_stats *stats)
{
	stats->tx_byte_uni = 0;
	stats->tx_byte_total = 0;
	stats->tx_tp_kbits = 0;
	stats->last_tx_time_ms = 0;
	stats->txtp.last_calc_time_ms = 0;
	stats->txtp.last_calc_time_ms = 0;
	stats->tx_traffic.lvl = RTW_TFC_IDLE;
	stats->tx_traffic.sts = 0;
}

void
phl_tx_traffic_upd(struct rtw_stats *sts)
{
	u32 tp_k = 0, tp_m = 0;
	enum rtw_tfc_lvl tx_tfc_lvl = RTW_TFC_IDLE;
	tp_k = sts->tx_tp_kbits;
	tp_m = sts->tx_tp_kbits >> 10;

	if (tp_m >= TX_HIGH_TP_THRES_MBPS)
		tx_tfc_lvl = RTW_TFC_HIGH;
	else if (tp_m >= TX_MID_TP_THRES_MBPS)
		tx_tfc_lvl = RTW_TFC_MID;
	else if (tp_m >= TX_LOW_TP_THRES_MBPS)
		tx_tfc_lvl = RTW_TFC_LOW;
	else if (tp_k >= TX_ULTRA_LOW_TP_THRES_KBPS)
		tx_tfc_lvl = RTW_TFC_ULTRA_LOW;
	else
		tx_tfc_lvl = RTW_TFC_IDLE;

	if (sts->tx_traffic.lvl > tx_tfc_lvl) {
		sts->tx_traffic.sts = (TRAFFIC_CHANGED | TRAFFIC_DECREASE);
		sts->tx_traffic.lvl = tx_tfc_lvl;
	} else if (sts->tx_traffic.lvl < tx_tfc_lvl) {
		sts->tx_traffic.sts = (TRAFFIC_CHANGED | TRAFFIC_INCREASE);
		sts->tx_traffic.lvl = tx_tfc_lvl;
	} else if (sts->tx_traffic.sts &
		(TRAFFIC_CHANGED | TRAFFIC_INCREASE | TRAFFIC_DECREASE)) {
		sts->tx_traffic.sts &= ~(TRAFFIC_CHANGED | TRAFFIC_INCREASE |
					 TRAFFIC_DECREASE);
	}
}

void phl_update_tx_stats(struct rtw_stats *stats, struct rtw_xmit_req *tx_req)
{
	u32 diff_t = 0, cur_time = _os_get_cur_time_ms();
	u64 diff_bits = 0;

	stats->last_tx_time_ms = cur_time;
	stats->tx_byte_total += tx_req->total_len;

	stats->txreq_num++;
	if (tx_req->mdata.bc == 0 && tx_req->mdata.mc == 0)
		stats->tx_byte_uni += tx_req->total_len;

	if (0 == stats->txtp.last_calc_time_ms ||
	    0 == stats->txtp.last_calc_bits) {
		stats->txtp.last_calc_time_ms = stats->last_tx_time_ms;
		stats->txtp.last_calc_bits = stats->tx_byte_uni * 8;
	} else {
		if (cur_time >= stats->txtp.last_calc_time_ms) {
			diff_t = cur_time - stats->txtp.last_calc_time_ms;
		} else {
			diff_t = RTW_U32_MAX - stats->txtp.last_calc_time_ms +
				cur_time + 1;
		}
		if (diff_t > TXTP_CALC_DIFF_MS && stats->tx_byte_uni != 0) {
			diff_bits = (stats->tx_byte_uni * 8) -
				stats->txtp.last_calc_bits;
			stats->tx_tp_kbits = (u32)_os_division64(diff_bits,
								 diff_t);
			stats->txtp.last_calc_bits = stats->tx_byte_uni * 8;
			stats->txtp.last_calc_time_ms = cur_time;
		}
	}
}

#if 0 // NEO : mark off first

void phl_tx_statistics(struct phl_info_t *phl_info, struct rtw_xmit_req *tx_req)
{
	struct rtw_phl_com_t *phl_com = phl_info->phl_com;
	struct rtw_stats *phl_stats = &phl_com->phl_stats;
	struct rtw_stats *sta_stats = NULL;
	struct rtw_phl_stainfo_t *sta = NULL;
	u16 macid = tx_req->mdata.macid;

	if (!phl_macid_is_valid(phl_info, macid))
		goto dev_stat;

	sta = rtw_phl_get_stainfo_by_macid(phl_info, macid);

	if (NULL == sta)
		goto dev_stat;
	sta_stats = &sta->stats;

	phl_update_tx_stats(sta_stats, tx_req);
dev_stat:
	phl_update_tx_stats(phl_stats, tx_req);
}

#endif // if 0 NEO 


static void _phl_free_phl_tring_list(void *phl,
				struct rtw_phl_tring_list *ring_list)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct rtw_phl_evt_ops *ops = &phl_info->phl_com->evt_ops;
	struct rtw_phl_tx_ring *ring;
	struct rtw_xmit_req *tx_req;
	u16 rptr = 0;
	u8 i = 0;

	for (i = 0; i < MAX_PHL_RING_CAT_NUM; i++) {
		ring = &ring_list->phl_ring[i];
		rptr = (u16)_os_atomic_read(drv_priv, &ring->phl_idx);

		while (rptr != ring->core_idx) {
			rptr += 1;
			if (rptr >= MAX_PHL_RING_ENTRY_NUM)
				rptr = 0;
			tx_req = (struct rtw_xmit_req *)ring->entry[rptr];
			if (NULL == tx_req)
				break;
			ops->tx_recycle(drv_priv, tx_req);
		}
	}
	_os_kmem_free(drv_priv, ring_list, sizeof(*ring_list));
}


void _phl_init_tx_plan(struct phl_tx_plan * tx_plan)
{
	INIT_LIST_HEAD(&tx_plan->list);
	tx_plan->sleep = false;
	tx_plan->has_mgnt = false;
	tx_plan->has_hiq = false;
	INIT_LIST_HEAD(&tx_plan->sorted_ring);
}


static struct rtw_phl_tring_list *_phl_allocate_phl_tring_list(void *phl,
							u16 macid)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_phl_tring_list *phl_tring_list = NULL;
	void *drv_priv = NULL;
	u32 buf_len = 0;
	u8 i = 0, dma_ch = 0, band = 0;

	drv_priv = phl_to_drvpriv(phl_info);

	buf_len = sizeof(struct rtw_phl_tring_list);
	phl_tring_list = (struct rtw_phl_tring_list *)_os_kmem_alloc(drv_priv,
								buf_len);

	if (NULL != phl_tring_list) {
		_os_mem_set(drv_priv, phl_tring_list, 0, buf_len);
		INIT_LIST_HEAD(&phl_tring_list->list);
		phl_tring_list->macid = macid;

		for (i = 0; i < MAX_PHL_RING_CAT_NUM; i++) {
			phl_tring_list->phl_ring[i].tid = i;
			/* hana_todo, force band to 0 temporarily */
			band = 0;
			// TODO : NEO : tx channel mapping
			RTW_INFO("%s : NEO TODO dma_ch\n", __func__);
			//dma_ch = rtw_hal_tx_chnl_mapping(phl_info->hal, macid,
			//				 i, band);
			phl_tring_list->phl_ring[i].band = band;
			phl_tring_list->phl_ring[i].dma_ch = dma_ch;
		}
		_phl_init_tx_plan(&phl_tring_list->tx_plan);
	}

	return phl_tring_list;
}


enum rtw_phl_status phl_register_tx_ring(void *phl, u16 macid)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct rtw_phl_tring_list *phl_tring_list = NULL;
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	_os_list *ring_list = NULL;

	phl_tring_list = _phl_allocate_phl_tring_list(phl, macid);

	if (NULL != phl_tring_list) {
		ring_list = &phl_info->t_ring_list;
		_os_spinlock(drv_priv, &phl_info->t_ring_list_lock, _bh, NULL);
		list_add_tail(&phl_tring_list->list, ring_list);
		_os_spinunlock(drv_priv, &phl_info->t_ring_list_lock, _bh, NULL);

		phl_status = RTW_PHL_STATUS_SUCCESS;
	}

	return phl_status;
}



enum rtw_phl_status phl_deregister_tx_ring(void *phl, u16 macid)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct rtw_phl_tring_list *phl_tring_list = NULL, *t;
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	_os_list *ring_list = NULL;

	ring_list = &phl_info->t_ring_list;

	_os_spinlock(drv_priv, &phl_info->t_ring_list_lock, _bh, NULL);

	phl_list_for_loop_safe(phl_tring_list, t, struct rtw_phl_tring_list,
					ring_list, list) {
		if (macid == phl_tring_list->macid) {
			list_del(&phl_tring_list->list);
			phl_status = RTW_PHL_STATUS_SUCCESS;
			break;
		}
	}

	_os_spinunlock(drv_priv, &phl_info->t_ring_list_lock, _bh, NULL);

	if (RTW_PHL_STATUS_SUCCESS == phl_status) {
		/* defer the free operation to avoid racing with _phl_tx_callback_xxx */
		_os_spinlock(drv_priv, &phl_info->t_ring_free_list_lock, _bh, NULL);
		list_add_tail(&phl_tring_list->list, &phl_info->t_ring_free_list);
		_os_spinunlock(drv_priv, &phl_info->t_ring_free_list_lock, _bh, NULL);
	}

	return phl_status;
}

void phl_free_deferred_tx_ring(struct phl_info_t *phl_info)
{
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct rtw_phl_tring_list *phl_tring_list = NULL, *t;
	_os_list *ring_list = NULL;

	ring_list = & phl_info->t_ring_free_list;

	_os_spinlock(drv_priv, &phl_info->t_ring_free_list_lock, _bh, NULL);

	if (list_empty(ring_list) == false) {
		phl_list_for_loop_safe(phl_tring_list, t, struct rtw_phl_tring_list,
					ring_list, list) {
		    list_del(&phl_tring_list->list);
		    _phl_free_phl_tring_list(phl_info, phl_tring_list);
		}
	}
	_os_spinunlock(drv_priv, &phl_info->t_ring_free_list_lock, _bh, NULL);
}


struct phl_ring_status *phl_alloc_ring_sts(struct phl_info_t *phl_info)
{
	struct phl_ring_sts_pool *ring_sts_pool = phl_info->ring_sts_pool;
	struct phl_ring_status *ring_sts = NULL;

	_os_spinlock(phl_to_drvpriv(phl_info), &ring_sts_pool->idle_lock, _bh, NULL);

	if (false == list_empty(&ring_sts_pool->idle)) {
		ring_sts = list_first_entry(&ring_sts_pool->idle,
					struct phl_ring_status, list);
		list_del(&ring_sts->list);
	}

	_os_spinunlock(phl_to_drvpriv(phl_info), &ring_sts_pool->idle_lock, _bh, NULL);

	return ring_sts;
}

void phl_release_ring_sts(struct phl_info_t *phl_info,
				struct phl_ring_status *ring_sts)
{
	struct phl_ring_sts_pool *ring_sts_pool = phl_info->ring_sts_pool;
	void *drv_priv = NULL;

	drv_priv = phl_to_drvpriv(phl_info);

	_os_spinlock(drv_priv, &ring_sts_pool->idle_lock, _bh, NULL);
	_os_mem_set(drv_priv, ring_sts, 0, sizeof(*ring_sts));
	INIT_LIST_HEAD(&ring_sts->list);
	list_add_tail(&ring_sts->list, &ring_sts_pool->idle);
	_os_spinunlock(drv_priv, &ring_sts_pool->idle_lock, _bh, NULL);
}


void _phl_ring_status_deinit(struct phl_info_t *phl_info)
{
	struct phl_ring_sts_pool *ring_sts_pool = NULL;
	u16 buf_len = 0;
	void *drv_priv = NULL;
	FUNCIN();
	drv_priv = phl_to_drvpriv(phl_info);
	ring_sts_pool = (struct phl_ring_sts_pool *)phl_info->ring_sts_pool;
	if (NULL != ring_sts_pool) {
		buf_len = sizeof(struct phl_ring_sts_pool);
		_os_spinlock_free(drv_priv, &ring_sts_pool->idle_lock);
		_os_spinlock_free(drv_priv, &ring_sts_pool->busy_lock);
		_os_mem_free(drv_priv, ring_sts_pool, buf_len);
	}
	FUNCOUT();
}


enum rtw_phl_status _phl_ring_status_init(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_ring_sts_pool *ring_sts_pool = NULL;
	struct phl_ring_status *ring_sts = NULL;
	void *drv_priv = NULL;
	u16 buf_len = 0;
	u8 i = 0;
	FUNCIN_WSTS(pstatus);

	drv_priv = phl_to_drvpriv(phl_info);
	buf_len = sizeof(struct phl_ring_sts_pool);
	ring_sts_pool =
			(struct phl_ring_sts_pool *)_os_mem_alloc(drv_priv, buf_len);

	if (NULL != ring_sts_pool) {
		_os_mem_set(drv_priv, ring_sts_pool, 0, buf_len);
		INIT_LIST_HEAD(&ring_sts_pool->idle);
		INIT_LIST_HEAD(&ring_sts_pool->busy);
		_os_spinlock_init(drv_priv, &ring_sts_pool->idle_lock);
		_os_spinlock_init(drv_priv, &ring_sts_pool->busy_lock);

		for (i = 0; i < MAX_PHL_RING_STATUS_NUMBER; i++) {
			ring_sts = &ring_sts_pool->ring_sts[i];
			INIT_LIST_HEAD(&ring_sts->list);
			_os_spinlock(drv_priv,
					(void *)&ring_sts_pool->idle_lock, _bh, NULL);
			list_add_tail(&ring_sts->list, &ring_sts_pool->idle);
			_os_spinunlock(drv_priv,
					(void *)&ring_sts_pool->idle_lock, _bh, NULL);
		}

		phl_info->ring_sts_pool = ring_sts_pool;

		pstatus = RTW_PHL_STATUS_SUCCESS;
	}

	if (RTW_PHL_STATUS_SUCCESS != pstatus)
		_phl_ring_status_deinit(phl_info);
	FUNCOUT_WSTS(pstatus);

	return pstatus;
}

struct phl_ring_status *_phl_check_ring_status(struct phl_info_t *phl_info,
					struct rtw_phl_tx_ring *ring,
					u16 macid)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_ring_status *ring_sts = NULL;
	u16 avail = 0, rptr = 0;
	void *drv_priv = phl_to_drvpriv(phl_info);

	do {
		rptr = (u16)_os_atomic_read(drv_priv, &ring->phl_idx);

		avail = phl_calc_avail_rptr(rptr, ring->core_idx,
					MAX_PHL_RING_ENTRY_NUM);
		if (0 == avail) {
			ring_sts = NULL;
			pstatus = RTW_PHL_STATUS_SUCCESS;
			break;
		} else {
			ring_sts = phl_alloc_ring_sts(phl_info);
			if (NULL == ring_sts) {
				PHL_ERR("query ring status fail!\n");
				pstatus = RTW_PHL_STATUS_RESOURCE;
				break;
			}
			ring_sts->macid = macid;
			ring_sts->req_busy = avail;
			ring_sts->ring_ptr = ring;

			rptr += 1;

			if (rptr >= MAX_PHL_RING_ENTRY_NUM)
				_os_atomic_set(drv_priv, &ring->phl_next_idx, 0);
			else
				_os_atomic_set(drv_priv, &ring->phl_next_idx, rptr);

			pstatus = RTW_PHL_STATUS_SUCCESS;
			break;
		}
	} while (false);

	return ring_sts;
}

void _phl_reset_tx_plan(struct phl_info_t *phl_info,
			 struct phl_tx_plan *tx_plan)
{
	struct phl_ring_status *ring_sts, *t;

	INIT_LIST_HEAD(&tx_plan->list);
        tx_plan->sleep = false;
	tx_plan->has_mgnt = false;
	tx_plan->has_hiq = false;
	phl_list_for_loop_safe(ring_sts, t, struct phl_ring_status,
			       &tx_plan->sorted_ring, list) {
		list_del(&ring_sts->list);
		phl_release_ring_sts(phl_info, ring_sts);
	}
	INIT_LIST_HEAD(&tx_plan->sorted_ring);
}


void _phl_sort_ring_by_tid(struct phl_ring_status *ring_sts,
			   struct phl_tx_plan *tx_plan,
			   enum rtw_phl_ring_cat cat)
{
	struct phl_ring_status *last_sts = NULL;

	if (ring_sts->ring_ptr->tid == 1) {
		list_add_tail(&ring_sts->list,
			      &tx_plan->sorted_ring);
	} else if (ring_sts->ring_ptr->tid == 2) {
	    if (list_empty(&tx_plan->sorted_ring)) {
		list_add_tail(&ring_sts->list,
			      &tx_plan->sorted_ring);
	    } else {
		last_sts = list_last_entry(&tx_plan->sorted_ring,
				struct phl_ring_status, list);
		if (1 == last_sts->ring_ptr->tid) {
			__list_add(&ring_sts->list,
				   _get_prev(&last_sts->list),
				   &last_sts->list);
		} else {
			list_add_tail(&ring_sts->list,
				      &tx_plan->sorted_ring);
		}
	    }
	} else {
		list_add(&ring_sts->list,
			 &tx_plan->sorted_ring);
		if (RTW_PHL_RING_CAT_MGNT == cat)
			tx_plan->has_mgnt = true;
		else if (RTW_PHL_RING_CAT_HIQ == cat)
			tx_plan->has_hiq = true;
	}

}


void _phl_check_tring_list(struct phl_info_t *phl_info,
			   struct rtw_phl_tring_list *tring_list,
			   _os_list *sta_list)
{
	struct phl_ring_status *ring_sts = NULL;
	struct rtw_phl_tx_ring *ring = NULL;
	struct phl_tx_plan *tx_plan = &tring_list->tx_plan;
	u8 i = 0;

	for (i = 0; i < MAX_PHL_RING_CAT_NUM; i++) {

		ring = &tring_list->phl_ring[i];

		ring_sts = _phl_check_ring_status(phl_info, ring,
						 tring_list->macid);

		if (NULL != ring_sts) {
			_phl_sort_ring_by_tid(ring_sts, tx_plan, i);
		} else {
			continue;
		}
	}
	/* hana_todo: check this macid is sleep or not */
	if (!list_empty(&tx_plan->sorted_ring)) {
		list_add_tail(&tx_plan->list, sta_list);
	}
}

u8 phl_check_xmit_ring_resource(struct phl_info_t *phl_info, _os_list *sta_list)
{
	void *drvpriv = phl_to_drvpriv(phl_info);
	_os_list *tring_list_head = &phl_info->t_ring_list;
	struct rtw_phl_tring_list *tring_list, *t;

	_os_spinlock(drvpriv, &phl_info->t_ring_list_lock, _bh, NULL);

	phl_list_for_loop_safe(tring_list, t, struct rtw_phl_tring_list,
				tring_list_head, list) {
		_phl_check_tring_list(phl_info, tring_list, sta_list);
	}

	_os_spinunlock(drvpriv, &phl_info->t_ring_list_lock, _bh, NULL);

	if (true == list_empty(sta_list))
		return false;
	else
		return true;
}

void phl_tx_flow_ctrl(struct phl_info_t *phl_info, _os_list *sta_list)
{
	_os_list *t_fctrl_result = &phl_info->t_fctrl_result;
	_os_list *tid_entry[MAX_PHL_RING_CAT_NUM] = {0};
	struct phl_tx_plan *tx_plan, *tp;
	struct phl_ring_status *ring_sts = NULL, *ts;
	u8 tid = 0;

	_os_mem_set(phl_to_drvpriv(phl_info), tid_entry, 0,
		    sizeof(_os_list *) * MAX_PHL_RING_CAT_NUM);

	phl_list_for_loop_safe(tx_plan, tp, struct phl_tx_plan, sta_list,
			       list) {
		/* drop power saving station */
		if (true == tx_plan->sleep) {
			list_del(&tx_plan->list);
			_phl_reset_tx_plan(phl_info, tx_plan);
			continue;
		}

		if (true == tx_plan->has_hiq) {
			ring_sts = list_first_entry(&tx_plan->sorted_ring,
						struct phl_ring_status, list);
			list_del(&ring_sts->list);
			list_add(&ring_sts->list, t_fctrl_result);
		}

		if (true == tx_plan->has_mgnt) {
			ring_sts = list_first_entry(&tx_plan->sorted_ring,
						struct phl_ring_status, list);
			list_del(&ring_sts->list);
			list_add(&ring_sts->list, t_fctrl_result);
		}

		/* todo: drop station which has reached tx limit */

		phl_list_for_loop_safe(ring_sts, ts, struct phl_ring_status,
				       &tx_plan->sorted_ring, list) {
			list_del(&ring_sts->list);
			tid = ring_sts->ring_ptr->tid;
			/* todo: drop tid which has reached tx limit */

			if (NULL == tid_entry[tid]) {
				list_add_tail(&ring_sts->list, t_fctrl_result);
			} else {
				__list_add(&ring_sts->list, tid_entry[tid],
					   _get_next(tid_entry[tid]));
			}
			tid_entry[tid] = &ring_sts->list;
		}

		/* clear tx plan */
		list_del(&tx_plan->list);
		_phl_reset_tx_plan(phl_info, tx_plan);
	}
}

enum rtw_phl_status phl_register_handler(struct rtw_phl_com_t *phl_com,
					 struct rtw_phl_handler *handler)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	_os_tasklet *tasklet = NULL;
	_os_workitem *workitem = NULL;
	void *drv_priv = phlcom_to_drvpriv(phl_com);

	FUNCIN_WSTS(phl_status);

	if (handler->type == RTW_PHL_HANDLER_PRIO_HIGH) {
		tasklet = &handler->os_handler.u.tasklet;
		phl_status = _os_tasklet_init(drv_priv, tasklet,
										handler->callback, handler);
	} else if (handler->type == RTW_PHL_HANDLER_PRIO_LOW) {
		workitem = &handler->os_handler.u.workitem;
		phl_status = _os_workitem_init(drv_priv, workitem,
										handler->callback, workitem);
	} else {
		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "[WARNING] unknown handle type(%d)\n",
				handler->type);
	}

	if (RTW_PHL_STATUS_SUCCESS != phl_status)
		phl_deregister_handler(phl_com, handler);

	FUNCOUT_WSTS(phl_status);
	return phl_status;
}

enum rtw_phl_status phl_deregister_handler(
	struct rtw_phl_com_t *phl_com, struct rtw_phl_handler *handler)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	_os_tasklet *tasklet = NULL;
	_os_workitem *workitem = NULL;
	void *drv_priv = phlcom_to_drvpriv(phl_com);

	FUNCIN_WSTS(phl_status);

	if (handler->type == RTW_PHL_HANDLER_PRIO_HIGH) {
		tasklet = &handler->os_handler.u.tasklet;
		phl_status = _os_tasklet_deinit(drv_priv, tasklet);
	} else if (handler->type == RTW_PHL_HANDLER_PRIO_LOW) {
		workitem = &handler->os_handler.u.workitem;
		phl_status = _os_workitem_deinit(drv_priv, workitem);
	} else {
		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "[WARNING] unknown handle type(%d)\n",
				handler->type);
	}

	if (RTW_PHL_STATUS_SUCCESS != phl_status) {
		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_,
			"[WARNING] deregister handler fail (status = 0x%08X)\n",
			phl_status);
	}

	FUNCOUT_WSTS(phl_status);
	return phl_status;
}

enum rtw_phl_status phl_schedule_handler(
	struct rtw_phl_com_t *phl_com, struct rtw_phl_handler *handler)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;
	_os_tasklet *tasklet = NULL;
	_os_workitem *workitem = NULL;
	void *drv_priv = phlcom_to_drvpriv(phl_com);

	FUNCIN_WSTS(phl_status);

	if (handler->type == RTW_PHL_HANDLER_PRIO_HIGH) {
		tasklet = &handler->os_handler.u.tasklet;
		phl_status = _os_tasklet_schedule(drv_priv, tasklet);
	} else if (handler->type == RTW_PHL_HANDLER_PRIO_LOW) {
		workitem = &handler->os_handler.u.workitem;
		phl_status = _os_workitem_schedule(drv_priv, workitem);
	} else {
		PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "[WARNING] unknown handle type(%d)\n",
				handler->type);
	}

	FUNCOUT_WSTS(phl_status);
	return phl_status;
}

static enum rtw_phl_status enqueue_h2c_pkt(
					struct phl_info_t *phl_info,
					struct phl_queue	*pool_list,
					struct rtw_h2c_pkt *h2c_pkt, u8 pos)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	void *drv = phl_to_drvpriv(phl_info);
	_os_spinlockfg sp_flags;


	if (h2c_pkt != NULL) {
		_os_spinlock(drv, &pool_list->lock, _irq, &sp_flags);
		if (_tail == pos)
			list_add_tail(&h2c_pkt->list, &pool_list->queue);
		else if (_first == pos)
			list_add(&h2c_pkt->list, &pool_list->queue);
		pool_list->cnt++;
		_os_spinunlock(drv, &pool_list->lock, _irq, &sp_flags);

		pstatus = RTW_PHL_STATUS_SUCCESS;
	}

	return pstatus;
}

static struct rtw_h2c_pkt *dequeue_h2c_pkt(struct phl_info_t *phl_info,
	struct phl_queue *pool_list)
{
	struct rtw_h2c_pkt *h2c_pkt = NULL;
	void *drv = phl_to_drvpriv(phl_info);
	_os_spinlockfg sp_flags;

	_os_spinlock(drv, &pool_list->lock, _irq, &sp_flags);
	if (list_empty(&pool_list->queue)) {
		h2c_pkt = NULL;
	} else {
		h2c_pkt = list_first_entry(&pool_list->queue, struct rtw_h2c_pkt, list);

		list_del(&h2c_pkt->list);
		pool_list->cnt--;
	}
	_os_spinunlock(drv, &pool_list->lock, _irq, &sp_flags);
	return h2c_pkt;
}

static void _phl_reset_h2c_pkt(struct phl_info_t *phl_info,
							struct rtw_h2c_pkt *h2c_pkt,
							u32 buf_len)
{
	enum rtw_h2c_pkt_type type = h2c_pkt->type;

	_os_mem_set(phl_to_drvpriv(phl_info), h2c_pkt->vir_head, 0, buf_len);
	h2c_pkt->buf_len = buf_len;
	h2c_pkt->id = 0;
	h2c_pkt->host_idx = 0;
	h2c_pkt->data_len = 0;
	h2c_pkt->h2c_seq = 0;

	switch (type) {
	case H2CB_TYPE_CMD:
		h2c_pkt->vir_data = h2c_pkt->vir_head + FWCMD_HDR_LEN + _WD_BODY_LEN;
		h2c_pkt->vir_tail = h2c_pkt->vir_data;
		h2c_pkt->vir_end = h2c_pkt->vir_data + H2C_CMD_LEN;
		break;
	case H2CB_TYPE_DATA:
		h2c_pkt->vir_data = h2c_pkt->vir_head + FWCMD_HDR_LEN + _WD_BODY_LEN;
		h2c_pkt->vir_tail = h2c_pkt->vir_data;
		h2c_pkt->vir_end = h2c_pkt->vir_data + H2C_DATA_LEN;
		break;
	case H2CB_TYPE_LONG_DATA:
		h2c_pkt->vir_data = h2c_pkt->vir_head + FWCMD_HDR_LEN + _WD_BODY_LEN;
		h2c_pkt->vir_tail = h2c_pkt->vir_data;
		h2c_pkt->vir_end = h2c_pkt->vir_data + H2C_LONG_DATA_LEN;
		break;
	case H2CB_TYPE_MAX:
		PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "_phl_reset_h2c_pkt(): Unsupported case:%d, please check it\n",
				type);
		break;
	default:
		PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "_phl_reset_h2c_pkt(): Unrecognize case:%d, please check it\n",
				type);
		break;
	}

}

enum rtw_phl_status phl_enqueue_busy_h2c_pkt(struct phl_info_t *phl_info,
				struct rtw_h2c_pkt *h2c_pkt, u8 pos)
{
	struct phl_h2c_pkt_pool *h2c_pkt_pool =
		(struct phl_h2c_pkt_pool *)phl_info->h2c_pool;
	struct phl_queue *queue = &h2c_pkt_pool->busy_h2c_pkt_list;

	return enqueue_h2c_pkt(phl_info, queue, h2c_pkt, pos);
}

enum rtw_phl_status phl_enqueue_idle_h2c_pkt(
				struct phl_info_t *phl_info,
				struct rtw_h2c_pkt *h2c_pkt)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_h2c_pkt_pool *h2c_pkt_pool =
		(struct phl_h2c_pkt_pool *)phl_info->h2c_pool;
	struct phl_queue *queue = NULL;
	int *idle_cnt = NULL;
	u32 buf_len = 0;

	if (!h2c_pkt)
		return pstatus;

	switch (h2c_pkt->type) {
	case H2CB_TYPE_CMD:
		buf_len = FWCMD_HDR_LEN + _WD_BODY_LEN + H2C_CMD_LEN;
		queue = &h2c_pkt_pool->idle_h2c_pkt_cmd_list;
		idle_cnt = &h2c_pkt_pool->idle_h2c_pkt_cmd_list.cnt;
		break;
	case H2CB_TYPE_DATA:
		buf_len = FWCMD_HDR_LEN + _WD_BODY_LEN + H2C_DATA_LEN;
		queue = &h2c_pkt_pool->idle_h2c_pkt_data_list;
		idle_cnt = &h2c_pkt_pool->idle_h2c_pkt_data_list.cnt;
		break;
	case H2CB_TYPE_LONG_DATA:
		buf_len = FWCMD_HDR_LEN + _WD_BODY_LEN + H2C_LONG_DATA_LEN;
		queue = &h2c_pkt_pool->idle_h2c_pkt_ldata_list;
		idle_cnt = &h2c_pkt_pool->idle_h2c_pkt_ldata_list.cnt;
		break;
	case H2CB_TYPE_MAX:
		PHL_ERR("%s : cannot find the matching case(%d).\n",
				__func__, h2c_pkt->type);
		break;
	default:
		PHL_ERR("%s : cannot find the matching cases(%d).\n",
				__func__, h2c_pkt->type);
		break;
	}

	_phl_reset_h2c_pkt(phl_info, h2c_pkt, buf_len);

	pstatus = enqueue_h2c_pkt(phl_info, queue, h2c_pkt, _tail);

	PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "%s : remaining %d (type %d).\n",
			  __func__, *idle_cnt, h2c_pkt->type);

	return pstatus;
}

struct rtw_h2c_pkt *phl_query_busy_h2c_pkt(struct phl_info_t *phl_info)
{
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_h2c_pkt_pool *h2c_pkt_pool = NULL;
	struct rtw_h2c_pkt *h2c_pkt = NULL;
	struct phl_queue *queue = NULL;

	h2c_pkt_pool = (struct phl_h2c_pkt_pool *)phl_info->h2c_pool;
	queue = &h2c_pkt_pool->busy_h2c_pkt_list;

	h2c_pkt = dequeue_h2c_pkt(phl_info, queue);

	return h2c_pkt;
}

struct rtw_h2c_pkt *phl_query_idle_h2c_pkt(struct phl_info_t *phl_info, u8 type)
{
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_h2c_pkt_pool *h2c_pkt_pool = NULL;
	struct rtw_h2c_pkt *h2c_pkt = NULL;
	enum rtw_h2c_pkt_type h2c_type = (enum rtw_h2c_pkt_type)type;
	struct phl_queue *queue = NULL;
	int *idle_cnt = NULL;

	h2c_pkt_pool = (struct phl_h2c_pkt_pool *)phl_info->h2c_pool;

	switch (h2c_type) {
	case H2CB_TYPE_CMD:
		queue = &h2c_pkt_pool->idle_h2c_pkt_cmd_list;
		idle_cnt = &h2c_pkt_pool->idle_h2c_pkt_cmd_list.cnt;
		break;
	case H2CB_TYPE_DATA:
		queue = &h2c_pkt_pool->idle_h2c_pkt_data_list;
		idle_cnt = &h2c_pkt_pool->idle_h2c_pkt_data_list.cnt;
		break;
	case H2CB_TYPE_LONG_DATA:
		queue = &h2c_pkt_pool->idle_h2c_pkt_ldata_list;
		idle_cnt = &h2c_pkt_pool->idle_h2c_pkt_ldata_list.cnt;
		break;
	case H2CB_TYPE_MAX:
		PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "phl_query_idle_h2c_pkt(): Unsupported case:%d, please check it\n",
				h2c_type);
		break;
	default:
		PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_, "phl_query_idle_h2c_pkt(): Unrecognize case:%d, please check it\n",
				h2c_type);
		break;
	}
	PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_,
		  "phl_query_idle_h2c_pkt => remaining %d (type %d).\n",
		  *idle_cnt, h2c_type);

	h2c_pkt = dequeue_h2c_pkt(phl_info, queue);

	return h2c_pkt;
}

#if 0
static enum rtw_phl_status phl_release_target_h2c_pkt(
					struct phl_info_t *phl_info,
					struct phl_h2c_pkt_pool *h2c_pkt_pool,
					struct rtw_h2c_pkt *h2c_pkt)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;

	if (h2c_pkt_pool != NULL && h2c_pkt != NULL) {
		phl_enqueue_idle_h2c_pkt(phl_info, h2c_pkt);
		pstatus = RTW_PHL_STATUS_SUCCESS;
	}

	return pstatus;
}
#endif

static void _phl_free_h2c_pkt(struct phl_info_t *phl_info,
					struct rtw_h2c_pkt *h2c_pkt_buf)
{
	u16 i = 0;
	struct rtw_h2c_pkt *h2c_pkt = h2c_pkt_buf;
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;

	if (NULL != h2c_pkt) {
		for (i = 0; i < MAX_H2C_PKT_NUM; i++) {
			if (NULL == h2c_pkt->vir_head)
				continue;
			hci_trx_ops->free_h2c_pkt_buf(phl_info, h2c_pkt);
			h2c_pkt->vir_head = NULL;
			h2c_pkt->cache = false;
			h2c_pkt++;
		}

		_os_mem_free(phl_to_drvpriv(phl_info), h2c_pkt_buf,
					sizeof(struct rtw_h2c_pkt) * MAX_H2C_PKT_NUM);
		h2c_pkt_buf = NULL;
	}
}

struct rtw_h2c_pkt *_phl_alloc_h2c_pkt(struct phl_info_t *phl_info,
	struct phl_h2c_pkt_pool *h2c_pool)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;
	struct rtw_h2c_pkt *h2c_pkt = NULL;
	struct rtw_h2c_pkt *h2c_pkt_root = NULL;
	struct phl_h2c_pkt_pool *h2c_pkt_pool = h2c_pool;
	u32 buf_len = 0;
	int i;

	buf_len = sizeof(struct rtw_h2c_pkt) * MAX_H2C_PKT_NUM;
	h2c_pkt_root = _os_mem_alloc(phl_to_drvpriv(phl_info), buf_len);
	h2c_pkt = h2c_pkt_root;
	if (h2c_pkt != NULL) {
		for (i = 0; i < MAX_H2C_PKT_NUM; i++) {
			h2c_pkt->cache = false;
			buf_len = get_h2c_size_by_range(i);
			hci_trx_ops->alloc_h2c_pkt_buf(phl_info, h2c_pkt, buf_len);

			if (NULL == h2c_pkt->vir_head) {
				pstatus = RTW_PHL_STATUS_RESOURCE;
				break;
			}

			h2c_pkt->buf_len = buf_len;
			h2c_pkt->vir_data = h2c_pkt->vir_head + FWCMD_HDR_LEN + _WD_BODY_LEN;
			h2c_pkt->vir_tail = h2c_pkt->vir_data;
			INIT_LIST_HEAD(&h2c_pkt->list);
			if (i < _H2CB_CMD_QLEN) {
				h2c_pkt->type = H2CB_TYPE_CMD;
				h2c_pkt->vir_end = h2c_pkt->vir_data + H2C_CMD_LEN;
				enqueue_h2c_pkt(phl_info,
					&h2c_pkt_pool->idle_h2c_pkt_cmd_list, h2c_pkt, _tail);
			} else if (i < _H2CB_CMD_QLEN + _H2CB_DATA_QLEN) {
				h2c_pkt->type = H2CB_TYPE_DATA;
				h2c_pkt->vir_end = h2c_pkt->vir_data + H2C_DATA_LEN;
				enqueue_h2c_pkt(phl_info,
					&h2c_pkt_pool->idle_h2c_pkt_data_list, h2c_pkt, _tail);
			} else {
				h2c_pkt->type = H2CB_TYPE_LONG_DATA;
				h2c_pkt->vir_end = h2c_pkt->vir_data + H2C_LONG_DATA_LEN;
				enqueue_h2c_pkt(phl_info,
					&h2c_pkt_pool->idle_h2c_pkt_ldata_list, h2c_pkt, _tail);
			}
			h2c_pkt++;
			pstatus = RTW_PHL_STATUS_SUCCESS;
		}
	}

	if (RTW_PHL_STATUS_SUCCESS != pstatus) {
		_phl_free_h2c_pkt(phl_info, h2c_pkt_root);
		h2c_pkt_root = NULL;
	}

	return h2c_pkt_root;
}

static void _phl_free_h2c_pool(struct phl_info_t *phl_info)
{
	struct phl_h2c_pkt_pool *h2c_pkt_pool = NULL;
	void *drv_priv = phl_to_drvpriv(phl_info);
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;

	FUNCIN();

	h2c_pkt_pool = phl_info->h2c_pool;
	if (NULL != h2c_pkt_pool) {
		h2c_pkt_pool->idle_h2c_pkt_cmd_list.cnt = 0;
		h2c_pkt_pool->idle_h2c_pkt_data_list.cnt = 0;
		h2c_pkt_pool->idle_h2c_pkt_ldata_list.cnt = 0;

		_phl_free_h2c_pkt(phl_info, h2c_pkt_pool->h2c_pkt_buf);
		h2c_pkt_pool->h2c_pkt_buf = NULL;
		_os_spinlock_free(drv_priv,
					&h2c_pkt_pool->idle_h2c_pkt_cmd_list.lock);
		_os_spinlock_free(drv_priv,
					&h2c_pkt_pool->idle_h2c_pkt_data_list.lock);
		_os_spinlock_free(drv_priv,
					&h2c_pkt_pool->idle_h2c_pkt_ldata_list.lock);
		_os_spinlock_free(drv_priv,
					&h2c_pkt_pool->busy_h2c_pkt_list.lock);
		_os_spinlock_free(drv_priv,
					&h2c_pkt_pool->recycle_lock);
		_os_mem_free(phl_to_drvpriv(phl_info), h2c_pkt_pool,
					sizeof(struct phl_h2c_pkt_pool));
	}
	FUNCOUT();
}

enum rtw_phl_status
_phl_alloc_h2c_pool(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;
	struct phl_h2c_pkt_pool *h2c_pkt_pool = NULL;
	struct rtw_h2c_pkt *h2c_pkt_buf = NULL;
	void *drv_priv = NULL;

	FUNCIN_WSTS(pstatus);
	drv_priv = phl_to_drvpriv(phl_info);

	h2c_pkt_pool = _os_mem_alloc(drv_priv, sizeof(struct phl_h2c_pkt_pool));
	if (NULL != h2c_pkt_pool) {

		INIT_LIST_HEAD(&h2c_pkt_pool->idle_h2c_pkt_cmd_list.queue);
		h2c_pkt_pool->idle_h2c_pkt_cmd_list.cnt = 0;

		INIT_LIST_HEAD(&h2c_pkt_pool->idle_h2c_pkt_data_list.queue);
		h2c_pkt_pool->idle_h2c_pkt_data_list.cnt = 0;

		INIT_LIST_HEAD(&h2c_pkt_pool->idle_h2c_pkt_ldata_list.queue);
		h2c_pkt_pool->idle_h2c_pkt_ldata_list.cnt = 0;

		INIT_LIST_HEAD(&h2c_pkt_pool->busy_h2c_pkt_list.queue);
		h2c_pkt_pool->busy_h2c_pkt_list.cnt = 0;

		_os_spinlock_init(drv_priv,
					&h2c_pkt_pool->idle_h2c_pkt_cmd_list.lock);
		_os_spinlock_init(drv_priv,
					&h2c_pkt_pool->idle_h2c_pkt_data_list.lock);
		_os_spinlock_init(drv_priv,
					&h2c_pkt_pool->idle_h2c_pkt_ldata_list.lock);
		_os_spinlock_init(drv_priv,
					&h2c_pkt_pool->busy_h2c_pkt_list.lock);
		_os_spinlock_init(drv_priv,
					&h2c_pkt_pool->recycle_lock);

		h2c_pkt_buf = _phl_alloc_h2c_pkt(phl_info, h2c_pkt_pool);

		if (NULL == h2c_pkt_buf) {
			_os_spinlock_free(drv_priv,
					&h2c_pkt_pool->idle_h2c_pkt_cmd_list.lock);
			_os_spinlock_free(drv_priv,
					&h2c_pkt_pool->idle_h2c_pkt_data_list.lock);
			_os_spinlock_free(drv_priv,
					&h2c_pkt_pool->idle_h2c_pkt_ldata_list.lock);
			_os_spinlock_free(drv_priv,
					&h2c_pkt_pool->busy_h2c_pkt_list.lock);
			_os_spinlock_free(drv_priv,
					&h2c_pkt_pool->recycle_lock);
			_os_mem_free(drv_priv, h2c_pkt_pool, sizeof(struct phl_h2c_pkt_pool));
			h2c_pkt_pool = NULL;
			pstatus = RTW_PHL_STATUS_RESOURCE;
		} else {
			h2c_pkt_pool->h2c_pkt_buf = h2c_pkt_buf;
			pstatus = RTW_PHL_STATUS_SUCCESS;
		}
	}

	if (RTW_PHL_STATUS_SUCCESS == pstatus)
		phl_info->h2c_pool = h2c_pkt_pool;

	FUNCOUT_WSTS(pstatus);

	return pstatus;
}

void phl_datapath_deinit(struct phl_info_t *phl_info)
{
	struct rtw_phl_handler *tx_handler = &phl_info->phl_tx_handler;
	struct rtw_phl_handler *rx_handler = &phl_info->phl_rx_handler;
	struct rtw_phl_handler *event_handler = &phl_info->phl_event_handler;
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;
	void *drv_priv = NULL;
	FUNCIN();
	drv_priv = phl_to_drvpriv(phl_info);

	phl_deregister_handler(phl_info->phl_com, event_handler);
	phl_deregister_handler(phl_info->phl_com, rx_handler);
	phl_deregister_handler(phl_info->phl_com, tx_handler);

	_phl_free_h2c_pool(phl_info);

	// NEO : mark off first
	//hci_trx_ops->hci_trx_deinit(phl_info);

	phl_rx_deinit(phl_info);

	_phl_ring_status_deinit(phl_info);

	_os_spinlock_free(drv_priv, &phl_info->t_ring_list_lock);
	_os_spinlock_free(drv_priv, &phl_info->rx_ring_lock);
	_os_spinlock_free(drv_priv, &phl_info->t_fctrl_result_lock);
	_os_spinlock_free(drv_priv, &phl_info->t_ring_free_list_lock);

	FUNCOUT();
}

enum rtw_phl_status phl_datapath_init(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;
	struct rtw_phl_handler *event_handler = &phl_info->phl_event_handler;
	void *drv_priv = NULL;
	FUNCIN_WSTS(pstatus);
	drv_priv = phl_to_drvpriv(phl_info);

	do {
		INIT_LIST_HEAD(&phl_info->t_ring_list);
		INIT_LIST_HEAD(&phl_info->t_fctrl_result);
		INIT_LIST_HEAD(&phl_info->t_ring_free_list);
		_os_spinlock_init(drv_priv, &phl_info->t_ring_list_lock);
		_os_spinlock_init(drv_priv, &phl_info->rx_ring_lock);
		_os_spinlock_init(drv_priv, &phl_info->t_fctrl_result_lock);
		_os_spinlock_init(drv_priv, &phl_info->t_ring_free_list_lock);

		event_handler->type = RTW_PHL_HANDLER_PRIO_HIGH;
		event_handler->callback = phl_event_indicator;
		event_handler->context = phl_info;
		event_handler->drv_priv = drv_priv;
		event_handler->status = 0;
		pstatus = phl_register_handler(phl_info->phl_com, event_handler);
		if (RTW_PHL_STATUS_SUCCESS != pstatus)
			break;

		pstatus = _phl_ring_status_init(phl_info);
		if (RTW_PHL_STATUS_SUCCESS != pstatus)
			break;

		pstatus = phl_rx_init(phl_info);
		if (RTW_PHL_STATUS_SUCCESS != pstatus)
			break;

		pstatus = hci_trx_ops->hci_trx_init(phl_info);
		if (RTW_PHL_STATUS_SUCCESS != pstatus)
			break;

		/* allocate h2c pkt */
		pstatus = _phl_alloc_h2c_pool(phl_info);
		if (RTW_PHL_STATUS_SUCCESS != pstatus)
			break;

	}while (false);

	if (RTW_PHL_STATUS_SUCCESS != pstatus) {
		_os_spinlock_free(drv_priv, &phl_info->t_ring_list_lock);
		_os_spinlock_free(drv_priv, &phl_info->t_fctrl_result_lock);
		_os_spinlock_free(drv_priv, &phl_info->t_ring_free_list_lock);
	}
	FUNCOUT_WSTS(pstatus);

	return pstatus;
}


enum rtw_phl_status phl_datapath_start(struct phl_info_t *phl_info)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;

	do {
		pstatus = hci_trx_ops->trx_cfg(phl_info);
		if (RTW_PHL_STATUS_SUCCESS != pstatus)
			break;

	}while (false);

	return pstatus;
}

void phl_datapath_stop(struct phl_info_t *phl_info)
{
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;

	hci_trx_ops->trx_stop(phl_info);
	phl_free_deferred_tx_ring(phl_info);
}

void rtw_phl_tx_stop(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;

	/* Pause SW Tx */
	hci_trx_ops->req_tx_stop(phl_info);

}

void rtw_phl_tx_resume(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;

	/* Resume SW Tx */
	hci_trx_ops->trx_resume(phl_info, PHL_CTRL_TX);
}


enum rtw_phl_status rtw_phl_tx_req_notify(void *phl)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	pstatus = phl_schedule_handler(phl_info->phl_com,
					&phl_info->phl_tx_handler);

	return pstatus;
}


enum rtw_phl_status rtw_phl_add_tx_req(void *phl,
				struct rtw_xmit_req *tx_req)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_tring_list *tring_list, *t;
	struct rtw_phl_tx_ring *ring = NULL;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = NULL;
	_os_list *list_head = &phl_info->t_ring_list;
	u16 macid = tx_req->mdata.macid;
	u8 tid = tx_req->mdata.tid;
	u16 ring_res = 0, rptr = 0;

	drv_priv = phl_to_drvpriv(phl_info);

	_os_spinlock(drv_priv, &phl_info->t_ring_list_lock, _bh, NULL);

	phl_list_for_loop_safe(tring_list, t, struct rtw_phl_tring_list,
				list_head, list) {
		if (macid != tring_list->macid) {
			continue;
		} else {
			/* hana_todo check mgnt frame case */
			ring = &tring_list->phl_ring[tid];
			break;
		}
	}

	if (NULL != ring) {
		rptr = (u16)_os_atomic_read(drv_priv, &ring->phl_idx);

		ring_res = phl_calc_avail_wptr(rptr, ring->core_idx,
						MAX_PHL_RING_ENTRY_NUM);
		if (ring_res > 0) {
			ring->core_idx =
				(ring->core_idx + 1) % MAX_PHL_RING_ENTRY_NUM;
			ring->entry[ring->core_idx] = (u8 *)tx_req;
			// TODO : NEO : mark off first
			//phl_tx_statistics(phl_info, tx_req);
			pstatus = RTW_PHL_STATUS_SUCCESS;
		} else {
			PHL_TRACE(COMP_PHL_DBG, _PHL_INFO_, "no ring resource to add new tx request!\n");
			pstatus = RTW_PHL_STATUS_RESOURCE;
		}
	}

	_os_spinunlock(drv_priv, &phl_info->t_ring_list_lock, _bh, NULL);

	return pstatus;
}

u16 rtw_phl_tring_rsc(void *phl, u16 macid, u8 tid)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct rtw_phl_tring_list *tring_list, *t;
	struct rtw_phl_tx_ring *ring = NULL;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv_priv = NULL;
	_os_list *list_head = &phl_info->t_ring_list;
	u16 ring_res = 0, rptr = 0;

	drv_priv = phl_to_drvpriv(phl_info);

	phl_list_for_loop_safe(tring_list, t, struct rtw_phl_tring_list,
				list_head, list) {
		if (macid != tring_list->macid) {
			continue;
		} else {
			/* hana_todo check mgnt frame case */
			ring = &tring_list->phl_ring[tid];
			break;
		}
	}

	if (NULL != ring) {
		rptr = (u16)_os_atomic_read(drv_priv, &ring->phl_idx);

		ring_res = phl_calc_avail_rptr(rptr, ring->core_idx,
						MAX_PHL_RING_ENTRY_NUM);

	}

	return ring_res;
}


enum rtw_phl_status phl_indic_pkt_complete(void *phl)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct rtw_evt_info_t *evt_info = &phl_info->phl_com->evt_info;
	void *drv_priv = phl_to_drvpriv(phl_info);

	do {
		_os_spinlock(drv_priv, &evt_info->evt_lock, _bh, NULL);
		evt_info->evt_bitmap |= RTW_PHL_EVT_TX_RECYCLE;
		_os_spinunlock(drv_priv, &evt_info->evt_lock, _bh, NULL);

		pstatus = phl_schedule_handler(phl_info->phl_com,
							&phl_info->phl_event_handler);
	} while (false);

	return pstatus;
}

enum rtw_phl_status rtw_phl_recycle_tx_buf(void *phl, u8 *tx_buf_ptr)
{
	enum rtw_phl_status pstatus = RTW_PHL_STATUS_FAILURE;
#ifdef CONFIG_USB_HCI
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	struct phl_hci_trx_ops *hci_trx_ops = phl_info->hci_trx_ops;

	pstatus = hci_trx_ops->recycle_tx_buf(phl, tx_buf_ptr);

#endif
	return pstatus;
}


#if 0 // TODO NEO mark off first
enum rtw_phl_status
rtw_phl_cfg_tx_ampdu(void *phl, struct rtw_phl_stainfo_t *sta)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;
	enum rtw_hal_status hsts = RTW_HAL_STATUS_FAILURE;
	void *drv_priv = phl_to_drvpriv(phl_info);

	hsts = rtw_hal_cfg_tx_ampdu(phl_info->hal, sta);
	if (RTW_HAL_STATUS_SUCCESS != hsts)
		goto fail;

	return RTW_PHL_STATUS_SUCCESS;

fail:
	return RTW_PHL_STATUS_FAILURE;
}

enum rtw_phl_status
rtw_phl_cmd_cfg_ampdu(void *phl,
			struct rtw_wifi_role_t *wrole,
			struct rtw_phl_stainfo_t *sta,
			enum phl_cmd_type cmd_type,
			u32 cmd_timeout)
{
	enum rtw_phl_status sts = RTW_PHL_STATUS_FAILURE;
#ifdef CONFIG_CMD_DISP
	sts = phl_cmd_enqueue(phl,
			wrole->hw_band,
			MSG_EVT_CFG_AMPDU,
			(u8 *)sta, 0,
			NULL,
			cmd_type, cmd_timeout);
	if (is_cmd_failure(sts))
		sts = RTW_PHL_STATUS_FAILURE;
	else
		sts = RTW_PHL_STATUS_SUCCESS;
	return sts;
#else
	PHL_ERR("%s : CONFIG_CMD_DISP need to be enabled for MSG_EVT_CFG_AMPDU !! \n", __func__);

	return sts;
#endif
}
#endif // if 0 NEO

void
phl_tx_watchdog(struct phl_info_t *phl_info)
{
	struct phl_hci_trx_ops *trx_ops = phl_info->hci_trx_ops;
	struct rtw_stats *phl_stats = &phl_info->phl_com->phl_stats;

	phl_tx_traffic_upd(phl_stats);
	trx_ops->tx_watchdog(phl_info);
}

#if 0 // NEO 
enum rtw_phl_status
_phl_poll_hw_tx_done(void)
{
	PHL_TRACE(COMP_PHL_XMIT, _PHL_ERR_, "[DATA_CTRL] Polling hw tx done is not supported now\n");

	return RTW_PHL_STATUS_FAILURE;
}

enum rtw_phl_status
_phl_hw_tx_resume(void)
{
	PHL_TRACE(COMP_PHL_XMIT, _PHL_ERR_, "[DATA_CTRL] Resume hw tx not is supported now\n");

	return RTW_PHL_STATUS_FAILURE;
}

enum rtw_phl_status
_phl_sw_tx_resume(struct phl_info_t *phl_info, struct phl_data_ctl_t *ctl)
{
	enum rtw_phl_status sts = RTW_PHL_STATUS_FAILURE;
	struct phl_hci_trx_ops *ops = phl_info->hci_trx_ops;

	if (ctl->id != phl_info->pause_tx_id) {
		PHL_WARN("[DATA_CTRL] module %d resume sw tx fail, sw tx is paused by module %d\n",
			 ctl->id, phl_info->pause_tx_id);
		return sts;
	}

	ops->trx_resume(phl_info, PHL_CTRL_TX);
	phl_info->pause_tx_id = PHL_MDL_ID_MAX;

	sts = rtw_phl_tx_req_notify(phl_info);

	return sts;
}

void
_phl_sw_tx_rst(struct phl_info_t *phl_info)
{
	struct phl_hci_trx_ops *ops = phl_info->hci_trx_ops;

	ops->trx_reset(phl_info, PHL_CTRL_TX);
}

enum rtw_phl_status
_phl_sw_tx_pause(struct phl_info_t *phl_info, struct phl_data_ctl_t *ctl,
		 bool rst_sw)
{
	enum rtw_phl_status sts = RTW_PHL_STATUS_FAILURE;
	struct phl_hci_trx_ops *ops = phl_info->hci_trx_ops;
	void *drv = phl_to_drvpriv(phl_info);
	u32 i = 0;

	if (PHL_TX_STATUS_SW_PAUSE ==
	    _os_atomic_read(drv, &phl_info->phl_sw_tx_sts)) {
		PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_,
			  "[DATA_CTRL] SW tx has been paused by module(%d)\n",
			  phl_info->pause_tx_id);

		PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_,
			  "[DATA_CTRL] Update pause sw tx id to module(%d)\n",
			  ctl->id);
		phl_info->pause_tx_id = ctl->id;
		sts = RTW_PHL_STATUS_SUCCESS;
		return sts;
	}

	if (PHL_TX_STATUS_STOP_INPROGRESS ==
	    _os_atomic_read(drv, &phl_info->phl_sw_tx_sts)) {
		PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_,
			  "[DATA_CTRL] SW tx has been requested to pause by module(%d)\n",
			  phl_info->pause_tx_id);

		PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_,
			  "[DATA_CTRL] Update pause sw tx id to module(%d)\n",
			  ctl->id);
		phl_info->pause_tx_id = ctl->id;
		sts = RTW_PHL_STATUS_SUCCESS;
		return sts;
	}

	/* requset sw tx to stop */
	ops->req_tx_stop(phl_info);

	/* notify sw tx one last time, and poll if it receviced the stop request
           and paused itself */
	if (RTW_PHL_STATUS_SUCCESS == rtw_phl_tx_req_notify(phl_info)) {

		for (i = 0; i < POLL_SW_TX_PAUSE_CNT; i++) {

			if (true == ops->is_tx_pause(phl_info)) {
				phl_info->pause_tx_id = ctl->id;
				sts = RTW_PHL_STATUS_SUCCESS;
				break;
			}
			_os_delay_ms(drv, POLL_SW_TX_PAUSE_MS);
		}

		if (RTW_PHL_STATUS_SUCCESS != sts) {
			phl_info->pause_tx_id = ctl->id;
			sts = RTW_PHL_STATUS_CMD_TIMEOUT;
			PHL_TRACE(COMP_PHL_XMIT, _PHL_ERR_,
				  "[DATA_CTRL] Module(%d) polling sw tx pause timeout (%d ms)!\n",
				  ctl->id,
				  (POLL_SW_TX_PAUSE_MS * POLL_SW_TX_PAUSE_CNT));
		} else {
			if (true == rst_sw) {
				PHL_TRACE(COMP_PHL_XMIT, _PHL_WARNING_,
					  "[DATA_CTRL] Pause Tx with reset is not supported now! requested by module(%d)\n",
					  ctl->id);
			}
		}
	} else {
		PHL_TRACE(COMP_PHL_XMIT, _PHL_WARNING_, "[DATA_CTRL] Schedule sw tx process fail!\n");
	}

	return sts;
}

enum rtw_phl_status
_phl_poll_hw_rx_done(void)
{
	PHL_TRACE(COMP_PHL_RECV, _PHL_ERR_, "[DATA_CTRL] Polling hw rx done is not supported now\n");

	return RTW_PHL_STATUS_FAILURE;
}

enum rtw_phl_status
_phl_hw_rx_resume(void)
{
	PHL_TRACE(COMP_PHL_RECV, _PHL_ERR_, "[DATA_CTRL] Resume hw rx not is supported now\n");

	return RTW_PHL_STATUS_FAILURE;
}

enum rtw_phl_status
_phl_sw_rx_resume(struct phl_info_t *phl_info, struct phl_data_ctl_t *ctl)
{
	enum rtw_phl_status sts = RTW_PHL_STATUS_FAILURE;
	struct phl_hci_trx_ops *ops = phl_info->hci_trx_ops;

	if (ctl->id != phl_info->pause_rx_id) {
		PHL_WARN("[DATA_CTRL] module %d resume sw rx fail, sw rx is paused by module %d\n",
			 ctl->id, phl_info->pause_rx_id);
		return sts;
	}

	ops->trx_resume(phl_info, PHL_CTRL_RX);
	phl_info->pause_rx_id = PHL_MDL_ID_MAX;

	sts = rtw_phl_start_rx_process(phl_info);

	return sts;
}

void
_phl_sw_rx_rst(struct phl_info_t *phl_info)
{
	struct phl_hci_trx_ops *ops = phl_info->hci_trx_ops;

	ops->trx_reset(phl_info, PHL_CTRL_RX);
}

enum rtw_phl_status
_phl_sw_rx_pause(struct phl_info_t *phl_info, struct phl_data_ctl_t *ctl,
		 bool rst_sw)
{
	enum rtw_phl_status sts = RTW_PHL_STATUS_FAILURE;
	struct phl_hci_trx_ops *ops = phl_info->hci_trx_ops;
	void *drv = phl_to_drvpriv(phl_info);
	u32 i = 0;

	if (PHL_RX_STATUS_SW_PAUSE ==
	    _os_atomic_read(drv, &phl_info->phl_sw_rx_sts)) {
		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_,
			  "[DATA_CTRL] SW rx has been paused by module(%d)\n",
			  phl_info->pause_rx_id);

		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_,
			  "[DATA_CTRL] Update pause sw rx id to module(%d)\n",
			  ctl->id);
		phl_info->pause_rx_id = ctl->id;
		sts = RTW_PHL_STATUS_SUCCESS;
		return sts;
	}

	if (PHL_RX_STATUS_STOP_INPROGRESS ==
	    _os_atomic_read(drv, &phl_info->phl_sw_rx_sts)) {
		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_,
			  "[DATA_CTRL] SW rx has been requested to pause by module(%d)\n",
			  phl_info->pause_rx_id);

		PHL_TRACE(COMP_PHL_RECV, _PHL_INFO_,
			  "[DATA_CTRL] Update pause sw rx id to module(%d)\n",
			  ctl->id);
		phl_info->pause_rx_id = ctl->id;
		sts = RTW_PHL_STATUS_SUCCESS;
		return sts;
	}

	/* requset sw rx to stop */
	ops->req_rx_stop(phl_info);

	/* notify sw rx one last time, and poll if it receviced the stop request
           and paused itself */
	if (RTW_PHL_STATUS_SUCCESS == rtw_phl_start_rx_process(phl_info)) {

		for (i = 0; i < POLL_SW_RX_PAUSE_CNT; i++) {

			if (true == ops->is_rx_pause(phl_info)) {
				phl_info->pause_rx_id = ctl->id;
				sts = RTW_PHL_STATUS_SUCCESS;
				break;
			}
			_os_delay_ms(drv, POLL_SW_RX_PAUSE_MS);
		}

		if (RTW_PHL_STATUS_SUCCESS != sts) {
			phl_info->pause_rx_id = ctl->id;
			sts = RTW_PHL_STATUS_CMD_TIMEOUT;
			PHL_TRACE(COMP_PHL_RECV, _PHL_ERR_,
				  "[DATA_CTRL] Module(%d) polling sw rx pause timeout (%d ms)!\n",
				  ctl->id,
				  (POLL_SW_RX_PAUSE_MS * POLL_SW_RX_PAUSE_CNT));
		} else {
			if (true == rst_sw) {
				PHL_TRACE(COMP_PHL_RECV, _PHL_WARNING_,
					  "[DATA_CTRL] Pause Rx with reset is not supported now! requested by module(%d)\n",
					  ctl->id);
			}
		}
	} else {
		PHL_TRACE(COMP_PHL_RECV, _PHL_WARNING_, "[DATA_CTRL] Schedule sw rx process fail!\n");
	}

	return sts;
}

enum rtw_phl_status
_phl_hw_trx_rst_resume(struct phl_info_t *phl_info)
{
	void *drv = phl_to_drvpriv(phl_info);
	u32 mac_err = 0;

	if (false == _os_atomic_read(drv, &phl_info->is_hw_trx_pause)) {
		PHL_TRACE(COMP_PHL_XMIT, _PHL_WARNING_, "[DATA_CTRL] HW T/Rx is not paused\n");
	}

	mac_err = rtw_hal_lv1_rcvy(phl_info->hal,
				   RTW_PHL_SER_LV1_SER_RCVY_STEP_2);

	if (0 != mac_err) {
		PHL_TRACE(COMP_PHL_XMIT, _PHL_ERR_,
		  "[DATA_CTRL] Reset and Resume HW T/Rx fail (mac_err = 0x%x)\n",
			  mac_err);
		return RTW_PHL_STATUS_FAILURE;
	} else {
		_os_atomic_set(drv, &phl_info->is_hw_trx_pause, false);
		return RTW_PHL_STATUS_SUCCESS;
	}
}

enum rtw_phl_status
_phl_hw_trx_pause(struct phl_info_t *phl_info)
{
	void *drv = phl_to_drvpriv(phl_info);
	u32 mac_err = 0;

	if (true == _os_atomic_read(drv, &phl_info->is_hw_trx_pause)) {
		PHL_TRACE(COMP_PHL_XMIT, _PHL_WARNING_, "[DATA_CTRL] HW T/Rx is already paused\n");
	}

	mac_err = rtw_hal_lv1_rcvy(phl_info->hal,
				   RTW_PHL_SER_LV1_RCVY_STEP_1);

	if (0 != mac_err) {
		PHL_TRACE(COMP_PHL_XMIT, _PHL_ERR_,
		  "[DATA_CTRL] Pause HW T/Rx fail (mac_err = 0x%x)\n",
			  mac_err);
		return RTW_PHL_STATUS_FAILURE;
	} else {
		_os_atomic_set(drv, &phl_info->is_hw_trx_pause, true);
		return RTW_PHL_STATUS_SUCCESS;
	}
}

enum rtw_phl_status
_phl_trx_pause(struct phl_info_t *phl_info, struct phl_data_ctl_t *ctl)
{
	enum rtw_phl_status sts = RTW_PHL_STATUS_FAILURE;

	do {
		sts = _phl_sw_tx_pause(phl_info, ctl, false);
		if (RTW_PHL_STATUS_SUCCESS != sts) {
			PHL_TRACE(COMP_PHL_XMIT, _PHL_WARNING_, "[DATA_CTRL] Pause SW Tx fail in PHL_DATA_CTL_TRX_PAUSE!\n");
			break;
		}

		sts = _phl_sw_rx_pause(phl_info, ctl, false);
		if (RTW_PHL_STATUS_SUCCESS != sts) {
			PHL_TRACE(COMP_PHL_RECV, _PHL_WARNING_, "[DATA_CTRL] Pause SW Rx fail in PHL_DATA_CTL_TRX_PAUSE!\n");
			break;
		}
	} while (false);

	return sts;
}

enum rtw_phl_status
_phl_trx_pause_w_rst(struct phl_info_t *phl_info, struct phl_data_ctl_t *ctl,
		     struct phl_msg *msg)
{
	enum rtw_phl_status sts = RTW_PHL_STATUS_FAILURE;
	enum data_ctrl_err_code *err_sts = NULL;

	if (msg->outbuf && msg->outlen == sizeof(*err_sts))
		err_sts = (enum data_ctrl_err_code *)msg->outbuf;

	do {
		sts = _phl_sw_tx_pause(phl_info, ctl, false);
		if (RTW_PHL_STATUS_SUCCESS != sts) {
			if (err_sts) {
				if (RTW_PHL_STATUS_CMD_TIMEOUT == sts)
					*err_sts = CTRL_ERR_SW_TX_PAUSE_POLLTO;
				else
					*err_sts = CTRL_ERR_SW_TX_PAUSE_FAIL;
			}
			PHL_TRACE(COMP_PHL_XMIT, _PHL_WARNING_, "[DATA_CTRL] Pause SW Tx fail in PHL_DATA_CTL_TRX_PAUSE_W_RST!\n");
			break;
		}

		sts = _phl_hw_trx_pause(phl_info);
		if (RTW_PHL_STATUS_SUCCESS != sts) {
			if (err_sts)
				*err_sts = CTRL_ERR_HW_TRX_PAUSE_FAIL;
			PHL_TRACE(COMP_PHL_XMIT, _PHL_WARNING_, "[DATA_CTRL] Pause HW T/Rx fail in PHL_DATA_CTL_TRX_PAUSE_W_RST!\n");
			break;
		}

		sts = _phl_sw_rx_pause(phl_info, ctl, false);
		if (RTW_PHL_STATUS_SUCCESS != sts) {
			if (err_sts) {
				if (RTW_PHL_STATUS_CMD_TIMEOUT == sts)
					*err_sts = CTRL_ERR_SW_RX_PAUSE_POLLTO;
				else
					*err_sts = CTRL_ERR_SW_RX_PAUSE_FAIL;
			}
			PHL_TRACE(COMP_PHL_RECV, _PHL_WARNING_, "[DATA_CTRL] Pause SW Rx fail in PHL_DATA_CTL_TRX_PAUSE_W_RST!\n");
			break;
		}

		_phl_sw_tx_rst(phl_info);
		_phl_sw_rx_rst(phl_info);
	} while (false);

	return sts;
}
#endif // if 0 NEO

enum rtw_phl_status
phl_data_ctrler(struct phl_info_t *phl_info, struct phl_data_ctl_t *ctl,
		struct phl_msg *msg)
{
	enum rtw_phl_status sts = RTW_PHL_STATUS_FAILURE;

	if (NULL == ctl) {
		PHL_WARN("phl_tx_ctrler(): input ctl is NULL\n");
		return RTW_PHL_STATUS_FAILURE;
	}

	RTW_INFO("%s NEO TODO\n", __func__);
	return RTW_PHL_STATUS_SUCCESS;
#if 0 // NEO
	switch (ctl->cmd) {
	case PHL_DATA_CTL_HW_TRX_RST_RESUME:
		sts = _phl_hw_trx_rst_resume(phl_info);
		break;
	case PHL_DATA_CTL_HW_TRX_PAUSE:
		sts = _phl_hw_trx_pause(phl_info);
		break;
	case PHL_DATA_CTL_SW_TX_RESUME:
		sts = _phl_sw_tx_resume(phl_info, ctl);
		break;
	case PHL_DATA_CTL_SW_RX_RESUME:
		sts = _phl_sw_rx_resume(phl_info, ctl);
		break;
	case PHL_DATA_CTL_SW_TX_PAUSE:
		sts = _phl_sw_tx_pause(phl_info, ctl, false);
		break;
 	case PHL_DATA_CTL_SW_RX_PAUSE:
		sts = _phl_sw_rx_pause(phl_info, ctl, false);
		break;
	case PHL_DATA_CTL_SW_TX_RESET:
		_phl_sw_tx_rst(phl_info);
		sts = RTW_PHL_STATUS_SUCCESS;
		break;
	case PHL_DATA_CTL_SW_RX_RESET:
		_phl_sw_rx_rst(phl_info);
		sts = RTW_PHL_STATUS_SUCCESS;
		break;
 	case PHL_DATA_CTL_TRX_PAUSE:
		sts = _phl_trx_pause(phl_info, ctl);
		break;
	case PHL_DATA_CTL_TRX_PAUSE_W_RST:
		sts = _phl_trx_pause_w_rst(phl_info, ctl, msg);
		break;
	default:
		PHL_TRACE(COMP_PHL_XMIT, _PHL_INFO_,
			  "Unknown data control command(%d)!\n", ctl->cmd);
		break;
	}
	return sts;
#endif // if 0 NEO
}

