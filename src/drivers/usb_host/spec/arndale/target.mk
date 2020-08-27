DDE_LINUX_DIR = $(BASE_DIR)/../dde_linux

include $(DDE_LINUX_DIR)/src/drivers/usb_host/target.inc

TARGET   = arndale_usb_host_drv
REQUIRES = arm_v7

INC_DIR += $(DDE_LINUX_DIR)/src/drivers/usb_host
INC_DIR += $(DDE_LINUX_DIR)/src/include
INC_DIR += $(DDE_LINUX_DIR)/src/drivers/usb_host/spec/arm
INC_DIR += $(DDE_LINUX_DIR)/src/include/spec/arm
INC_DIR += $(DDE_LINUX_DIR)/src/include/spec/arm_v7

SRC_CC  += spec/arndale/platform.cc

SRC_C   += usb/dwc3/core.c
SRC_C   += usb/dwc3/dwc3-exynos.c
SRC_C   += usb/dwc3/host.c
SRC_C   += usb/host/ehci-exynos.c
SRC_C   += usb/host/xhci-plat.c

CC_OPT  += -DCONFIG_USB_EHCI_TT_NEWSCHED=1
CC_OPT  += -DCONFIG_USB_DWC3_HOST=1
CC_OPT  += -DCONFIG_USB_OTG_UTILS=1
CC_OPT  += -DCONFIG_USB_XHCI_PLATFORM=1
CC_OPT  += -DDWC3_QUIRK

vpath %.cc $(DDE_LINUX_DIR)/src
vpath %    $(DDE_LINUX_DIR)/src/drivers/usb_host
