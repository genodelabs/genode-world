include $(call select_from_repositories,lib/mk/nss3_common.inc)

SOFTOKEN_DIR := $(NSS_PORT_DIR)/nss/lib/softoken
LIBS         += libc

INC_DIR += $(NSS_PORT_DIR)/nss/lib/sqlite

# freebl
SRC_C += loader.c

# sqlite
SRC_C += sqlite3.c

# softoken
SRC_C += fipsaudt.c \
         fipstest.c \
         fipstokn.c \
         kbkdf.c \
         lowkey.c \
         lowpbe.c \
         padbuf.c \
         pkcs11.c \
         pkcs11c.c \
         pkcs11u.c \
         sdb.c \
         sftkdb.c \
         sftkdhverify.c \
         sftkhmac.c \
         sftkike.c \
         sftkmessage.c \
         sftkpars.c \
         sftkpwd.c \
         softkver.c \
         tlsprf.c \
         jpakesftk.c \
         lgglue.c

CC_OPT += -DSOFTOKEN_LIB_NAME=\"softokn3.lib.so\"

vpath %.c $(SOFTOKEN_DIR)
vpath loader.c $(NSS_PORT_DIR)/nss/lib/freebl
vpath sqlite3.c $(NSS_PORT_DIR)/nss/lib/sqlite

SHARED_LIB = yes

CC_CXX_WARN_STRICT =
