MBEDTLS_PORT_DIR := $(call select_from_ports,mbedtls)
MBEDTLS_DIR      := $(MBEDTLS_PORT_DIR)/src/lib/mbedtls/

LIBS += libc

INC_DIR += $(MBEDTLS_PORT_DIR)/include/mbedtls
INC_DIR += $(MBEDTLS_PORT_DIR)/include/psa
INC_DIR += $(MBEDTLS_PORT_DIR)/src/lib/mbedtls/library/

CC_OLEVEL = -O2

SRC_C += $(notdir $(wildcard $(MBEDTLS_DIR)/library/*.c))

vpath %.c $(MBEDTLS_DIR)/library

SHARED_LIB = yes

CC_OPT += -Wno-missing-braces
CC_CXX_WARN_STRICT =
