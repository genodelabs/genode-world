include $(REP_DIR)/lib/import/import-c-toxcore.mk

TOXCORE_SRC_DIR = $(TOXCORE_PORT_DIR)/src/lib/c-toxcore

CC_C_OPT += -std=c99 -D_XOPEN_SOURCE

INC_DIR += $(TOXCORE_SRC_DIR)/toxcore

TOXCORE_SRC_C := $(notdir $(wildcard $(TOXCORE_SRC_DIR)/toxcore/*.c))

SRC_C  += $(filter-out $(TOXCORE_SRC_FILTER),$(TOXCORE_SRC_C))

LIBS := libsodium libc

vpath %.c  $(TOXCORE_SRC_DIR)/toxcore

SHARED_LIB = yes
