content: include/grpc include/grpcpp LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/protobuf_grpc)

include/grpc:
	mkdir -p $@
	cp -r $(PORT_DIR)/$@/* $@

include/grpcpp:
	mkdir -p $@
	cp -r $(PORT_DIR)/$@/* $@

LICENSE:
	cp $(PORT_DIR)/src/lib/grpc/LICENSE $@
