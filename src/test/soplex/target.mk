TARGET   = test-soplex
LIBS     = soplex stdcxx posix
SOPLEX   = $(call select_from_ports,soplex)/src/lib/soplex/src/
SRC_CC   = example.cpp

vpath example.cpp $(SOPLEX)
