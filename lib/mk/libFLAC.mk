include $(REP_DIR)/lib/import/import-libFLAC.mk
FLAC_SRC_DIR = $(FLAC_PORT_DIR)/src/lib/flac
LIBFLAC_SRC_DIR = $(FLAC_SRC_DIR)/src/libFLAC

LIBS += libc libogg

SHARED_LIB = yes

CC_OPT += -DHAVE_CONFIG_H

INC_DIR += \
	$(LIBFLAC_SRC_DIR)/include \
	$(FLAC_SRC_DIR)/include

SRC_C = \
	bitmath.c \
	bitreader.c \
	bitwriter.c \
	cpu.c \
	crc.c \
	fixed.c \
	fixed_intrin_sse2.c \
	fixed_intrin_ssse3.c \
	float.c \
	format.c \
	lpc.c \
	lpc_intrin_sse.c \
	lpc_intrin_sse2.c \
	lpc_intrin_sse41.c \
	lpc_intrin_avx2.c \
	md5.c \
	memory.c \
	metadata_iterators.c \
	metadata_object.c \
	stream_decoder.c \
	stream_encoder.c \
	stream_encoder_intrin_sse2.c \
	stream_encoder_intrin_ssse3.c \
	stream_encoder_intrin_avx2.c \
	stream_encoder_framing.c \
	window.c \
	ogg_decoder_aspect.c \
	ogg_encoder_aspect.c \
	ogg_helper.c \
	ogg_mapping.c

vpath %.c $(LIBFLAC_SRC_DIR)

