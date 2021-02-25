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

#ifndef _RTW_IO_H_
#define _RTW_IO_H_

#define NUM_IOREQ		8

#ifdef PLATFORM_LINUX
	#define MAX_PROT_SZ	(64-16)
#endif

#define _IOREADY			0
#define _IO_WAIT_COMPLETE   1
#define _IO_WAIT_RSP        2

/* IO COMMAND TYPE */
#define _IOSZ_MASK_		(0x7F)
#define _IO_WRITE_		BIT(7)
#define _IO_FIXED_		BIT(8)
#define _IO_BURST_		BIT(9)
#define _IO_BYTE_		BIT(10)
#define _IO_HW_			BIT(11)
#define _IO_WORD_		BIT(12)
#define _IO_SYNC_		BIT(13)
#define _IO_CMDMASK_	(0x1F80)


/*
	For prompt mode accessing, caller shall free io_req
	Otherwise, io_handler will free io_req
*/



/* IO STATUS TYPE */
#define _IO_ERR_		BIT(2)
#define _IO_SUCCESS_	BIT(1)
#define _IO_DONE_		BIT(0)


#define IO_RD32			(_IO_SYNC_ | _IO_WORD_)
#define IO_RD16			(_IO_SYNC_ | _IO_HW_)
#define IO_RD8			(_IO_SYNC_ | _IO_BYTE_)

#define IO_RD32_ASYNC	(_IO_WORD_)
#define IO_RD16_ASYNC	(_IO_HW_)
#define IO_RD8_ASYNC	(_IO_BYTE_)

#define IO_WR32			(_IO_WRITE_ | _IO_SYNC_ | _IO_WORD_)
#define IO_WR16			(_IO_WRITE_ | _IO_SYNC_ | _IO_HW_)
#define IO_WR8			(_IO_WRITE_ | _IO_SYNC_ | _IO_BYTE_)

#define IO_WR32_ASYNC	(_IO_WRITE_ | _IO_WORD_)
#define IO_WR16_ASYNC	(_IO_WRITE_ | _IO_HW_)
#define IO_WR8_ASYNC	(_IO_WRITE_ | _IO_BYTE_)

/*

	Only Sync. burst accessing is provided.

*/

#define IO_WR_BURST(x)		(_IO_WRITE_ | _IO_SYNC_ | _IO_BURST_ | ((x) & _IOSZ_MASK_))
#define IO_RD_BURST(x)		(_IO_SYNC_ | _IO_BURST_ | ((x) & _IOSZ_MASK_))



/* below is for the intf_option bit defition... */

#define _INTF_ASYNC_	BIT(0)	/* support async io */

struct intf_priv;
struct intf_hdl;
struct io_queue;

struct _io_ops {
	u32(*_write_port)(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *pmem);
	void (*_write_port_cancel)(struct intf_hdl *pintfhdl);
};

struct io_req {
	_list	list;
	u32	addr;
	volatile u32	val;
	u32	command;
	u32	status;
	u8	*pbuf;
	_sema	sema;
	void (*_async_io_callback)(_adapter *padater, struct io_req *pio_req, u8 *cnxt);
	u8 *cnxt;
};

struct	intf_hdl {
	_adapter *padapter;
	struct dvobj_priv *pintf_dev;/*	pointer to &(padapter->dvobjpriv); */
	struct _io_ops	io_ops;
};

struct reg_protocol_rd {

#ifdef CONFIG_LITTLE_ENDIAN

	/* DW1 */
	u32		NumOfTrans:4;
	u32		Reserved1:4;
	u32		Reserved2:24;
	/* DW2 */
	u32		ByteCount:7;
	u32		WriteEnable:1;		/* 0:read, 1:write */
	u32		FixOrContinuous:1;	/* 0:continuous, 1: Fix */
	u32		BurstMode:1;
	u32		Byte1Access:1;
	u32		Byte2Access:1;
	u32		Byte4Access:1;
	u32		Reserved3:3;
	u32		Reserved4:16;
	/* DW3 */
	u32		BusAddress;
	/* DW4 */
	/* u32		Value; */
#else


	/* DW1 */
	u32 Reserved1:4;
	u32 NumOfTrans:4;

	u32 Reserved2:24;

	/* DW2 */
	u32 WriteEnable:1;
	u32 ByteCount:7;


	u32 Reserved3:3;
	u32 Byte4Access:1;

	u32 Byte2Access:1;
	u32 Byte1Access:1;
	u32 BurstMode:1;
	u32 FixOrContinuous:1;

	u32 Reserved4:16;

	/* DW3 */
	u32		BusAddress;

	/* DW4 */
	/* u32		Value; */

#endif

};


struct reg_protocol_wt {


#ifdef CONFIG_LITTLE_ENDIAN

	/* DW1 */
	u32		NumOfTrans:4;
	u32		Reserved1:4;
	u32		Reserved2:24;
	/* DW2 */
	u32		ByteCount:7;
	u32		WriteEnable:1;		/* 0:read, 1:write */
	u32		FixOrContinuous:1;	/* 0:continuous, 1: Fix */
	u32		BurstMode:1;
	u32		Byte1Access:1;
	u32		Byte2Access:1;
	u32		Byte4Access:1;
	u32		Reserved3:3;
	u32		Reserved4:16;
	/* DW3 */
	u32		BusAddress;
	/* DW4 */
	u32		Value;

#else
	/* DW1 */
	u32 Reserved1:4;
	u32 NumOfTrans:4;

	u32 Reserved2:24;

	/* DW2 */
	u32 WriteEnable:1;
	u32 ByteCount:7;

	u32 Reserved3:3;
	u32 Byte4Access:1;

	u32 Byte2Access:1;
	u32 Byte1Access:1;
	u32 BurstMode:1;
	u32 FixOrContinuous:1;

	u32 Reserved4:16;

	/* DW3 */
	u32		BusAddress;

	/* DW4 */
	u32		Value;

#endif

};
#ifdef CONFIG_PCI_HCI
#define MAX_CONTINUAL_IO_ERR 4
#endif

#ifdef CONFIG_USB_HCI
#define MAX_CONTINUAL_IO_ERR 4
#endif

#ifdef CONFIG_SDIO_HCI
#define SD_IO_TRY_CNT (8)
#define MAX_CONTINUAL_IO_ERR SD_IO_TRY_CNT
#endif

#ifdef CONFIG_GSPI_HCI
#define SD_IO_TRY_CNT (8)
#define MAX_CONTINUAL_IO_ERR SD_IO_TRY_CNT
#endif


int rtw_inc_and_chk_continual_io_error(struct dvobj_priv *dvobj);
void rtw_reset_continual_io_error(struct dvobj_priv *dvobj);

/*
Below is the data structure used by _io_handler

*/

struct io_queue {
	_lock	lock;
	_list	free_ioreqs;
	_list		pending;		/* The io_req list that will be served in the single protocol read/write.	 */
	_list		processing;
	u8	*free_ioreqs_buf; /* 4-byte aligned */
	u8	*pallocated_free_ioreqs_buf;
	struct	intf_hdl	intf;
};

struct io_priv {

	_adapter *padapter;

	struct intf_hdl intf;

};

extern uint ioreq_flush(_adapter *adapter, struct io_queue *ioqueue);
extern void sync_ioreq_enqueue(struct io_req *preq, struct io_queue *ioqueue);
extern uint sync_ioreq_flush(_adapter *adapter, struct io_queue *ioqueue);

extern uint free_ioreq(struct io_req *preq, struct io_queue *pio_queue);
extern struct io_req *alloc_ioreq(struct io_queue *pio_q);

extern uint register_intf_hdl(u8 *dev, struct intf_hdl *pintfhdl);
extern void unregister_intf_hdl(struct intf_hdl *pintfhdl);

extern void _rtw_attrib_read(_adapter *adapter, u32 addr, u32 cnt, u8 *pmem);
extern void _rtw_attrib_write(_adapter *adapter, u32 addr, u32 cnt, u8 *pmem);

extern void _rtw_write_port_cancel(_adapter *adapter);

#define rtw_read8(adapter, addr) rtw_phl_read8(adapter_to_dvobj((_adapter *)(adapter))->phl, (addr))
#define rtw_read16(adapter, addr) rtw_phl_read16(adapter_to_dvobj((_adapter *)(adapter))->phl, (addr))
#define rtw_read32(adapter, addr) rtw_phl_read32(adapter_to_dvobj((_adapter *)(adapter))->phl, (addr))
#define rtw_write8(adapter, addr, val) rtw_phl_write8(adapter_to_dvobj((_adapter *)(adapter))->phl, (addr), (val))
#define rtw_write16(adapter, addr, val) rtw_phl_write16(adapter_to_dvobj((_adapter *)(adapter))->phl, (addr), (val))
#define rtw_write32(adapter, addr, val) rtw_phl_write32(adapter_to_dvobj((_adapter *)(adapter))->phl, (addr), (val))

#define rtw_write_port_cancel(adapter) _rtw_write_port_cancel((adapter))

extern void rtw_write_scsi(_adapter *adapter, u32 cnt, u8 *pmem);

/* ioreq */
extern void ioreq_read8(_adapter *adapter, u32 addr, u8 *pval);
extern void ioreq_read16(_adapter *adapter, u32 addr, u16 *pval);
extern void ioreq_read32(_adapter *adapter, u32 addr, u32 *pval);
extern void ioreq_write8(_adapter *adapter, u32 addr, u8 val);
extern void ioreq_write16(_adapter *adapter, u32 addr, u16 val);
extern void ioreq_write32(_adapter *adapter, u32 addr, u32 val);


extern uint async_read8(_adapter *adapter, u32 addr, u8 *pbuff,
	void (*_async_io_callback)(_adapter *padater, struct io_req *pio_req, u8 *cnxt), u8 *cnxt);
extern uint async_read16(_adapter *adapter, u32 addr,  u8 *pbuff,
	void (*_async_io_callback)(_adapter *padater, struct io_req *pio_req, u8 *cnxt), u8 *cnxt);
extern uint async_read32(_adapter *adapter, u32 addr,  u8 *pbuff,
	void (*_async_io_callback)(_adapter *padater, struct io_req *pio_req, u8 *cnxt), u8 *cnxt);

extern void async_read_mem(_adapter *adapter, u32 addr, u32 cnt, u8 *pmem);
extern void async_read_port(_adapter *adapter, u32 addr, u32 cnt, u8 *pmem);

extern void async_write8(_adapter *adapter, u32 addr, u8 val,
	void (*_async_io_callback)(_adapter *padater, struct io_req *pio_req, u8 *cnxt), u8 *cnxt);
extern void async_write16(_adapter *adapter, u32 addr, u16 val,
	void (*_async_io_callback)(_adapter *padater, struct io_req *pio_req, u8 *cnxt), u8 *cnxt);
extern void async_write32(_adapter *adapter, u32 addr, u32 val,
	void (*_async_io_callback)(_adapter *padater, struct io_req *pio_req, u8 *cnxt), u8 *cnxt);

extern void async_write_mem(_adapter *adapter, u32 addr, u32 cnt, u8 *pmem);
extern void async_write_port(_adapter *adapter, u32 addr, u32 cnt, u8 *pmem);


int rtw_init_io_priv(_adapter *padapter, void (*set_intf_ops)(_adapter *padapter, struct _io_ops *pops));


extern uint alloc_io_queue(_adapter *adapter);
extern void free_io_queue(_adapter *adapter);
extern void async_bus_io(struct io_queue *pio_q);
extern void bus_sync_io(struct io_queue *pio_q);
extern u32 _ioreq2rwmem(struct io_queue *pio_q);

/*
#define RTL_R8(reg)		rtw_read8(padapter, reg)
#define RTL_R16(reg)            rtw_read16(padapter, reg)
#define RTL_R32(reg)            rtw_read32(padapter, reg)
#define RTL_W8(reg, val8)       rtw_write8(padapter, reg, val8)
#define RTL_W16(reg, val16)     rtw_write16(padapter, reg, val16)
#define RTL_W32(reg, val32)     rtw_write32(padapter, reg, val32)
*/

/*
#define RTL_W8_ASYNC(reg, val8) rtw_write32_async(padapter, reg, val8)
#define RTL_W16_ASYNC(reg, val16) rtw_write32_async(padapter, reg, val16)
#define RTL_W32_ASYNC(reg, val32) rtw_write32_async(padapter, reg, val32)

#define RTL_WRITE_BB(reg, val32)	phy_SetUsbBBReg(padapter, reg, val32)
#define RTL_READ_BB(reg)	phy_QueryUsbBBReg(padapter, reg)
*/

#define PlatformEFIOWrite1Byte(_a, _b, _c)		\
	rtw_write8(_a, _b, _c)
#define PlatformEFIOWrite2Byte(_a, _b, _c)		\
	rtw_write16(_a, _b, _c)
#define PlatformEFIOWrite4Byte(_a, _b, _c)		\
	rtw_write32(_a, _b, _c)

#define PlatformEFIORead1Byte(_a, _b)		\
	rtw_read8(_a, _b)
#define PlatformEFIORead2Byte(_a, _b)		\
	rtw_read16(_a, _b)
#define PlatformEFIORead4Byte(_a, _b)		\
	rtw_read32(_a, _b)

#endif /* _RTL8711_IO_H_ */
