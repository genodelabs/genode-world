CMUS_DIR := $(call select_from_ports,cmus)/src/app/cmus

SRC_C := ip/opus.c

INC_DIR := $(CMUS_DIR)

LIBS := libc opus opusfile
# XXX pull in dep for opusfile
LIBS += libogg

SHARED_LIB := yes

vpath %.c $(CMUS_DIR)
