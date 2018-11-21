CMUS_DIR := $(call select_from_ports,cmus)/src/app/cmus

SRC_C := ip/mad.c ip/nomad.c

INC_DIR := $(CMUS_DIR)

LIBS := libc libmad

SHARED_LIB := yes

vpath %.c $(CMUS_DIR)
