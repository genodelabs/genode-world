MIRROR_FROM_REP_DIR = lib/import/import-libuuid.mk lib/symbols/libuuid

content: $(MIRROR_FROM_REP_DIR) include LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/util-linux)

include:
	mkdir -p $@/uuid
	cp -r $(PORT_DIR)/include/libuuid/* $@
	cp -r $(PORT_DIR)/include/libuuid/* $@/uuid

LICENSE:
	cp $(PORT_DIR)/src/lib/util-linux/COPYING $@
