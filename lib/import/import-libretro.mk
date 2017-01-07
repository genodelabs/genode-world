RETROARCH_PORT_DIR := $(call select_from_ports,retroarch)

INC_DIR += $(RETROARCH_PORT_DIR)/include/libretro

SYMBOLS = $(REP_DIR)/lib/symbols/libretro
