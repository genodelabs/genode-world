content: include/grpc include/grpcpp lib/import/import-grpc.mk lib/symbols/grpc LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/protobuf_grpc)

include/grpc:
	mkdir -p $@
	cp -r $(PORT_DIR)/$@/* $@

include/grpcpp:
	mkdir -p $@
	cp -r $(PORT_DIR)/$@/* $@

lib/import/import-grpc.mk:
	mkdir -p $(dir $@)
	cp -a $(REP_DIR)/$@ $@

lib/symbols/grpc:
	mkdir -p $(dir $@)
	cp -a $(REP_DIR)/$@ $@

LICENSE:
	cp $(PORT_DIR)/src/lib/grpc/LICENSE $@
