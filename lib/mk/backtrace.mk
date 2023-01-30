MY_BUILD_DIR := $(LIB_CACHE_DIR)/backtrace
MY_TARGET := $(MY_BUILD_DIR)/.libs/libbacktrace.a

# to glue gnu_build.mk
CUSTOM_TARGET_DEPS := finished.tag

$(MY_TARGET): built.tag

finished.tag: $(MY_TARGET)
	@$(MSG_INST)$*
	ln -sf $(MY_TARGET) $(MY_BUILD_DIR)/backtrace.lib.a
	@touch $@

include $(REP_DIR)/lib/mk/libbacktrace.inc
