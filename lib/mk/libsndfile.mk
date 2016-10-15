include $(REP_DIR)/lib/import/import-libsndfile.mk

SNDFILE_SRC_DIR := $(SNDFILE_PORT_DIR)/src/lib/libsndfile/src

LIBS = gsm10 g72x alac libogg libvorbis libFLAC libc

COMMON = \
	common.c file_io.c command.c pcm.c ulaw.c alaw.c float32.c \
	double64.c ima_adpcm.c ms_adpcm.c gsm610.c dwvw.c vox_adpcm.c \
	interleave.c strings.c dither.c cart.c broadcast.c audio_detect.c \
 	ima_oki_adpcm.c alac.c chunk.c ogg.c chanmap.c \
	windows.c id3.c

FILESPECIFIC = \
	sndfile.c aiff.c au.c avr.c caf.c dwd.c flac.c g72x.c htk.c ircam.c \
	macos.c mat4.c mat5.c nist.c paf.c pvf.c raw.c rx2.c sd2.c \
	sds.c svx.c txw.c voc.c wve.c w64.c wavlike.c wav.c xi.c mpc2k.c rf64.c \
	ogg_vorbis.c ogg_speex.c ogg_pcm.c ogg_opus.c

SRC_C := $(COMMON) $(FILESPECIFIC)

INC_DIR += $(SNDFILE_SRC_DIR) $(REP_DIR)/src/lib/libsndfile

vpath %.c $(SNDFILE_SRC_DIR)

SHARED_LIB = yes
