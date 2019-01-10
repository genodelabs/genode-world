include $(REP_DIR)/lib/import/import-libsodium.mk

LIBSODIUM_PORT_DIR = $(call select_from_ports,libsodium)
LIBSODIUM_SRC_DIR = $(call select_from_ports,libsodium)/src/lib/libsodium/src/libsodium

LIBS += jitterentropy libc

CC_DEF += -DSODIUM_VERSION_STRING=\"\"
CC_DEF += -DSODIUM_LIBRARY_VERSION_MAJOR=0
CC_DEF += -DSODIUM_LIBRARY_VERSION_MINOR=0
CC_DEF += -DHAVE_ATOMIC_OPS

EXPERT_CRYPTO_MAGIC := $(wildcard \
	$(LIBSODIUM_SRC_DIR)/*/*.c \
	$(LIBSODIUM_SRC_DIR)/*/*/*.c \
	$(LIBSODIUM_SRC_DIR)/*/*/*/*.c)
	
TOO_MUCH_MAGIC = core.c randombytes_sysrandom.c argon

SRC_C += $(filter-out $(TOO_MUCH_MAGIC),$(notdir $(EXPERT_CRYPTO_MAGIC)))
SRC_C += onetimeauth_poly1305.c
vpath %.c $(sort $(dir $(EXPERT_CRYPTO_MAGIC)))

SRC_C += randombytes_salsa20_jitterentropy.c
vpath randombytes_salsa20_jitterentropy.c $(REP_DIR)/src/lib/libsodium

SRC_CC += genode_core.cc
vpath %.cc $(REP_DIR)/src/lib/libsodium

SHARED_LIB = 1
