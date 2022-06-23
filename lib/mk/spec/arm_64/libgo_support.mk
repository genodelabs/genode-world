SPEC_ARCH := arm_64

include $(REP_DIR)/lib/mk/libgo_support.inc

CC_OPT += -I$(BASE_DIR)/src/include
CC_OPT += -I$(BASE_DIR)/src/core/include
