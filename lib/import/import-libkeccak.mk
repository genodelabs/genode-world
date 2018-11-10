XKCP_SRC_DIR := $(call select_from_ports,xkcp)

ifeq ($(filter-out $(SPECS),64bit),)
	INC_DIR += $(XKCP_SRC_DIR)/generic64/libkeccak.a
endif

ifeq ($(filter-out $(SPECS),32bit),)
	INC_DIR += $(XKCP_SRC_DIR)/generic32/libkeccak.a
endif
