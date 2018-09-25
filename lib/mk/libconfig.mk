include $(REP_DIR)/lib/import/import-libconfig.mk

LIBCONFIG_SRC_DIR := $(LIBCONFIG_PORT_DIR)/src/lib/libconfig/lib

SRC_C := $(notdir $(wildcard $(LIBCONFIG_SRC_DIR)/*.c))

LIBS += libc

SHARED_LIB := yes

vpath %.c $(LIBCONFIG_SRC_DIR)
