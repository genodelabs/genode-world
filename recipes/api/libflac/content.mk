content: include lib/symbols/libFLAC LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/flac)

include:
	cp -r $(PORT_DIR)/$@ $@

lib/symbols/libFLAC:
	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/src/lib/flac/COPYING.Xiph $@
