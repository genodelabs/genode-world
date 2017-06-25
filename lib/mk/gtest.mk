GTEST_DIR := $(call select_from_ports,googletest)/src/lib/googletest/googletest

include $(REP_DIR)/lib/import/import-gtest.mk

SRC_CC = gtest-all.cc

vpath %.cc $(GTEST_DIR)/src

INC_DIR += $(GTEST_DIR)
INC_DIR += $(GTEST_DIR)/include
INC_DIR += $(GTEST_DIR)/include/internal

LIBS += libc libm stdcxx
