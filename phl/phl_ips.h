/******************************************************************************
 *
 * Copyright(c) 2019 - 2020 Realtek Corporation.
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
#ifndef __PHL_IPS_H__
#define __PHL_IPS_H__

#ifdef CONFIG_PS_IPS

#define IPS_RSVD_TOKEN	0xFFFFFFFF

enum phl_ips_mode {
	IPS_MODE_PWR_DOWN,
	IPS_MODE_FW_IPS,
};

/* inactive power save fsm init api */
enum rtw_phl_status
phl_ips_init(void **ips_obj, struct phl_info_t *phl_info);
void phl_ips_deinit(void *ips_obj);
void phl_ips_periodic_chk(void *ips_obj);
enum rtw_phl_status
phl_ips_issue_cmd(void *ips_obj, struct ps_ntfy *ntfy);
void phl_ips_cancel_cmd(void *ips_obj, struct ps_ntfy *ntfy);
void phl_ips_dbg_dump_obj(void *ips_obj, u32 *used, char input[][MAX_ARGV],
			u32 input_num, char *output, u32 out_len);


#else
#define phl_ips_init(_ips_obj, _phl_info) RTW_PHL_STATUS_SUCCESS
#define phl_ips_deinit(_ips_obj) do {} while (0)
#define phl_ips_periodic_chk(_ips_obj)
#define phl_ips_issue_cmd(_ips_obj, _ntfy) RTW_PHL_STATUS_SUCCESS
#define phl_ips_cancel_cmd(_ips_obj, _ntfy)
#define phl_ips_dbg_dump_obj(_ips_obj, _used, _input,	\
				_input_num, _output, _out_len);

#endif /* CONFIG_PS_IPS */

#endif /* __PHL_IPS_H__ */

