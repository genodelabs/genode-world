TARGET = gtest-samples

GTEST_DIR := $(call select_from_ports,googletest)/src/lib/googletest/googletest

SRC_CC = main.cc gtest_main.cc sample1.cc sample1_unittest.cc

vpath gtest_main.cc       $(GTEST_DIR)/src
vpath sample1.cc          $(GTEST_DIR)/samples
vpath sample1_unittest.cc $(GTEST_DIR)/samples

INC_DIR += $(GTEST_DIR)/samples

LIBS = posix stdcxx gtest
