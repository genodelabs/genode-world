#
# \brief  Build description for Genode's core component
# \author Stefan Kalkowski
# \author Martin Stein
# \date   2012-10-04
#

# add include paths
INC_DIR += $(REP_DIR)/src/core/board/panda
INC_DIR += $(REP_DIR)/src/include

# add C++ sources
SRC_CC += platform_services.cc

NR_OF_CPUS += 2

CC_MARCH = -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=softfp

# include less specific configuration
include $(BASE_DIR)/../base-hw/lib/mk/spec/cortex_a9/core-hw.inc
