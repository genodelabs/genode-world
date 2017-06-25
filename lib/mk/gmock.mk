GMOCK_DIR := $(call select_from_ports,googletest)/src/lib/googletest/googlemock
GTEST_DIR := $(call select_from_ports,googletest)/src/lib/googletest/googletest

include $(REP_DIR)/lib/import/import-gtest.mk

SHARED_LIB = yes

SRC_CC = gmock-all.cc

vpath %.cc $(GMOCK_DIR)/src

INC_DIR += $(GMOCK_DIR)
INC_DIR += $(GMOCK_DIR)/include
INC_DIR += $(GMOCK_DIR)/include/internal

INC_DIR += $(GTEST_DIR)/include
INC_DIR += $(GTEST_DIR)/include/internal

LIBS += stdcxx
