content: include/snappy lib/symbols/snappy LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/snappy)

include/snappy:
	mkdir -p $@
	cp -r $(PORT_DIR)/$@/* $@

lib/symbols/snappy:
	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/src/lib/snappy/COPYING $@
