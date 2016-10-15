include $(REP_DIR)/lib/import/import-libsndfile.mk

SNDFILE_SRC_DIR := $(SNDFILE_PORT_DIR)/src/lib/libsndfile/src

LIBS = libc

G72x_SOURCES = \
	G72x/g721.c G72x/g723_16.c G72x/g723_24.c G72x/g723_40.c G72x/g72x.c

SRC_C := $(notdir $(G72x_SOURCES))

vpath %.c $(SNDFILE_SRC_DIR)/G72x

