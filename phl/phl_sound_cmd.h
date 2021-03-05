/******************************************************************************
 *
 * Copyright(c) 2020 Realtek Corporation.
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
#ifndef _PHL_SOUND_CMD_H_
#define _PHL_SOUND_CMD_H_

#ifdef CONFIG_CMD_DISP

enum snd_cmd_disp_ctrl {
	SND_CMD_DISP_CTRL_BFEE = 0,
	SND_CMD_DISP_CTRL_BFER,
	SND_CMD_DISP_CTRL_MAX
};


enum rtw_phl_status phl_snd_cmd_register_module(struct phl_info_t *phl_info);

#endif
#endif