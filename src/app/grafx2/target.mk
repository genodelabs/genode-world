TARGET := grafx2

GRAFX2_DIR := $(call select_from_ports,grafx2)/src/app/grafx2/src

SRC_C    = $(notdir $(wildcard $(GRAFX2_DIR)/*.c))
SRC_CC   = dummy.cc

INC_DIR += $(GRAFX2_DIR)

CC_OPT  += -DGENODE -DNOTTF=1

# Prevent link error with GCC 10, which defaults to -fno-common
CC_OPT  += -fcommon

LIBS    += libc libpng sdl sdl_image libm zlib base

CUSTOM_TARGET_DEPS += grafx2_data.tar

BUILD_ARTIFACTS := $(TARGET) grafx2_data.tar

grafx2_data.tar:
	$(VERBOSE)cd $(GRAFX2_DIR)/../; tar cf $(PWD)/bin/$@ share

vpath %.c $(GRAFX2_DIR)

CC_CXX_WARN_STRICT =
