LIBS = libretro libc libm stdcxx

SHARED_LIB = yes

CC_OPT += \
	-fno-rtti -pedantic \
	-DHAVE_STRINGS_H -DHAVE_STDINT_H -DRIGHTSHIFT_IS_SAR -D__LIBRETRO__ \
	-O3 -DNDEBUG

CORE_DIR := $(call select_from_ports,snes9x-libretro)/src/libretro/snes9x
-include $(CORE_DIR)/libretro/Makefile.common

INC_DIR += $(CORE_DIR)/libretro $(CORE_DIR) $(CORE_DIR)/apu/ $(CORE_DIR)/apu/bapu

SRC_CC := $(notdir $(SOURCES_CXX))

vpath %.cpp $(CORE_DIR)/apu
vpath %.cpp $(CORE_DIR)/apu/bapu/dsp
vpath %.cpp $(CORE_DIR)/apu/bapu/smp
vpath %.cpp $(CORE_DIR)/libretro
vpath %.cpp $(CORE_DIR)
