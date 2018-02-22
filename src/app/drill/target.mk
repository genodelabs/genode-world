TARGET = drill

LIBS += base libc posix ldns libssl libcrypto

DRILL_SRC_DIR += $(call select_from_ports,ldns)/src/lib/ldns/drill

INC_DIR += $(REP_DIR)/include/ldns

SRC_C += $(notdir $(wildcard $(DRILL_SRC_DIR)/*.c))

vpath %.c $(DRILL_SRC_DIR)
