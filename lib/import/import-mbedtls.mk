MBEDTLS_PORT_DIR := $(call select_from_ports,mbedtls)
LIBS += libc
INC_DIR += $(MBEDTLS_PORT_DIR)/include/mbedtls/
INC_DIR += $(MBEDTLS_PORT_DIR)/include/psa/

