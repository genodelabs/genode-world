include $(call select_from_repositories,lib/mk/nss3.inc)

CKFW_DIR := $(NSS_PORT_DIR)/nss/lib/ckfw
LIBS     += libc

INC_DIR += $(CKFW_DIR)

SRC_C += anchor.c \
         constants.c \
         bfind.c \
         binst.c\
         bobject.c \
         bsession.c \
         bslot.c \
         btoken.c \
         certdata.c \
         ckbiver.c

vpath %.c $(CKFW_DIR)/builtins

SHARED_LIB = yes

CC_CXX_WARN_STRICT =
