TARGET := scrcpy

SCRCPY_DIR := $(call select_from_ports,scrcpy)/src/app/scrcpy

SRC_C  = $(notdir $(wildcard $(SCRCPY_DIR)/app/src/*.c))
SRC_C += $(addprefix util/, $(notdir $(wildcard $(SCRCPY_DIR)/app/src/util/*.c)))
SRC_C += sys/unix/command.c

#sys/unix/command.c - avoid missing strdup and strtok_r
CC_OPT_sys/unix/command += -include sys/cdefs.h

#app/src/cli.c - avoid undefined reference to `__inet_network'
CC_OPT_cli += -D_C11_SOURCE

INC_DIR += $(PRG_DIR)
INC_DIR += $(SCRCPY_DIR)/app/src

LIBS += libc avformat avutil avcodec sdl2

vpath %.c $(SCRCPY_DIR)/app/src
