MIRROR_FROM_REP_DIR := lib/mk/ubuntu-ui-toolkit.mk

content: $(MIRROR_FROM_REP_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/ubuntu-ui-toolkit)

MIRROR_FROM_PORT_DIR := src/lib/ubuntu-ui-toolkit

content: $(MIRROR_FROM_PORT_DIR)

$(MIRROR_FROM_PORT_DIR):
	mkdir -p $(dir $@)
	cp -r $(PORT_DIR)/$@ $(dir $@)

content: LICENSE

LICENSE:
	cp $(PORT_DIR)/src/lib/ubuntu-ui-toolkit/COPYING $@
