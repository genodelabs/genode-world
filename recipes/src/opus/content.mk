MIRROR_FROM_REP_DIR = lib/import/import-opus.mk lib/mk/opus.mk

content: $(MIRROR_FROM_REP_DIR) src/lib/opus LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/opus)

src/lib/opus:
	mkdir -p src/lib/opus
	cp -r $(PORT_DIR)/src/lib/opus/* src/lib/opus

LICENSE:
	cp $(PORT_DIR)/src/lib/opus/COPYING $@
