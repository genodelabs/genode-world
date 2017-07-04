LIBS = libretro libc stdcxx

SHARED_LIB = yes

PORT_DIR := $(call select_from_ports,meteor-libretro)/src/lib/meteor-libretro

CORE_DIR := $(PORT_DIR)
-include $(PORT_DIR)/libretro/Makefile.common

CXX_DEF += -D__LIBRETRO__ -DNDEBUG -DFRONTEND_SUPPORTS_RGB565

CC_WARN += -Wno-parentheses

INC_DIR += $(PORT_DIR)/libretro $(PORT_DIR)/ameteor/include

SRC_CC := $(notdir $(SOURCES_CXX))

vpath %.cpp $(sort $(dir $(SOURCES_CXX)))
