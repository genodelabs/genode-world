CMUS_DIR := $(call select_from_ports,cmus)/src/app/cmus

SRC_C := ip/flac.c

INC_DIR := $(CMUS_DIR)

LIBS := libc libFLAC

SHARED_LIB := yes

vpath %.c $(CMUS_DIR)
