/******************************************************************************
 *
 * Copyright(c) 2007 - 2017 Realtek Corporation.
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
#ifndef __SDIO_OPS_LINUX_H__
#define __SDIO_OPS_LINUX_H__


bool rtw_is_sdio30(_adapter *adapter);

/* The unit of return value is Hz */
static inline u32 rtw_sdio_get_clock(struct dvobj_priv *d)
{
	return d->intf_data.clock;
}

void rtw_sdio_set_irq_thd(struct dvobj_priv *dvobj, _thread_hdl_ thd_hdl);
int __must_check rtw_sdio_raw_read(struct dvobj_priv *d, unsigned int addr,
				void *buf, size_t len, bool fixed);
int __must_check rtw_sdio_raw_write(struct dvobj_priv *d, unsigned int addr,
				void *buf, size_t len, bool fixed);

#endif /* __SDIO_OPS_LINUX_H__ */

