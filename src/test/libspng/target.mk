TARGET := test-libspng
SRC_C  := example.c
LIBS   := libc libspng posix

LIBSPNG_DIR := $(call select_from_ports,libspng)

INC_DIR += $(LIBSPNG_DIR)/include/libspng

LIBSPNG_SRC_DIR := $(LIBSPNG_DIR)/src/lib/libspng

vpath %.c $(LIBSPNG_SRC_DIR)/examples
