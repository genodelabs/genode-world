include $(REP_DIR)/lib/import/import-snappy.mk

SNAPPY_SRC_DIR = $(SNAPPY_PORT_DIR)/src/lib/snappy

INC_DIR += $(SNAPPY_SRC_DIR)

SRC_CC += snappy-c.cc snappy-sinksource.cc snappy-stubs-internal.cc snappy.cc

vpath %.cc $(SNAPPY_SRC_DIR)

LIBS += stdcxx

SHARED_LIB = yes

CC_WARN += -Wno-sign-compare
CC_CXX_WARN_STRICT =
