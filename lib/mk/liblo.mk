include $(REP_DIR)/lib/import/import-liblo.mk
LIBLO_SRC_DIR := $(LIBLO_PORT_DIR)/src/lib/liblo/src

LIBS := libc

CC_OPT += \
	-DHAVE_CONFIG_H \
	-D_POSIX_C_SOURCE=200112 \
	-D__BSD_VISIBLE \

INC_DIR += \
	$(REP_DIR)/src/lib/liblo \
	$(LIBLO_PORT_DIR)/include/lo \
	$(LIBLO_PORT_DIR)/include \

SRC_C := \
	address.c send.c message.c server.c method.c \
	blob.c bundle.c timetag.c pattern_match.c version.c

vpath %.c   $(LIBLO_SRC_DIR)
