SHARED_LIB = 1
LIBS += libc

include $(REP_DIR)/lib/import/import-libmpg123.mk

vpath %.c $(MPG123_SRC_DIR)
vpath %.c $(MPG123_SRC_DIR)/../compat
vpath %.S $(MPG123_SRC_DIR)

INC_DIR += $(MPG123_SRC_DIR)
INC_DIR += $(MPG123_SRC_DIR)/..
INC_DIR += $(MPG123_SRC_DIR)/../compat
INC_DIR += $(REP_DIR)/src/lib/mpg123

CC_DEF += -DOPT_MULTI -DOPT_GENERIC -DOPT_GENERIC_DITHER -DREAL_IS_FLOAT
CC_DEF += -fomit-frame-pointer -funroll-all-loops -finline-functions -ffast-math

SRC_C += frame.c tabinit.c libmpg123.c synth.c dither.c stringbuf.c readers.c icy2utf8.c layer1.c ntom.c dct64.c optimize.c equalizer.c layer2.c layer3.c lfs_alias.c icy.c format.c parse.c synth_8bit.c id3.c synth_real.c synth_s32.c index.c feature.c

SRC_C += compat.c compat_str.c
