MIRROR_FROM_REP_DIR = lib/import/import-jsonc.mk lib/symbols/jsonc

content: $(MIRROR_FROM_REP_DIR) include LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/jsonc)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

include:
	mkdir -p $@
	cp -R $(PORT_DIR)/include/* $@

LICENSE:
	cp $(PORT_DIR)/src/lib/jsonc/COPYING  $@
