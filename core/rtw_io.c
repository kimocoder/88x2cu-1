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
/*

The purpose of rtw_io.c

a. provides the API

b. provides the protocol engine

c. provides the software interface between caller and the hardware interface


Compiler Flag Option:

1. CONFIG_SDIO_HCI:
    a. USE_SYNC_IRP:  Only sync operations are provided.
    b. USE_ASYNC_IRP:Both sync/async operations are provided.

2. CONFIG_USB_HCI:
   a. USE_ASYNC_IRP: Both sync/async operations are provided.

3. CONFIG_CFIO_HCI:
   b. USE_SYNC_IRP: Only sync operations are provided.


Only sync read/rtw_write_mem operations are provided.

jackson@realtek.com.tw

*/

#define _RTW_IO_C_

#include <drv_types.h>
#include <hal_data.h>

#if defined(CONFIG_SDIO_HCI) || defined(CONFIG_PLATFORM_RTL8197D)
	#define rtw_le16_to_cpu(val)		val
	#define rtw_le32_to_cpu(val)		val
	#define rtw_cpu_to_le16(val)		val
	#define rtw_cpu_to_le32(val)		val
#else
	#define rtw_le16_to_cpu(val)		le16_to_cpu(val)
	#define rtw_le32_to_cpu(val)		le32_to_cpu(val)
	#define rtw_cpu_to_le16(val)		cpu_to_le16(val)
	#define rtw_cpu_to_le32(val)		cpu_to_le32(val)
#endif


u32 _rtw_write_port(_adapter *adapter, u32 addr, u32 cnt, u8 *pmem)
{
	u32(*_write_port)(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *pmem);
	/* struct	io_queue  	*pio_queue = (struct io_queue *)adapter->pio_queue; */
	struct io_priv *pio_priv = &adapter->iopriv;
	struct	intf_hdl		*pintfhdl = &(pio_priv->intf);
	u32 ret = _SUCCESS;


	_write_port = pintfhdl->io_ops._write_port;

	ret = _write_port(pintfhdl, addr, cnt, pmem);


	return ret;
}

u32 _rtw_write_port_and_wait(_adapter *adapter, u32 addr, u32 cnt, u8 *pmem, int timeout_ms)
{
	int ret = _SUCCESS;
	struct xmit_buf *pxmitbuf = (struct xmit_buf *)pmem;
	struct submit_ctx sctx;

	rtw_sctx_init(&sctx, timeout_ms);
	pxmitbuf->sctx = &sctx;

	ret = _rtw_write_port(adapter, addr, cnt, pmem);

	if (ret == _SUCCESS) {
		ret = rtw_sctx_wait(&sctx, __func__);

		if (ret != _SUCCESS)
			pxmitbuf->sctx = NULL;
	}

	return ret;
}

void _rtw_write_port_cancel(_adapter *adapter)
{
	void (*_write_port_cancel)(struct intf_hdl *pintfhdl);
	struct io_priv *pio_priv = &adapter->iopriv;
	struct intf_hdl *pintfhdl = &(pio_priv->intf);

	_write_port_cancel = pintfhdl->io_ops._write_port_cancel;

	RTW_DISABLE_FUNC(adapter_to_dvobj(adapter), DF_TX_BIT);

	if (_write_port_cancel)
		_write_port_cancel(pintfhdl);
}

int rtw_init_io_priv(_adapter *padapter, void (*set_intf_ops)(_adapter *padapter, struct _io_ops *pops))
{
	struct io_priv	*piopriv = &padapter->iopriv;
	struct intf_hdl *pintf = &piopriv->intf;

	if (set_intf_ops == NULL)
		return _FAIL;

	piopriv->padapter = padapter;
	pintf->padapter = padapter;
	pintf->pintf_dev = adapter_to_dvobj(padapter);

	set_intf_ops(padapter, &pintf->io_ops);

	return _SUCCESS;
}

/*
* Increase and check if the continual_io_error of this @param dvobjprive is larger than MAX_CONTINUAL_IO_ERR
* @return _TRUE:
* @return _FALSE:
*/
int rtw_inc_and_chk_continual_io_error(struct dvobj_priv *dvobj)
{
	int ret = _FALSE;
	int value;

	value = ATOMIC_INC_RETURN(&dvobj->continual_io_error);
	if (value > MAX_CONTINUAL_IO_ERR) {
		RTW_INFO("[dvobj:%p][ERROR] continual_io_error:%d > %d\n", dvobj, value, MAX_CONTINUAL_IO_ERR);
		ret = _TRUE;
	} else {
		/* RTW_INFO("[dvobj:%p] continual_io_error:%d\n", dvobj, value); */
	}
	return ret;
}

/*
* Set the continual_io_error of this @param dvobjprive to 0
*/
void rtw_reset_continual_io_error(struct dvobj_priv *dvobj)
{
	ATOMIC_SET(&dvobj->continual_io_error, 0);
}

#ifdef DBG_IO
#define RTW_IO_SNIFF_TYPE_RANGE	0 /* specific address range is accessed */
#define RTW_IO_SNIFF_TYPE_VALUE	1 /* value match for sniffed range */

struct rtw_io_sniff_ent {
	u8 chip;
	u8 hci;
	u32 addr;
	u8 type;
	union {
		u32 end_addr;
		struct {
			u32 mask;
			u32 val;
			bool equal;
		} vm; /* value match */
	} u;
	bool trace;
	char *tag;
	bool (*assert_protsel)(_adapter *adapter, u32 addr, u8 len);
};

#define RTW_IO_SNIFF_RANGE_ENT(_chip, _hci, _addr, _end_addr, _trace, _tag) \
	{.chip = _chip, .hci = _hci, .addr = _addr, .u.end_addr = _end_addr, .trace = _trace, .tag = _tag, .type = RTW_IO_SNIFF_TYPE_RANGE,}

#define RTW_IO_SNIFF_RANGE_PROT_ENT(_chip, _hci, _addr, _end_addr, _assert_protsel, _tag) \
	{.chip = _chip, .hci = _hci, .addr = _addr, .u.end_addr = _end_addr, .trace = 1, .assert_protsel = _assert_protsel, .tag = _tag, .type = RTW_IO_SNIFF_TYPE_RANGE,}

#define RTW_IO_SNIFF_VALUE_ENT(_chip, _hci, _addr, _mask, _val, _equal, _trace, _tag) \
	{.chip = _chip, .hci = _hci, .addr = _addr, .u.vm.mask = _mask, .u.vm.val = _val, .u.vm.equal = _equal, .trace = _trace, .tag = _tag, .type = RTW_IO_SNIFF_TYPE_VALUE,}

/* part or all sniffed range is enabled (not all 0) */
#define RTW_IO_SNIFF_EN_ENT(_chip, _hci, _addr, _mask, _trace, _tag) \
	{.chip = _chip, .hci = _hci, .addr = _addr, .u.vm.mask = _mask, .u.vm.val = 0, .u.vm.equal = 0, .trace = _trace, .tag = _tag, .type = RTW_IO_SNIFF_TYPE_VALUE,}

/* part or all sniffed range is disabled (not all 1) */
#define RTW_IO_SNIFF_DIS_ENT(_chip, _hci, _addr, _mask, _trace, _tag) \
	{.chip = _chip, .hci = _hci, .addr = _addr, .u.vm.mask = _mask, .u.vm.val = 0xFFFFFFFF, .u.vm.equal = 0, .trace = _trace, .tag = _tag, .type = RTW_IO_SNIFF_TYPE_VALUE,}

const struct rtw_io_sniff_ent read_sniff[] = {
#ifdef DBG_IO_HCI_EN_CHK
	RTW_IO_SNIFF_EN_ENT(MAX_CHIP_TYPE, RTW_SDIO, 0x02, 0x1FC, 1, "SDIO 0x02[8:2] not all 0"),
	RTW_IO_SNIFF_EN_ENT(MAX_CHIP_TYPE, RTW_USB, 0x02, 0x1E0, 1, "USB 0x02[8:5] not all 0"),
	RTW_IO_SNIFF_EN_ENT(MAX_CHIP_TYPE, RTW_PCIE, 0x02, 0x01C, 1, "PCI 0x02[4:2] not all 0"),
#endif
#ifdef DBG_IO_SNIFF_EXAMPLE
	RTW_IO_SNIFF_RANGE_ENT(MAX_CHIP_TYPE, 0, 0x522, 0x522, 0, "read TXPAUSE"),
	RTW_IO_SNIFF_DIS_ENT(MAX_CHIP_TYPE, 0, 0x02, 0x3, 0, "0x02[1:0] not all 1"),
#endif
#ifdef DBG_IO_PROT_SEL
	RTW_IO_SNIFF_RANGE_PROT_ENT(MAX_CHIP_TYPE, 0, 0x1501, 0x1513, rtw_assert_protsel_port, "protsel port"),
	RTW_IO_SNIFF_RANGE_PROT_ENT(MAX_CHIP_TYPE, 0, 0x153a, 0x153b, rtw_assert_protsel_atimdtim, "protsel atimdtim"),
#endif
};

const int read_sniff_num = sizeof(read_sniff) / sizeof(struct rtw_io_sniff_ent);

const struct rtw_io_sniff_ent write_sniff[] = {
#ifdef DBG_IO_HCI_EN_CHK
	RTW_IO_SNIFF_EN_ENT(MAX_CHIP_TYPE, RTW_SDIO, 0x02, 0x1FC, 1, "SDIO 0x02[8:2] not all 0"),
	RTW_IO_SNIFF_EN_ENT(MAX_CHIP_TYPE, RTW_USB, 0x02, 0x1E0, 1, "USB 0x02[8:5] not all 0"),
	RTW_IO_SNIFF_EN_ENT(MAX_CHIP_TYPE, RTW_PCIE, 0x02, 0x01C, 1, "PCI 0x02[4:2] not all 0"),
#endif
#ifdef DBG_IO_8822C_1TX_PATH_EN
	RTW_IO_SNIFF_VALUE_ENT(RTL8822C, 0, 0x1a04, 0xc0000000, 0x02, 1, 0, "write tx_path_en_cck A enabled"),
	RTW_IO_SNIFF_VALUE_ENT(RTL8822C, 0, 0x1a04, 0xc0000000, 0x01, 1, 0, "write tx_path_en_cck B enabled"),
	RTW_IO_SNIFF_VALUE_ENT(RTL8822C, 0, 0x1a04, 0xc0000000, 0x03, 1, 1, "write tx_path_en_cck AB enabled"),
	RTW_IO_SNIFF_VALUE_ENT(RTL8822C, 0, 0x820, 0x03, 0x01, 1, 0, "write tx_path_en_ofdm_1sts A enabled"),
	RTW_IO_SNIFF_VALUE_ENT(RTL8822C, 0, 0x820, 0x03, 0x02, 1, 0, "write tx_path_en_ofdm_1sts B enabled"),
	RTW_IO_SNIFF_VALUE_ENT(RTL8822C, 0, 0x820, 0x03, 0x03, 1, 1, "write tx_path_en_ofdm_1sts AB enabled"),
	RTW_IO_SNIFF_VALUE_ENT(RTL8822C, 0, 0x820, 0x30, 0x01, 1, 0, "write tx_path_en_ofdm_2sts A enabled"),
	RTW_IO_SNIFF_VALUE_ENT(RTL8822C, 0, 0x820, 0x30, 0x02, 1, 0, "write tx_path_en_ofdm_2sts B enabled"),
	RTW_IO_SNIFF_VALUE_ENT(RTL8822C, 0, 0x820, 0x30, 0x03, 1, 1, "write tx_path_en_ofdm_2sts AB enabled"),
#endif
#ifdef DBG_IO_SNIFF_EXAMPLE
	RTW_IO_SNIFF_RANGE_ENT(MAX_CHIP_TYPE, 0, 0x522, 0x522, 0, "write TXPAUSE"),
	RTW_IO_SNIFF_DIS_ENT(MAX_CHIP_TYPE, 0, 0x02, 0x3, 0, "0x02[1:0] not all 1"),
#endif
};

const int write_sniff_num = sizeof(write_sniff) / sizeof(struct rtw_io_sniff_ent);

static bool match_io_sniff_ranges(_adapter *adapter
	, const struct rtw_io_sniff_ent *sniff, int i, u32 addr, u16 len)
{

	/* check if IO range after sniff end address */
	if (addr > sniff->u.end_addr)
		return 0;

	if (sniff->assert_protsel &&
	    sniff->assert_protsel(adapter, addr, len))
		return 0;

	return 1;
}

static bool match_io_sniff_value(_adapter *adapter
	, const struct rtw_io_sniff_ent *sniff, int i, u32 addr, u8 len, u32 val)
{
	u8 sniff_len;
	s8 mask_shift;
	u32 mask;
	s8 value_shift;
	u32 value;
	bool ret = 0;

	/* check if IO range after sniff end address */
	sniff_len = 4;
	while (!(sniff->u.vm.mask & (0xFF << ((sniff_len - 1) * 8)))) {
		sniff_len--;
		if (sniff_len == 0)
			goto exit;
	}
	if (sniff->addr + sniff_len <= addr)
		goto exit;

	/* align to IO addr */
	mask_shift = (sniff->addr - addr) * 8;
	value_shift = mask_shift + bitshift(sniff->u.vm.mask);
	if (mask_shift > 0)
		mask = sniff->u.vm.mask << mask_shift;
	else if (mask_shift < 0)
		mask = sniff->u.vm.mask >> -mask_shift;
	else
		mask = sniff->u.vm.mask;

	if (value_shift > 0)
		value = sniff->u.vm.val << value_shift;
	else if (mask_shift < 0)
		value = sniff->u.vm.val >> -value_shift;
	else
		value = sniff->u.vm.val;

	if ((sniff->u.vm.equal && (mask & val) == (mask & value))
		|| (!sniff->u.vm.equal && (mask & val) != (mask & value))
	) {
		ret = 1;
		if (0)
			RTW_INFO(FUNC_ADPT_FMT" addr:0x%x len:%u val:0x%x (i:%d sniff_len:%u m_shift:%d mask:0x%x v_shifd:%d value:0x%x equal:%d)\n"
				, FUNC_ADPT_ARG(adapter), addr, len, val, i, sniff_len, mask_shift, mask, value_shift, value, sniff->u.vm.equal);
	}

exit:
	return ret;
}

static bool match_io_sniff(_adapter *adapter
	, const struct rtw_io_sniff_ent *sniff, int i, u32 addr, u8 len, u32 val)
{
	bool ret = 0;

	if (sniff->chip != MAX_CHIP_TYPE
		&& sniff->chip != rtw_get_chip_type(adapter))
		goto exit;
	if (sniff->hci
		&& !(sniff->hci & rtw_get_intf_type(adapter)))
		goto exit;
	if (sniff->addr >= addr + len) /* IO range below sniff start address */
		goto exit;

	switch (sniff->type) {
	case RTW_IO_SNIFF_TYPE_RANGE:
		ret = match_io_sniff_ranges(adapter, sniff, i, addr, len);
		break;
	case RTW_IO_SNIFF_TYPE_VALUE:
		if (len == 1 || len == 2 || len == 4)
			ret = match_io_sniff_value(adapter, sniff, i, addr, len, val);
		break;
	default:
		rtw_warn_on(1);
		break;
	}

exit:
	return ret;
}

u32 match_read_sniff(_adapter *adapter, u32 addr, u16 len, u32 val)
{
	int i;
	bool trace = 0;
	u32 match = 0;

	for (i = 0; i < read_sniff_num; i++) {
		if (match_io_sniff(adapter, &read_sniff[i], i, addr, len, val)) {
			match++;
			trace |= read_sniff[i].trace;
			if (read_sniff[i].tag)
				RTW_INFO("DBG_IO TAG %s\n", read_sniff[i].tag);
		}
	}

	rtw_warn_on(trace);

	return match;
}

u32 match_write_sniff(_adapter *adapter, u32 addr, u16 len, u32 val)
{
	int i;
	bool trace = 0;
	u32 match = 0;

	for (i = 0; i < write_sniff_num; i++) {
		if (match_io_sniff(adapter, &write_sniff[i], i, addr, len, val)) {
			match++;
			trace |= write_sniff[i].trace;
			if (write_sniff[i].tag)
				RTW_INFO("DBG_IO TAG %s\n", write_sniff[i].tag);
		}
	}

	rtw_warn_on(trace);

	return match;
}

struct rf_sniff_ent {
	u8 path;
	u16 reg;
	u32 mask;
};

struct rf_sniff_ent rf_read_sniff_ranges[] = {
	/* example for all path addr 0x55 with all RF Reg mask */
	/* {MAX_RF_PATH, 0x55, bRFRegOffsetMask}, */
};

struct rf_sniff_ent rf_write_sniff_ranges[] = {
	/* example for all path addr 0x55 with all RF Reg mask */
	/* {MAX_RF_PATH, 0x55, bRFRegOffsetMask}, */
};

int rf_read_sniff_num = sizeof(rf_read_sniff_ranges) / sizeof(struct rf_sniff_ent);
int rf_write_sniff_num = sizeof(rf_write_sniff_ranges) / sizeof(struct rf_sniff_ent);

bool match_rf_read_sniff_ranges(_adapter *adapter, u8 path, u32 addr, u32 mask)
{
	int i;

	for (i = 0; i < rf_read_sniff_num; i++) {
		if (rf_read_sniff_ranges[i].path == MAX_RF_PATH || rf_read_sniff_ranges[i].path == path)
			if (addr == rf_read_sniff_ranges[i].reg && (mask & rf_read_sniff_ranges[i].mask))
				return _TRUE;
	}

	return _FALSE;
}

bool match_rf_write_sniff_ranges(_adapter *adapter, u8 path, u32 addr, u32 mask)
{
	int i;

	for (i = 0; i < rf_write_sniff_num; i++) {
		if (rf_write_sniff_ranges[i].path == MAX_RF_PATH || rf_write_sniff_ranges[i].path == path)
			if (addr == rf_write_sniff_ranges[i].reg && (mask & rf_write_sniff_ranges[i].mask))
				return _TRUE;
	}

	return _FALSE;
}

void dbg_rtw_reg_read_monitor(_adapter *adapter, u32 addr, u32 len, u32 val, const char *caller, const int line)
{
	if (match_read_sniff(adapter, addr, len, val)) {
		switch (len) {
		case 1:
			RTW_INFO("DBG_IO %s:%d read8(0x%04x) return 0x%02x\n"
				, caller, line, addr, val);
			break;
		case 2:
			RTW_INFO("DBG_IO %s:%d read16(0x%04x) return 0x%04x\n"
				, caller, line, addr, val);
			break;
		case 4:
			RTW_INFO("DBG_IO %s:%d read32(0x%04x) return 0x%08x\n"
				, caller, line, addr, val);
			break;
		default:
			RTW_INFO("DBG_IO %s:%d readN(0x%04x, %u)\n"
				, caller, line, addr, len);
		}
	}
}

void dbg_rtw_reg_write_monitor(_adapter *adapter, u32 addr, u32 len, u32 val, const char *caller, const int line)
{
	if (match_write_sniff(adapter, addr, len, val)) {
		switch (len) {
		case 1:
			RTW_INFO("DBG_IO %s:%d write8(0x%04x, 0x%02x)\n"
				, caller, line, addr, val);
			break;
		case 2:
			RTW_INFO("DBG_IO %s:%d write16(0x%04x, 0x%04x)\n"
				, caller, line, addr, val);
			break;
		case 4:
			RTW_INFO("DBG_IO %s:%d write32(0x%04x, 0x%08x)\n"
				, caller, line, addr, val);
			break;
		default:
			RTW_INFO("DBG_IO %s:%d rtw_writeN(0x%04x, %u)\n"
				, caller, line, addr, len);
		}
	}
}


#endif
