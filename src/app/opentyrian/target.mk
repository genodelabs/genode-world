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

.PHONY: tyrian.tar

$(TARGET): tyrian.tar

tyrian.tar:
	$(VERBOSE) tar cf $@ -C $(OPENTYRIAN_DIR)/tyrian21 .

CC_CXX_WARN_STRICT =
