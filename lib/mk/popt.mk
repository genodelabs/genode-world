POPT_DIR := $(call select_from_ports,popt)/src/lib/popt

LIBS += libc

SRC_C := lookup3.c
SRC_C += popt.c
SRC_C += poptconfig.c
SRC_C += popthelp.c
SRC_C += poptint.c
SRC_C += poptparse.c
SRC_C += tdict.c

CC_C_OPT += -Wno-implicit-function-declaration -Wno-unused-but-set-variable -Wno-unused-variable

CC_CXX_WARN_STRICT =

INC_DIR += $(POPT_DIR)

vpath %.c $(POPT_DIR)

SHARED_LIB = 1
