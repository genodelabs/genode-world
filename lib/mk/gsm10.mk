include $(REP_DIR)/lib/import/import-libsndfile.mk

SNDFILE_SRC_DIR := $(SNDFILE_PORT_DIR)/src/lib/libsndfile/src

LIBS = libc

GSM610_SOURCES = \
	GSM610/add.c GSM610/code.c GSM610/decode.c GSM610/gsm_create.c \
	GSM610/gsm_decode.c GSM610/gsm_destroy.c GSM610/gsm_encode.c \
	GSM610/gsm_option.c GSM610/long_term.c GSM610/lpc.c GSM610/preprocess.c \
	GSM610/rpe.c GSM610/short_term.c GSM610/table.c

SRC_C := $(notdir $(GSM610_SOURCES))

vpath %.c $(SNDFILE_SRC_DIR)/GSM610

