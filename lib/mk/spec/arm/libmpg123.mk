include $(REP_DIR)/lib/mk/libmpg123.inc

CC_DEF += -DOPT_ARM

SRC_S += $(notdir $(wildcard $(MPG123_SRC_DIR)/*arm*.S))
