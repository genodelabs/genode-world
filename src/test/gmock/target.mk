TARGET = gmock

GMOCK_DIR := $(call select_from_ports,googletest)/src/lib/googletest/googlemock
GTEST_DIR := $(call select_from_ports,googletest)/src/lib/googletest/googletest

SRC_CC = gmock_all_test.cc

vpath gmock_all_test.cc $(GMOCK_DIR)/test

INC_DIR += $(GMOCK_DIR) $(GTEST_DIR)
CC_OPT  += -DGTEST_HAS_PTHREAD=0

LIBS = posix stdcxx gtest gmock
