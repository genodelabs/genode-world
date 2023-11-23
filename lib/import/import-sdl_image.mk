SDL_IMAGE_PORT_DIR := $(call select_from_ports,sdl_image)
INC_DIR += $(addprefix $(SDL_IMAGE_PORT_DIR)/,include include/SDL)
