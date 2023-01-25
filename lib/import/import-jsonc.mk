JSONC_PORT_DIR := $(call select_from_ports,jsonc)

INC_DIR += $(JSONC_PORT_DIR)/include
INC_DIR += $(JSONC_PORT_DIR)/include/json-c
