TARGET   := sha1-test

NETTLE_TESTSUITE_DIR := $(call select_from_ports,nettle)/src/lib/nettle/testsuite
NETTLE_DIR := $(call select_from_ports,nettle)/src/lib/nettle

LIBS  := posix stdcxx nettle gmp libc base
SRC_C := sha1-test.c testutils.c

vpath sha1-test.c     $(NETTLE_TESTSUITE_DIR)
vpath testutils.c     $(NETTLE_TESTSUITE_DIR)

INC_DIR += $(NETTLE_TESTSUITE_DIR) \
           $(NETTLE_DIR) \
           $(REP_DIR)/src/lib/nettle

CC_OPT  = -DHAVE_CONFIG_H -v

CC_CXX_WARN_STRICT =
