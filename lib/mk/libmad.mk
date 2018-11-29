include $(REP_DIR)/lib/import/import-libmad.mk

LIBMAD_SRC_DIR := $(LIBMAD_PORT_DIR)/src/lib/libmad
LIBMAD_SRC_C   := $(notdir $(wildcard $(LIBMAD_SRC_DIR)/*.c))

SRC_C   := $(filter-out minimad.c,$(LIBMAD_SRC_C))
INC_DIR += $(REP_DIR)/src/lib/libmad

CC_OPT += -DHAVE_CONFIG -DFPM_DEFAULT -DNDEBUG -DSIZEOF_INT=4

LIBS += libc

SHARED_LIB := yes

vpath %.c $(LIBMAD_SRC_DIR)
