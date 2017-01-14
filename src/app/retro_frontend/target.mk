TARGET = retro_frontend
SRC_CC = component.cc
LIBS   = base libc

LIBRETRO_INC := $(call select_from_ports,libretro)/include

INC_DIR += $(LIBRETRO_INC)

component.cc: $(LIBRETRO_INC)/libretro.h
