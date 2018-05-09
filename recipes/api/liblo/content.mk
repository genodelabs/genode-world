MIRROR_FROM_PORT_AND_REP_DIR = src/lib/liblo
MIRROR_FROM_PORT_DIR = include
MIRROR_FROM_REP_DIR = lib/mk/liblo.mk lib/import/import-liblo.mk

content: \
	$(MIRROR_FROM_PORT_AND_REP_DIR) \
	$(MIRROR_FROM_PORT_DIR) $(MIRROR_FROM_REP_DIR) \
	LICENSE \

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/liblo)

$(MIRROR_FROM_PORT_AND_REP_DIR):
	mkdir -p $(dir $@)
	cp -r $(PORT_DIR)/$@ $(dir $@)
	$(mirror_from_rep_dir)

$(MIRROR_FROM_PORT_DIR):
	mkdir -p $(dir $@)
	cp -r $(PORT_DIR)/$@ $(dir $@)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)


LICENSE:
	cp $(PORT_DIR)/src/lib/liblo/COPYING $@
