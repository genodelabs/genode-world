TARGET := chuck

LIBS = stdcxx libc pthread rtaudio rtmidi liblo libsndfile base libogg libvorbis libFLAC

CHUCK_DIR = $(call select_from_ports,chuck)/src/app/chuck/src

CC_OPT += \
	-D__PLATFORM_GENODE__ \
	-D__DISABLE_MIDI__ \

CC_WARN += -Wno-sign-compare

INC_DIR += $(PRG_DIR) $(CHUCK_DIR)

CHUCK_SRC_C := \
	chuck.tab.c chuck.yy.c util_math.c util_network.c util_raw.c \
	util_xforms.c

CHUCK_SRC_CC := \
	chuck_absyn.cpp chuck_parse.cpp chuck_errmsg.cpp \
	chuck_frame.cpp chuck_symbol.cpp chuck_table.cpp chuck_utils.cpp \
	chuck_vm.cpp chuck_instr.cpp chuck_scan.cpp chuck_type.cpp chuck_emit.cpp \
	chuck_compile.cpp chuck_dl.cpp chuck_oo.cpp chuck_lang.cpp chuck_ugen.cpp \
	chuck_otf.cpp chuck_stats.cpp chuck_bbq.cpp chuck_shell.cpp \
	chuck_console.cpp chuck_globals.cpp chuck_io.cpp chuck_system.cpp \
	digiio_rtaudio.cpp hidio_sdl.cpp \
	midiio_rtmidi.cpp ugen_osc.cpp ugen_filter.cpp \
	ugen_stk.cpp ugen_xxx.cpp ulib_machine.cpp ulib_math.cpp ulib_std.cpp \
	ulib_opsc.cpp ulib_regex.cpp util_buffers.cpp util_console.cpp \
	util_string.cpp util_thread.cpp util_opsc.cpp util_serial.cpp \
	util_hid.cpp uana_xform.cpp uana_extract.cpp

LO_SRC_C := \
	lo/address.c lo/blob.c lo/bundle.c lo/message.c lo/method.c \
    lo/pattern_match.c lo/send.c lo/server.c lo/server_thread.c lo/timetag.c

SRC_C  := $(CHUCK_SRC_C) $(util_sndfile.c)
SRC_CC := $(filter-out util_console.cpp,$(CHUCK_SRC_CC)) dummies.cc component.cc

vpath %.c   $(CHUCK_DIR)
vpath %.cpp $(CHUCK_DIR)
