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
#ifndef _TEST_MODULE_H_
#define _TEST_MODULE_H_

#ifdef CONFIG_PHL_TEST_SUITE
#define TEST_NAME_LEN		32
#define TEST_RPT_RSN_LEN	32

#define TEST_LVL_LOW_TO		1000
#define TEST_LVL_NORMAL_TO	50
#define TEST_LVL_HIGH_TO	20

/*add cusstomized BP types in the following enum,
and hanle them in corresponding bp handler*/
enum TEST_BP_INFO_TYPE{
	BP_INFO_TYPE_NONE = 0,
	BP_INFO_TYPE_WAIT_BEACON_JOIN = 0x1,
	BP_INFO_TYPE_SEND_AUTH_ODD = 0x2,
	BP_INFO_TYPE_SEND_ASOC_REQ = 0x3,
	BP_INFO_TYPE_SEND_DISASSOC = 0x4,
	BP_INFO_TYPE_FILL_DISASSOC_RSN = 0x5,
	BP_INFO_TYPE_SEND_PROBE_REQ = 0x6,
	BP_INFO_TYPE_MP_CMD_EVENT = 0x7,
	BP_INFO_TYPE_RX_TEST_WPRPT = 0x8,
	BP_INFO_TYPE_RX_TEST_PATTERN = 0x9,
	BP_INFO_TYPE_MP_RX_PHYSTS = 0xA,
	BP_INFO_TYPE_TX_4_WAY = 0xB,
	BP_INFO_TYPE_RX_4_WAY = 0xC,
	BP_INFO_TYPE_MAX
};

enum TEST_RUN_LVL{
	TEST_LVL_NONE = 0,
	TEST_LVL_LOW,
	TEST_LVL_NORMAL,
	TEST_LVL_HIGH,
	TEST_LVL_MAX
};
enum TEST_BP_RETURN_TYPE{
	BP_RET_SKIP_SECTION = 0,
	BP_RET_RUN_ORIGIN_SEC,
	BP_RET_LEAVE_FUNC,
	BP_RET_MAX
};

enum TEST_SUB_MODULE {
	TEST_SUB_MODULE_MP = 0,
	TEST_SUB_MODULE_FPGA = 1,
	TEST_SUB_MODULE_VERIFY = 2,
	TEST_SUB_MODULE_TOOL = 3,
	TEST_SUB_MODULE_TRX = 4,
	TEST_SUB_MODULE_UNKNOWN,
};

enum TEST_MODULE_MODE_TYPE {
	UNIT_TEST_MODE = 0,
	INTGR_TEST_MODE = 1,
	FUNC_TEST_MODE = 2
};

struct test_bp_info{
	enum TEST_BP_INFO_TYPE type;
	u32 len;
	void* ptr;
};

/**
 * test_obj_ctrl_interface - basic test control methods for generic management.
 * @start_test: test entry, initiate & run a test
 * @is_test_end: return true when test ends. 
 *		 NOTE: Do not use evt/lock inside this method for sync.
 * @is_test_pass: return true when test passed. 
 * @get_fail_rsn: if test fails, construct a reasonable string as fail description, 
 *		  not just a status code.
 * @bp_handler: handle break point which is currently being hit, 
 *		use rtw_test_setup_bp to add new break point in source code
 *		and add customized BP type in TEST_BP_INFO_TYPE for recognition.
 */

struct test_obj_ctrl_interface{
	u8 (*start_test)(void *priv);
	u8 (*is_test_end)(void *priv);
	u8 (*is_test_pass)(void *priv);
	u8 (*get_fail_rsn)(void *priv,char* rsn, u32 max_len);
	u8 (*bp_handler)(void *priv, struct test_bp_info* bp_info);
};

struct test_object {
 	_os_list list;
	void* priv;
	enum TEST_RUN_LVL run_lvl;
	char name[TEST_NAME_LEN];
	struct test_obj_ctrl_interface ctrl;
	s32 total_time_ms; // optional, set 0 to use default see TEST_LVL_LOW_TO
};
struct test_rpt {
	char name[TEST_NAME_LEN];
	u8 status;
	char rsn[TEST_RPT_RSN_LEN];
	u32 total_time; // in ms
};

struct phl_info_t;

/* called in phl init*/
u8 rtw_test_module_alloc(struct phl_info_t *phl_info);
/* called in phl deinit*/
void rtw_test_module_free(struct rtw_phl_com_t* phl_com);

u8 rtw_test_module_init(struct rtw_phl_com_t* phl_com);
void rtw_test_module_deinit(struct rtw_phl_com_t* phl_com);

/* following methods are being called on demand*/
u8 rtw_test_add_new_test_obj(struct rtw_phl_com_t* phl_com, char *name, void* priv,
	enum TEST_RUN_LVL lvl,  struct test_obj_ctrl_interface* ctrl_intf,
	s32 total_time_ms, u8 objid, u8 test_mode);
u8 rtw_test_setup_bp(struct rtw_phl_com_t* phl_com,struct test_bp_info* bp_info, u8 submdid);
u8 rtw_test_is_test_complete(struct rtw_phl_com_t* phl_com);
u8 rtw_test_get_test_rpt(struct rtw_phl_com_t* phl_com, u8* buf, u32 len);
u8 rtw_test_set_max_run_time(struct rtw_phl_com_t* phl_com, enum TEST_RUN_LVL lvl, u32 timeout_ms);

/* phl test mp command */
enum rtw_phl_status rtw_test_mp_alloc(struct phl_info_t *phl_info, void *hal, void **mp);
void rtw_test_mp_free(void **mp);

void rtw_test_mp_init(void *mp);
void rtw_test_mp_deinit(void *mp);
void rtw_test_mp_start(void *mp, u8 tm_mode);
void rtw_test_mp_stop(void *mp, u8 tm_mode);
void rtw_test_mp_cmd_process(void *mp, void *buf, u32 buf_len, u8 submdid);
void rtw_test_mp_get_rpt(void *mp, void *buf, u32 buf_len);

/* phl test verify command */
enum rtw_phl_status rtw_test_verify_alloc(struct phl_info_t *phl_info, void *hal, void **ctx);
void rtw_test_verify_free(void **ctx);
void rtw_test_verify_init(void *ctx);
void rtw_test_verify_deinit(void *ctx);
void rtw_test_verify_start(void *ctx);
void rtw_test_verify_stop(void *ctx);
void rtw_test_verify_cmd_process(void *ctx, void *buf, u32 buf_len, u8 submdid);
void rtw_test_verify_get_rpt(void *ctx, void *buf, u32 buf_len);

/* test submodule function */
void rtw_test_submodule_init(void *tm, void *buf);
void rtw_test_submodule_deinit(void *tm, void *buf);
void rtw_test_submodule_cmd_process(void *tm, void *buf, u32 buf_len);
void rtw_test_submodule_get_rpt(void *tm, void *buf, u32 buf_len);

enum rtw_phl_status rtw_phl_reset(void *phl);


#else
#define rtw_test_module_alloc(phl_info) 1
#define rtw_test_module_free(phl_com)
#define rtw_test_module_init(phl_com) true
#define rtw_test_module_deinit
#define rtw_test_get_test_rpt(phl_com, buf, len)
#define rtw_test_submodule_init(_tm, _buf)
#define rtw_test_submodule_deinit(_tm, _buf)
#define rtw_test_submodule_cmd_process(_tm, _buf, _buf_len)
#define rtw_test_submodule_get_rpt(_tm, _buf, _buf_len)
#endif /*CONFIG_PHL_TEST_SUITE*/

#endif	/* _TEST_MODULE_H_ */
