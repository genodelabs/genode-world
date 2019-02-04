content: include/libspng lib/symbols/libspng LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libspng)

include/libspng:
	mkdir -p $@
	cp -r $(PORT_DIR)/$@/* $@

lib/symbols/libspng:
	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/src/lib/libspng/LICENSE $@
