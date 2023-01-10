LIBBACKTRACE_PORT_DIR := $(call select_from_ports,libgo)
INC_DIR += $(LIB_CACHE_DIR)/libbacktrace
INC_DIR += $(LIBBACKTRACE_PORT_DIR)/include
