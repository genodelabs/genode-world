KEYNOTE_PORT_DIR := $(call select_from_ports,keynote)
KEYNOTE_DIR      := $(KEYNOTE_PORT_DIR)/src/lib/keynote

LIBS    += libc libm libcrypto
SHARED_LIB = yes

INC_DIR += $(REP_DIR)/src/lib/keynote

# keynote headres
INC_DIR += $(KEYNOTE_DIR)/include
SRC_C   = 	k.tab.c \
		lex.kn.c \
		environment.c \
		parse_assertion.c \
		signature.c \
		auxil.c \
		base64.c \
		z.tab.c	\
		lex.kv.c \
		keynote-verify.c \
		keynote-sign.c \
		keynote-sigver.c \
		keynote-keygen.c\
		keynote-main.c

CC_OPT = -O2 -w  -DCRYPTO -DHAVE_CONFIG_H
vpath %.c $(KEYNOTE_DIR)
