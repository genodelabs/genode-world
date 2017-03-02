#
# \brief  Build config for Genodes core process
# \author Johannes Schlatow
# \date   2016-05-03
#

TMP := $(call select_from_repositories,lib/mk/spec/zynq/core-hw.inc)
BASE_HW_DIR := $(TMP:%lib/mk/spec/zynq/core-hw.inc=%)

# add include paths
INC_DIR += $(REP_DIR)/src/core/include/spec/xilinx_uartps_1
INC_DIR += $(REP_DIR)/src/core/include/spec/zynq_parallella

# include less specific configuration
include $(BASE_HW_DIR)/lib/mk/spec/zynq/core-hw.inc
