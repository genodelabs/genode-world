TARGET := opentyrian

OPENTYRIAN_DIR := $(call select_from_ports,opentyrian)/src/app/opentyrian
OPENTYRIAN_SRC := $(OPENTYRIAN_DIR)/src

SRC_C := $(notdir $(wildcard $(OPENTYRIAN_SRC)/*.c))

vpath %.c $(OPENTYRIAN_SRC)

LIBS += posix sdl sdl_net

CC_OPT += -std=c99 -DTARGET_UNIX

CC_WARN += -Wno-implicit-function-declaration

.PHONY: tyrian.tar

$(TARGET): tyrian.tar

tyrian.tar:
	$(VERBOSE) tar cf $@ -C $(OPENTYRIAN_DIR)/tyrian21 .
