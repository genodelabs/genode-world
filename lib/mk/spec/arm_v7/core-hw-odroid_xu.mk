#
# \brief  Build config for Genodes core process
# \author Stefan Kalkowski
# \date   2015-02-09
#

# add include paths
INC_DIR += $(REP_DIR)/src/core/board/odroid_xu
INC_DIR += $(REP_DIR)/src/include

# add C++ sources
SRC_CC += spec/arm/gicv2.cc
SRC_CC += kernel/vm_thread_off.cc
SRC_CC += platform_services.cc

# include less specific configuration
include $(REP_DIR)/lib/mk/spec/arm_v7/core-hw-exynos5.inc
