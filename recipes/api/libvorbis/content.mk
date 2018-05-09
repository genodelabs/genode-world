content: include lib/symbols/libvorbis LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libvorbis)

include:
	cp -r $(PORT_DIR)/$@ $@

lib/symbols/libvorbis:
	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/src/lib/libvorbis/COPYING $@
