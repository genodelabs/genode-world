TARGET := opentyrian

OPENTYRIAN_DIR := $(call select_from_ports,opentyrian)/src/app/opentyrian
OPENTYRIAN_SRC := $(OPENTYRIAN_DIR)/src

SRC_C := $(notdir $(wildcard $(OPENTYRIAN_SRC)/*.c))

vpath %.c $(OPENTYRIAN_SRC)

LIBS += libc libm sdl sdl_net base

CC_OPT += -std=c99 -DTARGET_UNIX

# Prevent link error with GCC 10, which defaults to -fno-common
CC_OPT += -fcommon

CC_WARN += -Wno-implicit-function-declaration

CUSTOM_TARGET_DEPS += tyrian.tar

BUILD_ARTIFACTS := $(TARGET) tyrian.tar

tyrian.tar:
	$(VERBOSE)cd $(OPENTYRIAN_DIR)/tyrian21; tar cf $(PWD)/bin/$@ .

CC_CXX_WARN_STRICT =
