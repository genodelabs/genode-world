include $(REP_DIR)/lib/mk/freebl3.inc

CC_MARCH += -march=armv8-a+crypto

SRC_C += gcm-aarch64.c
