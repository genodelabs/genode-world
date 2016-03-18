LEMON_DIR = $(call select_from_ports,lemon)/src/lib/lemon/lemon/
LIBS    += stdcxx libm
INC_DIR += $(call select_from_ports,lemon)/include/
INC_DIR += $(REP_DIR)/src/lib/lemon/
SRC_CC   = 	base.cc

CC_WARN  =

vpath %.cc $(LEMON_DIR)

#SHARED_LIB = yes
