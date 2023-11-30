PORT_DIR := $(call select_from_ports,util-linux)
UTIL_LINUX_DIR := $(PORT_DIR)/src/lib/util-linux
LIBUUID_DIR := $(UTIL_LINUX_DIR)/libuuid/src

INC_DIR += $(LIBUUID_DIR)
INC_DIR += $(UTIL_LINUX_DIR)/include
INC_DIR += $(PORT_DIR)/include
