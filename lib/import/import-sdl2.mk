SDL2_PORT_DIR := $(call select_from_ports,sdl2)

INC_DIR      += $(SDL2_PORT_DIR)/include $(SDL2_PORT_DIR)/include/SDL2
REP_INC_DIR  += include/SDL2
