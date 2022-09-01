MIRROR_FROM_REP_DIR = lib/import/import-snappy.mk lib/mk/snappy.mk

content: $(MIRROR_FROM_REP_DIR) src/lib/snappy LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/snappy)

src/lib/snappy:
	mkdir -p src/lib/snappy
	cp -r $(PORT_DIR)/src/lib/snappy/* src/lib/snappy

LICENSE:
	cp $(PORT_DIR)/src/lib/snappy/COPYING $@
