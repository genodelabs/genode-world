TARGET := chuck

LIBS += base libc libm pthread stdcxx rtaudio liblo sdl
LIBS += libsndfile libogg libvorbis libFLAC

CHUCK_SRC_DIR = $(call select_from_ports,chuck)/src/app/chuck/src
CHUCK_CORE_DIR = $(CHUCK_SRC_DIR)/core
CHUCK_HOST_DIR = $(CHUCK_SRC_DIR)/host

CC_OPT += \
	-D__PLATFORM_GENODE__ \
	-D__GENODE_AUDIO__ \
	-D__DISABLE_MIDI__ \
	-DCPU_IS_LITTLE_ENDIAN=1 \
	-D__CK_SNDFILE_NATIVE__ \

CC_WARN += -Wno-sign-compare

INC_DIR += $(PRG_DIR) $(CHUCK_CORE_DIR) $(CHUCK_HOST_DIR)

CHUCK_SRC_C := \
	chuck.tab.c chuck.yy.c util_math.c util_network.c util_raw.c \
	util_xforms.c

CHUCK_SRC_CC := $(notdir $(wildcard $(CHUCK_CORE_DIR)/*.cpp))

CHUCK_SRC_CC_FILTER = util_console.cpp

LO_SRC_C := \
	lo/address.c lo/blob.c lo/bundle.c lo/message.c lo/method.c \
    lo/pattern_match.c lo/send.c lo/server.c lo/server_thread.c lo/timetag.c

SRC_C  += $(CHUCK_SRC_C) $(util_sndfile.c)
SRC_CC += $(filter-out $(CHUCK_SRC_CC_FILTER),$(CHUCK_SRC_CC))
SRC_CC += dummies.cc chuck_component.cc chuck_audio.cpp

vpath %.c   $(CHUCK_CORE_DIR)
vpath %.cpp $(CHUCK_CORE_DIR) $(CHUCK_HOST_DIR)

CC_CXX_WARN_STRICT =
