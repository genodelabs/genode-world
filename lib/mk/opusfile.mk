include $(REP_DIR)/lib/import/import-opusfile.mk

OPUSFILE_SRC_DIR = $(OPUSFILE_PORT_DIR)/src/lib/opusfile/src

SRC_C = info.c internal.c opusfile.c stream.c

LIBS += libc libogg opus

vpath %.c $(OPUSFILE_SRC_DIR)
