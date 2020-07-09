GLIB_PORT_DIR := $(call select_from_ports,glib)
INC_DIR += $(GLIB_PORT_DIR)/include/glib

GLIB_REP_INC_DIR := $(call select_from_repositories,include/glib)

ifeq ($(filter-out $(SPECS),32bit),)
TARGET_MACHINE := 32bit
else ifeq ($(filter-out $(SPECS),64bit),)
TARGET_MACHINE := 64bit
endif

INC_DIR += $(GLIB_REP_INC_DIR) $(GLIB_REP_INC_DIR)/spec/$(TARGET_MACHINE)
