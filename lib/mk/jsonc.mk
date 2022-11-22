JSONC_DIR := $(call select_from_ports,jsonc)/src/lib/jsonc

SHARED_LIB := yes
include $(REP_DIR)/lib/import/import-jsonc.mk

SRC_C = arraylist.c \
	debug.c \
	json_c_version.c \
	json_object.c \
	json_object_iterator.c \
	json_pointer.c \
	json_tokener.c \
	json_util.c \
	json_visit.c \
	linkhash.c \
	printbuf.c \
	random_seed.c \
	strerror_override.c

vpath %.c $(JSONC_DIR)

INC_DIR += $(JSONC_DIR)

LIBS += libc
