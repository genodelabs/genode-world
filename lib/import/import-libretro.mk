LIBRETRO_PORT_DIR := $(call select_from_ports,libretro)

INC_DIR += $(LIBRETRO_PORT_DIR)/include

SYMBOLS = $(REP_DIR)/lib/symbols/libretro
