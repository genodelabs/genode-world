content: include/ogg lib/symbols/libogg LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libogg)

include/ogg:
	mkdir -p $@
	cp -r $(PORT_DIR)/$@/* $@
	cp -r $(REP_DIR)/$@/* $@

lib/symbols/libogg:
	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/src/lib/libogg/COPYING $@
