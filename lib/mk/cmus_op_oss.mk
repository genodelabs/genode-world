CMUS_DIR := $(call select_from_ports,cmus)/src/app/cmus

SRC_C := op/oss.c debug.c

INC_DIR := $(CMUS_DIR)

LIBS := libc

SHARED_LIB := yes

vpath %.c $(CMUS_DIR)
