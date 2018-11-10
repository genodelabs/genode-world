INC_DIR += $(REP_DIR)/src/lib/keccak
SRC_C += $(notdir $(wildcard $(KECCAK_SRC_DIR)/*.c))
vpath %.c $(KECCAK_SRC_DIR)
