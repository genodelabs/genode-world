TARGET := grafx2

GRAFX2_DIR := $(call select_from_ports,grafx2)/src/app/grafx2/src

SRC_C    = $(notdir $(wildcard $(GRAFX2_DIR)/*.c))
SRC_CC   = dummy.cc

INC_DIR += $(GRAFX2_DIR)

CC_OPT  += -DGENODE -DNOTTF=1

LIBS    += libc libpng sdlmain sdl sdl_image libm zlib base

$(TARGET): grafx2_data.tar
grafx2_data.tar:
	$(VERBOSE)cd $(GRAFX2_DIR)/../; tar cf $(PWD)/bin/$@ share

vpath %.c $(GRAFX2_DIR)

CC_CXX_WARN_STRICT =
