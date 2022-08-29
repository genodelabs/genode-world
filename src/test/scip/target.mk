TARGET = test-scip
LIBS   = posix scip soplex stdcxx zlib gmp
SCIP_DIR = $(call select_from_ports,scip)/src/lib/scip/
EX_DIR   = $(SCIP_DIR)/examples/Queens/src
INC_DIR  += $(EX_DIR)
SRC_CC = queens.cpp queens_main.cpp

vpath %.cpp $(EX_DIR)

CC_CXX_WARN_STRICT =
