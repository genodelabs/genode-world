CRYPTOPP_SRC_DIR = $(call select_from_ports,cryptopp)/src/lib/cryptopp

LIBS += stdcxx

CRYPTOPP_SRC_CC := $(notdir $(wildcard $(CRYPTOPP_SRC_DIR)/*.cpp))

CRYPTOPP_FILTER = bench.cpp bench2.cpp test.cpp validat1.cpp

SRC_CC := $(filter-out $(CRYPTOPP_FILTER),$(CRYPTOPP_SRC_CC))

CC_WARN += -Wno-delete-non-virtual-dtor

vpath %.cpp $(CRYPTOPP_SRC_DIR)
