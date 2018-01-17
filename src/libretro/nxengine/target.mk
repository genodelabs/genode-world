TARGET = nxengine
LIBS   = nxengine_libretro
SRC_CC = main.cc

vpath %.cc $(call select_from_repositories,src/test/libports)

CC_CXX_WARN_STRICT =
