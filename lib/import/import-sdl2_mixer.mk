SDL2_MIXER_PORT_DIR := $(call select_from_ports,sdl2_mixer)
INC_DIR += $(SDL2_MIXER_PORT_DIR)/include $(SDL2_MIXER_PORT_DIR)/include/SDL2
