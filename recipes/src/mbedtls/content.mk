MIRROR_FROM_PORT_DIR := src/lib/mbedtls include

MIRROR_FROM_REP_DIR := lib/mk/mbedtls.mk

content: $(MIRROR_FROM_PORT_DIR) $(MIRROR_FROM_REP_DIR) LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/mbedtls)

$(MIRROR_FROM_PORT_DIR):
	mkdir -p $(dir $@)
	cp -r $(PORT_DIR)/$@ $(dir $@)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/src/lib/mbedtls/LICENSE $@
