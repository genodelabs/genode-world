include $(REP_DIR)/lib/mk/libmpg123.inc

CC_DEF += -DOPT_X86_64

SRC_S += $(notdir $(wildcard $(MPG123_SRC_DIR)/*x86_64*.S))
