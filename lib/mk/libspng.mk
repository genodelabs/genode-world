include $(REP_DIR)/lib/import/import-libspng.mk

LIBSPNG_SRC_DIR := $(LIBSPNG_PORT_DIR)/src/lib/libspng/src

SRC_C := common.c decode.c

LIBS += libc zlib

vpath %.c $(LIBSPNG_SRC_DIR)

SHARED_LIB := yes

CC_CXX_WARN_STRICT :=

# Prevent link error with GCC 10, which defaults to -fno-common
CC_OPT = -fcommon
