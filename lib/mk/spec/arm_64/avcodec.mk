CC_C_OPT += -DARCH_AARCH64=1

include $(REP_DIR)/lib/mk/avcodec.inc

-include $(LIBAVCODEC_DIR)/aarch64/Makefile
