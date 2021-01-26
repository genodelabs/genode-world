#
# \brief  Build config for Genodes core process
# \author Martin Stein
# \date   2011-12-16
#

TMP := $(call select_from_repositories,lib/mk/core-hw.inc)
BASE_HW_DIR := $(TMP:%lib/mk/core-hw.inc=%)

# add include paths
INC_DIR += $(REP_DIR)/src/core

# add C++ sources
SRC_CC += spec/arm/exynos_mct.cc

# include less specific configuration
include $(BASE_HW_DIR)/lib/mk/spec/cortex_a15/core-hw.inc

vpath spec/arm/exynos_mct.cc ${REP_DIR}/src/core
