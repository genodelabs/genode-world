XKCP_SRC_DIR := $(call select_from_ports,xkcp)
KECCAK_SRC_DIR := $(XKCP_SRC_DIR)/generic32/libkeccak.a

include $(REP_DIR)/lib/mk/libkeccak.inc
