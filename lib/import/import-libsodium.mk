LIBSODIUM_PORT_DIR = $(call select_from_ports,libsodium)
LIBSODIUM_REP_INC = $(call select_from_repositories,include/libsodium)

INC_DIR += $(LIBSODIUM_PORT_DIR)/include/libsodium
INC_DIR += $(LIBSODIUM_PORT_DIR)/include/libsodium/sodium
INC_DIR += $(LIBSODIUM_REP_INC)
INC_DIR += $(LIBSODIUM_REP_INC)/sodium
