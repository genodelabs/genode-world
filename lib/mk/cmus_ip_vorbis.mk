CMUS_DIR := $(call select_from_ports,cmus)/src/app/cmus

SRC_C := ip/vorbis.c

INC_DIR := $(CMUS_DIR)

LIBS := libc libvorbis libogg

SHARED_LIB := yes

vpath %.c $(CMUS_DIR)
