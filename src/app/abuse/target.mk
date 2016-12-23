TARGET := abuse

ABUSE_DIR := $(call select_from_ports,abuse)/src/app/abuse
ABUSE_SRC := $(ABUSE_DIR)/src

SRC_CC := \
	$(notdir $(wildcard $(ABUSE_SRC)/*.cpp)) \
	$(notdir $(wildcard $(ABUSE_SRC)/lol/*.cpp)) \
	$(notdir $(wildcard $(ABUSE_SRC)/ui/*.cpp)) \
	$(notdir $(wildcard $(ABUSE_SRC)/net/*.cpp)) \
	$(notdir $(wildcard $(ABUSE_SRC)/lisp/*.cpp))


SDLPORT_SRC_CC = $(notdir $(wildcard $(ABUSE_SRC)/sdlport/*.cpp))

SRC_CC += $(SDLPORT_SRC_CC)

vpath %.cpp $(ABUSE_SRC)
vpath %.cpp $(ABUSE_SRC)/lol
vpath %.cpp $(ABUSE_SRC)/ui
vpath %.cpp $(ABUSE_SRC)/lisp
vpath %.cpp $(ABUSE_SRC)/net
vpath %.cpp $(ABUSE_SRC)/sdlport

INC_DIR += $(PRG_DIR) \
	$(ABUSE_SRC) \
	$(ABUSE_SRC)/imlib \
	$(ABUSE_SRC)/lisp \
	$(ABUSE_SRC)/lol \
	$(ABUSE_SRC)/net \
	$(ABUSE_SRC)/ui

LIBS += abuse_imlib posix stdcxx sdl sdl_image sdl_mixer sdl_net

CC_WARN += \
	-Wno-unused-but-set-variable \
	-Wno-delete-non-virtual-dtor \
	-Wno-unused-but-set-variable \
	-Wno-unused-function \
	-Wno-narrowing

CC_OPT += \
	-DHAVE_CONFIG_H \
	-DNO_CHECK \
	-DASSETDIR=\"$(ABUSE_DIR)/data\" \
	-D_GNU_SOURCE=1

.PHONY: abuse.tar

$(TARGET): abuse.tar

abuse.tar:
	$(VERBOSE) tar cf $@ -C $(ABUSE_DIR)/data .
