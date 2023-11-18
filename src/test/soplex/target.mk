TARGET   = test-soplex
LIBS     = soplex stdcxx posix zlib gmp
SOPLEX   = $(call select_from_ports,soplex)/src/lib/soplex/src/
SRC_CC   = example.cpp

vpath example.cpp $(SOPLEX)

CC_CXX_WARN_STRICT =
CC_CXX_OPT_STD = -std=gnu++17
