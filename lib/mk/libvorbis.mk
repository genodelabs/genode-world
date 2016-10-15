include $(REP_DIR)/lib/import/import-libvorbis.mk

LIBVORBIS_SRC_DIR = $(LIBVORBIS_PORT_DIR)/src/lib/libvorbis/lib

INC_DIR += $(LIBVORBIS_SRC_DIR)

# filter the test programs
FILTER_OUT = barmel.c tone.c psytune.c

SRC_C = $(filter-out $(FILTER_OUT), $(notdir $(wildcard $(LIBVORBIS_SRC_DIR)/*.c)))
SRC_C += $(notdir $(wildcard $(LIBVORBIS_SRC_DIR)/books/*.c))
SRC_C += $(notdir $(wildcard $(LIBVORBIS_SRC_DIR)/modes/*.c))

LIBS =  libogg libc

vpath %.c $(LIBVORBIS_SRC_DIR)
vpath %.c $(LIBVORBIS_SRC_DIR)/books
vpath %.c $(LIBVORBIS_SRC_DIR)/modes

SHARED_LIB = yes
