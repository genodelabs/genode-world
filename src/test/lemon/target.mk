TARGET   = test-lemon
LIBS     = lemon stdcxx
LEMON    = $(call select_from_ports,lemon)/src/lib/lemon/test
SRC_CC   = graph_test.cc

vpath graph_test.cc $(LEMON)
