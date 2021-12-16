LIBBACKTRACE_DIR := $(call select_from_ports,libbacktrace)/src/lib/libbacktrace

#SRC_C = $(notdir $(wildcard $(LIBATOMIC_DIR)/SDLnet*.c))

#vpath %.c $(LIBATOMIC_DIR)

#INC_DIR += $(LIBBACKTRACE_DIR)

LIBS += libc

#SHARED_LIB = no
