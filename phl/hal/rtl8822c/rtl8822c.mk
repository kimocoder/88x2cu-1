EXTRA_CFLAGS += -DCONFIG_RTL8822C
IC_NAME := rtl8822c

ifeq ($(CONFIG_MP_INCLUDED), y)
### 8852A Default Enable VHT MP HW TX MODE ###
#EXTRA_CFLAGS += -DCONFIG_MP_VHT_HW_TX_MODE
#CONFIG_MP_VHT_HW_TX_MODE = y
endif

ifeq ($(CONFIG_PHL_ARCH), y)
HAL = phl/hal
else
HAL = hal
endif

ifeq ($(CONFIG_USB_HCI), y)
	FILE_NAME = rtl8822cu
endif
ifeq ($(CONFIG_PCI_HCI), y)
	FILE_NAME = rtl8822ce
endif
ifeq ($(CONFIG_SDIO_HCI), y)
	FILE_NAME = rtl8822cs
endif


_HAL_IC_FILES +=	$(HAL)/$(IC_NAME)/$(IC_NAME)_ops.o
#_HAL_IC_FILES +=	$(HAL)/$(IC_NAME)/$(IC_NAME)_halinit.o \
			$(HAL)/$(IC_NAME)/$(IC_NAME)_mac.o \
			$(HAL)/$(IC_NAME)/$(IC_NAME)_cmd.o \
			$(HAL)/$(IC_NAME)/$(IC_NAME)_phy.o \
			$(HAL)/$(IC_NAME)/$(IC_NAME)_ops.o \
			$(HAL)/$(IC_NAME)/$(IC_NAME)_ps.o \
			$(HAL)/$(IC_NAME)/hal_trx_8852a.o

_HAL_IC_FILES +=	$(HAL)/$(IC_NAME)/$(HCI_NAME)/$(FILE_NAME)_ops.o
#_HAL_IC_FILES +=	$(HAL)/$(IC_NAME)/$(HCI_NAME)/$(FILE_NAME)_halinit.o \
			$(HAL)/$(IC_NAME)/$(HCI_NAME)/$(FILE_NAME)_halmac.o \
			$(HAL)/$(IC_NAME)/$(HCI_NAME)/$(FILE_NAME)_io.o \
			$(HAL)/$(IC_NAME)/$(HCI_NAME)/$(FILE_NAME)_led.o \
			$(HAL)/$(IC_NAME)/$(HCI_NAME)/$(FILE_NAME)_ops.o

#ifeq ($(CONFIG_SDIO_HCI), y)
#_HAL_IC_FILES += $(HAL)/$(IC_NAME)/$(HCI_NAME)/hal_trx_8852as.o
#endif

ifeq ($(CONFIG_USB_HCI), y)
#_HAL_IC_FILES += $(HAL)/$(IC_NAME)/$(HCI_NAME)/hal_trx_8852au.o
endif

ifeq ($(CONFIG_PCI_HCI), y)
_HAL_IC_FILES += $(HAL)/$(IC_NAME)/$(HCI_NAME)/hal_trx_8852ae.o
endif
