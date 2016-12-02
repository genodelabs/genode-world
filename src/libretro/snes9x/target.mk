TARGET = snes9x
LIBS   = snes9x_libretro
SRC_CC = main.cc

vpath %.cc $(call select_from_repositories,src/test/libports)
