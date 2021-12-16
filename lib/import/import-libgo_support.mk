INC_DIR += $(REP_DIR)/src/lib/libgo_support

LD_OPT += --defsym=__data_start=_parent_cap
