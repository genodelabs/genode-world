
include $(REP_DIR)/lib/import/import-cmocka.mk

CMOCKA_SRC_DIR  := $(CMOCKA_PORT_DIR)/src/lib/cmocka/src

SHARED_LIB       := yes

LIBS             += base
LIBS             += libc

SRC_C            := cmocka.c

INC_DIR          += $(REP_DIR)/src/lib/cmocka

CC_OPT += -DHAVE_CONFIG_H

vpath %.c $(CMOCKA_SRC_DIR)

CC_CXX_WARN_STRICT_CONVERSION :=

