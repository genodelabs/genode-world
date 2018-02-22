LDNS_PORT_DIR := $(call select_from_ports,ldns)
INC_DIR += $(LDNS_PORT_DIR)/include
CC_DEF  += -DLDNS_TRUST_ANCHOR_FILE=\"/etc/unbound/root.key\"
