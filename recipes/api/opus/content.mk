content: include lib/symbols/opus LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/opus)

include:
	cp -r $(PORT_DIR)/$@/opus $@

lib/symbols/opus:
	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/src/lib/opus/COPYING $@
