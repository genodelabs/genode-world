# must be defined before the inclusion of the libavutil 'Makefile'
ARCH_ARM=yes

CC_C_OPT += -DARCH_AARCH64=1

include $(REP_DIR)/lib/mk/avutil.inc

-include $(LIBAVUTIL_DIR)/aarch64/Makefile
