content: include/google lib/import/import-protobuf.mk lib/symbols/protobuf LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/protobuf_grpc)

include/google:
	mkdir -p $@
	cp -r $(PORT_DIR)/$@/* $@

lib/import/import-protobuf.mk:
	mkdir -p $(dir $@)
	cp -a $(REP_DIR)/$@ $@

lib/symbols/protobuf:
	mkdir -p $(dir $@)
	cp -a $(REP_DIR)/$@ $@

LICENSE:
	cp $(PORT_DIR)/src/lib/grpc/third_party/protobuf/LICENSE $@
