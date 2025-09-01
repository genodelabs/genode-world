TARGET   := random-prime

NETTLE_EXAMPLE_DIR := $(call select_from_ports,nettle)/src/lib/nettle/examples
NETTLE_DIR         := $(call select_from_ports,nettle)/src/lib/nettle

LIBS     := posix stdcxx nettle gmp libc base
SRC_CC   := io.c random-prime.c

vpath io.c             $(NETTLE_EXAMPLE_DIR)
vpath random-prime.c   $(NETTLE_EXAMPLE_DIR)

INC_DIR += $(NETTLE_EXAMPLE_DIR) \
           $(NETTLE_DIR) \
           $(REP_DIR)/src/lib/nettle

CC_OPT  = -DHAVE_CONFIG_H

CC_CXX_WARN_STRICT =
