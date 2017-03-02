TMP := $(call select_from_repositories,lib/mk/spec/zynq/bootstrap-hw.inc)
BASE_HW_DIR := $(TMP:%lib/mk/spec/zynq/bootstrap-hw.inc=%)

INC_DIR += $(REP_DIR)/src/core/include/spec/xilinx_uartps_1
INC_DIR += $(REP_DIR)/src/core/include/spec/zynq_zedboard

include $(BASE_HW_DIR)/lib/mk/spec/zynq/bootstrap-hw.inc
