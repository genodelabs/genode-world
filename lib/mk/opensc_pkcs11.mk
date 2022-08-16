PORT_DIR := $(call select_from_ports,opensc)/src/opensc

SRC_C += pkcs11/pkcs11-global.c
SRC_C += pkcs11/pkcs11-session.c
SRC_C += pkcs11/pkcs11-object.c
SRC_C += pkcs11/misc.c
SRC_C += pkcs11/slot.c
SRC_C += pkcs11/mechanism.c
SRC_C += pkcs11/openssl.c
SRC_C += pkcs11/framework-pkcs15.c
SRC_C += pkcs11/framework-pkcs15init.c
SRC_C += pkcs11/debug.c
SRC_C += pkcs11/pkcs11-display.c
SRC_C += common/libscdl.c
SRC_C += ui/notify.c
SRC_C += $(shell cd $(PORT_DIR)/src; find libopensc/*.c)
SRC_C += sm/sm-eac.c
SRC_C += common/simclist.c
SRC_C += $(shell cd $(PORT_DIR)/src; find scconf/*.c)

SRC_C += dummies.c

CC_OPT += -shared -fPIC -DPIC -lcrypto -ldl -g -DHAVE_CONFIG_H
CC_OPT += -DOPENSC_CONF_PATH=\"$(PORT_DIR)/etc/opensc.conf\"
CC_OPT += -DDEFAULT_SM_MODULE_PATH=\"/\"
CC_OPT += -DDEFAULT_SM_MODULE=\"libsmm_not_available.lib.so\"

LIBS += stdcxx libc libssl pcsc-lite

INC_DIR += $(REP_DIR)/src/lib/opensc_pkcs11
INC_DIR += $(PORT_DIR)/src

vpath dummies.c $(REP_DIR)/src/lib/opensc_pkcs11
vpath % $(PORT_DIR)/src

SHARED_LIB := yes
