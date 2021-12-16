LIBBACKTRACE_PORT_DIR := $(call select_from_ports,libbacktrace)
INC_DIR += $(BUILD_BASE_DIR)/lib/libbacktrace
INC_DIR += $(LIBBACKTRACE_PORT_DIR)/include
