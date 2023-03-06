LWEXT4_PORT_DIR := $(call select_from_ports,lwext4)
LWEXT4_DIR      := $(LWEXT4_PORT_DIR)/src/lib/lwext4

SRC_C := $(notdir $(wildcard $(LWEXT4_DIR)/src/*.c))
SRC_C += $(notdir $(wildcard $(LWEXT4_DIR)/blockdev/*.c))

# from musl
SRC_C += qsort.c

SRC_CC := block libc.cc

INC_DIR += $(LWEXT4_PORT_DIR)/include
INC_DIR += $(LWEXT4_DIR)/blockdev
INC_DIR += $(REP_DIR)/src/lib/lwext4/include

CC_OPT += -DCONFIG_USE_DEFAULT_CFG=1
CC_OPT += -DCONFIG_HAVE_OWN_ERRNO=1
CC_OPT += -DCONFIG_HAVE_OWN_ASSERT=1
CC_OPT += -DCONFIG_BLOCK_DEV_CACHE_SIZE=256

LIBS += base format

#SHARED_LIB = yes

vpath     %.c $(LWEXT4_DIR)/src
vpath     %.c $(LWEXT4_DIR)/blockdev
vpath qsort.c $(REP_DIR)/src/lib/lwext4/
vpath    %.cc $(REP_DIR)/src/lib/lwext4/

CC_CXX_WARN_STRICT =
