SDL_GFX_SRC_DIR := $(call select_from_ports,sdl_gfx)/src/lib/sdl_gfx

SRC_C   = $(notdir $(wildcard $(SDL_GFX_SRC_DIR)/*.c))
LIBS   += sdl libc
CC_OPT += -Wno-unused-but-set-variable \
          -Wno-maybe-uninitialized \
          -Wno-pointer-sign \
          -Wno-comment

vpath %.c $(SDL_GFX_SRC_DIR)

SHARED_LIB = yes
