LIBS = libretro libc libm stdcxx

SHARED_LIB = yes

PORT_DIR := $(call select_from_ports,tyrquake-libretro)/src/lib/tyrquake-libretro
CORE_DIR := $(PORT_DIR)

-include $(PORT_DIR)/Makefile.common

INC_DIR += \
	$(REP_DIR)/src/libretro/tyrquake \
	$(PORT_DIR)/libretro-common/include

CC_OPT = \
	-DHAVE_STRINGS_H \
	-DHAVE_STDINT_H \
	-DHAVE_INTTYPES_H \
	-D__LIBRETRO__ \
	-DINLINE=inline \
	-DNQ_HACK \
	-DQBASEDIR=$(CORE_DIR) \
	-DTYR_VERSION=0.62

SRC_C  := $(notdir $(SOURCES_C))

vpath %.c   $(sort $(dir $(SOURCES_C)))
