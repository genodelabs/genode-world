include $(REP_DIR)/lib/import/import-libogg.mk

LIBOGG_SRC_DIR = $(LIBOGG_PORT_DIR)/src/lib/libogg/src

SRC_C =  framing.c bitwise.c

LIBS += libc

vpath %.c $(LIBOGG_SRC_DIR)

SHARED_LIB = yes
