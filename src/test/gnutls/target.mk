TARGET   := print-ciphersuites

GNUTLS_EXAMPLE_DIR := $(call select_from_ports,gnutls)/src/lib/gnutls/doc/examples
GNUTLS_DIR := $(call select_from_ports,gnutls)/src/lib/gnutls

LIBS     := posix stdcxx gnutls nettle gmp libc base
SRC_CC   := print-ciphersuites.c
#SRC_CC   := io.c random-prime.c

vpath print-ciphersuites.c     $(GNUTLS_EXAMPLE_DIR)
#vpath nettle-internal.c    $(GNUTLS_DIR)

INC_DIR += $(GNUTLS_EXAMPLE_DIR)
INC_DIR += $(GNUTLS_DIR)
INC_DIR += $(REP_DIR)/src/lib/gnutls

CC_OPT  = -DHAVE_CONFIG_H -v

CC_CXX_WARN_STRICT =

