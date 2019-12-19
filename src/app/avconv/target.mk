include $(REP_DIR)/lib/import/import-av.inc

TARGET = avconv
SRC_C  = avconv.c cmdutils.c avconv_opt.c avconv_filter.c
LIBS  += avfilter avformat avcodec avutil avdevice swscale avresample
LIBS  += base posix

CC_WARN += -Wno-parentheses -Wno-switch -Wno-uninitialized \
           -Wno-format-zero-length -Wno-pointer-sign

LIBAV_PORT_DIR := $(call select_from_ports,libav)

# version.h
INC_DIR += $(PRG_DIR)

# config.h
INC_DIR += $(REP_DIR)/src/lib/libav

CC_C_OPT += -DLIBAV_VERSION=\"0.8\"

vpath %.c $(LIBAV_PORT_DIR)/src/lib/libav

CC_CXX_WARN_STRICT =
