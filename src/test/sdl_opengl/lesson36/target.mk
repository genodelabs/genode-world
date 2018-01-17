TARGET = sdl_opengl-lesson36
LIBS   = libm libc egl mesa sdl

LD_OPT = --export-dynamic

SRC_C  = main.c error.c lesson36.c
SRC_CC = sdl_main.cc

INC_DIR += $(PRG_DIR)

vpath sdl_main.cc $(PRG_DIR)/../

CC_CXX_WARN_STRICT =
