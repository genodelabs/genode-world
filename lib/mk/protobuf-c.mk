include $(REP_DIR)/lib/import/import-protobuf-c.mk

PROTOBUF_C_SRC_DIR := $(PROTOBUF_C_PORT_DIR)/src/lib/protobuf-c/protobuf-c

SRC_C := protobuf-c.c

LIBS := libc

SHARED_LIB := yes

vpath %.c $(PROTOBUF_C_SRC_DIR)
