TARGET = tyrquake
LIBS   = tyrquake_libretro
SRC_CC = main.cc

vpath %.cc $(call select_from_repositories,src/test/libports)
