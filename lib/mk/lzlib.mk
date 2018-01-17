LZLIB_DIR := $(call select_from_ports,lzlib)
LZLIB_SRC_DIR := $(LZLIB_DIR)/src/lib/lzlib

LIBS += libc

SRC_C = lzlib.c

INC_DIR += $(LZLIB_SRC_DIR)

vpath %.c $(LZLIB_SRC_DIR)

CC_CXX_WARN_STRICT =
