include $(REP_DIR)/lib/import/import-ldns.mk
LDNS_SRC_DIR := $(LDNS_PORT_DIR)/src/lib/ldns

LIBS += libc libssl

SRC_LDNS_C := $(notdir $(wildcard $(LDNS_SRC_DIR)/*.c))

SRC_C += $(filter-out linktest.c,$(SRC_LDNS_C))
SRC_C += $(notdir $(wildcard $(LDNS_SRC_DIR)/compat/b64*.c))
SRC_CC +=  getproto.cc

vpath %.c $(LDNS_SRC_DIR) $(LDNS_SRC_DIR)/compat
vpath %.cc $(REP_DIR)/src/lib/ldns
