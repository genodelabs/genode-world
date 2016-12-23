TARGET = ltris
LIBS   = posix stdcxx sdl sdl_mixer

LTRIS_DIR := $(call select_from_ports,ltris)/src/app/ltris

# determine version by looking at the configure script
VERSION := $(shell grep "^ VERSION" $(LTRIS_DIR)/configure | sed "s/.*=//")

CC_OPT += -DHI_DIR=\"/var\" -DSRC_DIR=\"/\" \
          -DVERSION=\"$(VERSION)\"

INC_DIR   += $(LTRIS_DIR)/src
SRC_C     := $(notdir $(wildcard $(LTRIS_DIR)/src/*.c))

vpath %.c $(LTRIS_DIR)/src

$(TARGET): ltris_data.tar
ltris_data.tar:
	$(VERBOSE)cd $(LTRIS_DIR)/src; tar cf $(PWD)/bin/$@ gfx sounds figures
