LIBS = libretro libc libm stdcxx zlib

SHARED_LIB = yes

CC_OPT += \
	-O3 -D__LIBRETRO__ -DSOUND_QUALITY=0 -DPATH_MAX=1024 -DINLINE=inline -DPSS_STYLE=1 -DFCEU_VERSION_NUMERIC=9813 -DFRONTEND_SUPPORTS_RGB565 -DHAVE_ASPRINTF

CORE_DIR := $(call select_from_ports,fceumm-libretro)/src/libretro/fceumm/src
-include $(CORE_DIR)/../Makefile.common

INC_DIR += \
	$(CORE_DIR)/drivers/libretro \
	$(CORE_DIR) \
	$(CORE_DIR)/input \
	$(CORE_DIR)/boards \
	$(CORE_DIR)/mappers

SRC_CC := $(notdir $(SOURCES_CXX))
SRC_C  := $(notdir $(SOURCES_C))


vpath %.c $(CORE_DIR)
vpath %.c $(CORE_DIR)/boards
vpath %.c $(CORE_DIR)/drivers/libretro
vpath %.c $(CORE_DIR)/drivers/libretro/libretro-common/streams
vpath %.c $(CORE_DIR)/input
vpath %.c $(CORE_DIR)/mappers

