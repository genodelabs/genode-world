SDL2_TTF_PORT_DIR := $(call select_from_ports,sdl2_ttf)
SRC_DIR := $(SDL2_TTF_PORT_DIR)/src/lib/sdl2_ttf

SRC_C := SDL_ttf.c
LIBS += libc libm freetype sdl2

CC_C_OPT += -Wno-missing-braces

vpath %.c $(SRC_DIR)

SHARED_LIB := yes
