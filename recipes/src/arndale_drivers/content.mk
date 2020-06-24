include $(GENODE_DIR)/repos/base/recipes/src/content.inc

MIRROR_FROM_REP_DIR := include/regulator_session \
                       include/regulator \
                       include/spec/exynos5

content: src/drivers $(MIRROR_FROM_REP_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

src/drivers:
	mkdir -p $@/framebuffer
	cp    -r $(REP_DIR)/src/drivers/framebuffer/spec/exynos5/* $@/framebuffer
	mkdir -p $@/platform
	cp    -r $(REP_DIR)/src/drivers/platform/spec/arndale/* $@/platform
