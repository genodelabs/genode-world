TARGET := atari800

ATARI800_DIR := $(call select_from_ports,atari800)/src/app/atari800

FILTER_OUT_SRC := atari_basic.c atari_curses.c atari_falcon.c atari_ps2.c \
                  atari_rpi.c atari_x11.c joycfg.c mkimg.c rdevice.c \
                  sound_falcon.c sound_oss.c sdl/video_gl.c

SRC_C := $(notdir $(wildcard $(ATARI800_DIR)/src/*.c))
SRC_C += $(addprefix sdl/,$(notdir $(wildcard $(ATARI800_DIR)/src/sdl/*.c)))
SRC_C += atari_ntsc/atari_ntsc.c

SRC_C := $(filter-out $(FILTER_OUT_SRC),$(SRC_C))

vpath %.c $(ATARI800_DIR)/src

INC_DIR += $(ATARI800_DIR)/src $(PRG_DIR)

CC_WARN := -Wall -Wno-unused-but-set-variable -Wno-logical-not-parentheses

LIBS += sdl sdlmain zlib libc libpng

$(TARGET): atari800_rom.tar
atari800_rom.tar:
	$(VERBOSE)cd $(ATARI800_DIR)/xformer; tar cf $(PWD)/bin/$@ *.ROM

