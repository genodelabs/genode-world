MIRROR_FROM_REP_DIR := lib/import/import-ubuntu-ui-toolkit.mk
 
content: $(MIRROR_FROM_REP_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/ubuntu-ui-toolkit)

MIRROR_FROM_PORT_DIR := src/lib/ubuntu-ui-toolkit/genode/api/*

content: $(MIRROR_FROM_PORT_DIR)

$(MIRROR_FROM_PORT_DIR):
	cp -r $(PORT_DIR)/$@ .

content: LICENSE

LICENSE:
	cp $(PORT_DIR)/src/lib/ubuntu-ui-toolkit/COPYING $@
