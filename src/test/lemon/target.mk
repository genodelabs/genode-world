TARGET   = test-lemon
LIBS     = lemon stdcxx posix
LEMON    = $(call select_from_ports,lemon)/src/lib/lemon/test
SRC_CC   = graph_test.cc

vpath graph_test.cc $(LEMON)

CC_CXX_OPT_STD = -std=gnu++17
CC_CXX_WARN_STRICT =
