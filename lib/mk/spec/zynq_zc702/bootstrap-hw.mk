TMP := $(call select_from_repositories,lib/mk/spec/zynq/core-hw.inc)
BASE_HW_DIR := $(TMP:%lib/mk/spec/zynq/core-hw.inc=%)

INC_DIR += $(BASE_HW_DIR)/src/bootstrap/spec/zynq

SRC_S   += bootstrap/spec/arm/crt0.s

SRC_CC  += bootstrap/spec/arm/cpu.cc
SRC_CC  += bootstrap/spec/arm/cortex_a9_mmu.cc
SRC_CC  += bootstrap/spec/arm/pic.cc
SRC_CC  += bootstrap/spec/zynq/platform.cc
SRC_CC  += hw/spec/arm/arm_v7_cpu.cc

include $(BASE_HW_DIR)/lib/mk/bootstrap-hw.inc
