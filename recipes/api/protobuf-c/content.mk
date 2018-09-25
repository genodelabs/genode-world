content: include/protobuf-c lib/symbols/protobuf-c LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/protobuf-c)

include/protobuf-c:
	mkdir -p $@
	cp -r $(PORT_DIR)/$@/* $@

lib/symbols/protobuf-c:
	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/src/lib/protobuf-c/LICENSE $@
