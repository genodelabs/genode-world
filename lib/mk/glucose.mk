GLUCOSE_DIR = $(call select_from_ports,glucose)/src/lib/glucose/
LIBS    += stdcxx zlib libc libm
INC_DIR += $(GLUCOSE_DIR)
SRC_CC   = 	Solver.cc

CC_OPT += -D__STDC_LIMIT_MACROS -D__STDC_FORMAT_MACROS

CC_WARN  =

vpath %.cc $(GLUCOSE_DIR)/core
vpath %.cc $(GLUCOSE_DIR)/utils

#SHARED_LIB = yes
