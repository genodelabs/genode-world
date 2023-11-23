SDL_TTF_PORT_DIR := $(call select_from_ports,sdl_ttf)
INC_DIR += $(addprefix $(SDL_TTF_PORT_DIR)/,include)
