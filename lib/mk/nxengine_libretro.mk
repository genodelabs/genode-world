LIBS = libretro libc libm stdcxx

SHARED_LIB = yes

PORT_DIR := $(call select_from_ports,nxengine-libretro)/src/lib/nxengine-libretro

CORE_DIR   := $(PORT_DIR)/nxengine
EXTRACTDIR := $(CORE_DIR)/extract-auto

-include $(PORT_DIR)/Makefile.common

CC_OPT = -DFRONTEND_SUPPORTS_RGB565

INC_DIR += \
	$(CORE_DIR) $(CORE_DIR)/graphics \
	$(CORE_DIR)/libretro $(CORE_DIR)/libretro/libretro-common/include \
	$(CORE_DIR)/sdl/include

SRC_C  := $(notdir $(SOURCES_C))
SRC_CC := $(notdir $(SOURCES_CXX))

vpath %.cpp $(sort $(dir $(SOURCES_CXX)))
vpath %.c   $(sort $(dir $(SOURCES_C)))
