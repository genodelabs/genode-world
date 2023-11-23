SDL2_IMAGE_PORT_DIR := $(call select_from_ports,sdl2_image)
SRC_DIR := $(SDL2_IMAGE_PORT_DIR)/src/lib/sdl2_image

SRC_C := $(notdir $(wildcard $(SRC_DIR)/IMG*.c))

LIBS += libc libm sdl2 jpeg libpng zlib

REP_INC_DIR += include/SDL2

INC_DIR += $(SDL2_IMAGE_PORT_DIR)/include/SDL2
INC_DIR += $(SRC_DIR)

SUPPORTED_FORMATS = PNG JPG TGA PNM XPM
CC_OPT += $(addprefix -DLOAD_,$(SUPPORTED_FORMATS))

CC_C_OPT += -Wno-missing-braces

vpath %.c $(SRC_DIR)

SHARED_LIB := yes
