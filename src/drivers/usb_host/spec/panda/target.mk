DDE_LINUX_DIR = $(BASE_DIR)/../dde_linux

include $(DDE_LINUX_DIR)/src/drivers/usb_host/target.inc

TARGET   = panda_usb_host_drv
REQUIRES = arm_v7

INC_DIR += $(DDE_LINUX_DIR)/src/drivers/usb_host
INC_DIR += $(DDE_LINUX_DIR)/src/include
INC_DIR += $(DDE_LINUX_DIR)/src/drivers/usb_host/spec/arm
INC_DIR += $(DDE_LINUX_DIR)/src/include/spec/arm
INC_DIR += $(DDE_LINUX_DIR)/src/include/spec/arm_v7

SRC_CC  += spec/panda/platform.cc
SRC_C   += usb/host/ehci-omap.c

CC_OPT  += -DCONFIG_USB_EHCI_HCD_OMAP=1
CC_OPT  += -DCONFIG_USB_EHCI_TT_NEWSCHED=1
CC_OPT  += -DCONFIG_EXTCON=1

vpath %.cc $(DDE_LINUX_DIR)/src/lib/legacy
vpath %    $(DDE_LINUX_DIR)/src/drivers/usb_host
