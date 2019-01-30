AUNIT_SRC_DIR := $(call select_from_ports,aunit)/aunit-gpl-2018-src

INC_DIR += $(AUNIT_SRC_DIR)/include/aunit/framework
INC_DIR += $(AUNIT_SRC_DIR)/include/aunit/framework/staticmemory
INC_DIR += $(AUNIT_SRC_DIR)/include/aunit/framework/nocalendar
INC_DIR += $(AUNIT_SRC_DIR)/include/aunit/framework/fullexception
INC_DIR += $(AUNIT_SRC_DIR)/include/aunit/containers
INC_DIR += $(AUNIT_SRC_DIR)/include/aunit/reporters

REP_INC_DIR += include/aunit/framework
REP_INC_DIR += include/aunit/framework/staticmemory
REP_INC_DIR += include/aunit/framework/nocalendar
REP_INC_DIR += include/aunit/framework/fullexception
REP_INC_DIR += include/aunit/containers
REP_INC_DIR += include/aunit/reporters
