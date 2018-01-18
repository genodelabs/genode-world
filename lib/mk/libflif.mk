SHARED_LIB = yes

LIBS += stdcxx libpng zlib

CC_OPT += \
	-std=gnu++11 \
	-DLODEPNG_NO_COMPILE_PNG -DLODEPNG_NO_COMPILE_DISK \
	-DNDEBUG -ftree-vectorize \
	-DINT16_MAX=0x7fff \
	-D__GENODE__ \

FLIF_SRC_DIR = $(call select_from_ports,flif)/src/lib/flif

SRC_CC = \
	flif-interface.cpp \
	chance.cpp \
	symbol.cpp \
	crc32k.cpp \
	image.cpp \
	image-png.cpp \
	image-pnm.cpp \
	image-pam.cpp \
	image-rggb.cpp \
	image-metadata.cpp \
	color_range.cpp \
	factory.cpp \
	common.cpp \
	flif-enc.cpp \
	flif-dec.cpp \
	lodepng.cpp \
	io.cpp \

vpath %.cpp $(FLIF_SRC_DIR)/src
vpath %.cpp $(FLIF_SRC_DIR)/src/image
vpath %.cpp $(FLIF_SRC_DIR)/src/library
vpath %.cpp $(FLIF_SRC_DIR)/src/maniac
vpath %.cpp $(FLIF_SRC_DIR)/src/transform
vpath %.cpp $(FLIF_SRC_DIR)/extern

CC_CXX_WARN_STRICT =
