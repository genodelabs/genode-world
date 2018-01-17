ABUSE_SRC := $(call select_from_ports,abuse)/src/app/abuse/src

LIBS += libc stdcxx sdl_image

SRC_CC := \
	dprint.cpp \
	filesel.cpp \
	filter.cpp \
	fonts.cpp \
	guistat.cpp \
	image.cpp \
	include.cpp \
	input.cpp \
	jrand.cpp \
	jwindow.cpp \
	keys.cpp \
	linked.cpp \
	palette.cpp \
	pcxread.cpp \
	pmenu.cpp \
	scroller.cpp \
	specs.cpp \
	sprite.cpp \
	status.cpp \
	supmorph.cpp \
	tools.cpp \
	transimage.cpp \
	video.cpp

INC_DIR += $(ABUSE_SRC)/imlib $(ABUSE_SRC)

vpath %.cpp $(ABUSE_SRC)/imlib

CC_WARN += \
	-Wno-unused-but-set-variable \
	-Wno-delete-non-virtual-dtor \
	-Wno-unused-but-set-variable \
	-Wno-unused-function \
	-Wno-narrowing

CC_CXX_WARN_STRICT =
