include $(REP_DIR)/lib/import/import-libsndfile.mk

SNDFILE_SRC_DIR := $(SNDFILE_PORT_DIR)/src/lib/libsndfile/src

LIBS = libc

CC_C_OPT += -std=c11

ALAC_SOURCES = \
	ALAC/ALACBitUtilities.c ALAC/ag_dec.c \
	ALAC/ag_enc.c ALAC/dp_dec.c ALAC/dp_enc.c ALAC/matrix_dec.c \
	ALAC/matrix_enc.c ALAC/alac_decoder.c ALAC/alac_encoder.c

SRC_C := $(notdir $(ALAC_SOURCES))

INC_DIR += $(SNDFILE_SRC_DIR) $(REP_DIR)/src/lib/libsndfile

vpath %.c $(SNDFILE_SRC_DIR)/ALAC
