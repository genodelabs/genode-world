content: src/lib/flif lib/mk/libflif.mk LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/flif)

src/lib/flif:
	$(mirror_from_rep_dir)
	cp -r $(PORT_DIR)/src/lib/flif/* $@

lib/mk/libflif.mk:
	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/src/lib/flif/LICENSE $@

