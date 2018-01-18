content: include lib/symbols/libflif LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/flif)

include:
	mkdir $@
	cp -r $(PORT_DIR)/include/flif/* $@/

lib/symbols/libflif:
	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/src/lib/flif/LICENSE $@
