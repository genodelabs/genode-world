TARGET = sdl_opengl-lesson3
LIBS   = libm libc egl mesa sdl

LD_OPT = --export-dynamic

SRC_CC  = lesson3.cc
SRC_CC += sdl_main.cc

vpath sdl_main.cc $(PRG_DIR)/../

CC_CXX_WARN_STRICT =
