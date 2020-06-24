include $(GENODE_DIR)/repos/base/recipes/src/content.inc

MIRROR_FROM_OS_REP_DIR := include/gpio

content: src/drivers $(MIRROR_FROM_OS_REP_DIR)

$(MIRROR_FROM_OS_REP_DIR):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/os/$@ $@

src/drivers:
	mkdir -p $@/framebuffer
	cp    -r $(REP_DIR)/src/drivers/framebuffer/spec/omap4/* $@/framebuffer
	mkdir -p $@/gpio
	cp    -r $(REP_DIR)/src/drivers/gpio/spec/omap4/* $@/gpio
