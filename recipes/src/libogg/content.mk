MIRROR_FROM_REP_DIR = lib/import/import-libogg.mk lib/mk/libogg.mk

content: $(MIRROR_FROM_REP_DIR) src/lib/libogg LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libogg)

src/lib/libogg:
	mkdir -p src/lib/libogg
	cp -r $(PORT_DIR)/src/lib/libogg/* src/lib/libogg

LICENSE:
	cp $(PORT_DIR)/src/lib/libogg/COPYING $@
