LIBBACKTRACE_PORT_DIR := $(call select_from_ports,libgo)
INC_DIR += $(LIBBACKTRACE_PORT_DIR)/include

# checked by backtrace.h
CC_OPT += -DHAVE_STDINT_H

# for '_Unwind_Backtrace' and '_Unwind_GetIPInfo'
EXT_OBJECTS += $(shell $(CC) $(CC_MARCH) -print-file-name=libgcc_eh.a)
