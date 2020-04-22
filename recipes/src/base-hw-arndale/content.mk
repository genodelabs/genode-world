ARCH  := arm_v7
BOARD := arndale

content: lib/mk/spec/arm_v7/core-hw-exynos5.inc

lib/mk/spec/arm_v7/core-hw-exynos5.inc: lib/mk
	cp -r $(REP_DIR)/$@ $@

include $(REP_DIR)/recipes/src/base-hw_content.inc
