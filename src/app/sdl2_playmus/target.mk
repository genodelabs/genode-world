TARGET := sdl2_playmus

SDL2_DIR := $(call select_from_ports,sdl2_mixer)/src/lib/sdl2_mixer

SRC_C := playmus.c

LIBS := libc libm sdl2 sdl2_mixer

vpath % $(SDL2_DIR)

CC_CXX_WARN_STRICT :=
