#
# \brief  Build config for Genodes core process
# \author Johannes Schlatow
# \date   2016-05-03
#

TMP := $(call select_from_repositories,lib/mk/spec/arm_v7/core-hw-zynq.inc)
BASE_HW_DIR := $(TMP:%lib/mk/spec/arm_v7/core-hw-zynq.inc=%)

# add include paths
INC_DIR += $(REP_DIR)/src/core/board/zynq_zedboard

NR_OF_CPUS = 2

# include less specific configuration
include $(BASE_HW_DIR)/lib/mk/spec/arm_v7/core-hw-zynq.inc

CC_CXX_WARN_STRICT =
