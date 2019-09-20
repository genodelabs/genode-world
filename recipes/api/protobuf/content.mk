content: include/google LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/protobuf_grpc)

include/google:
	mkdir -p $@
	cp -r $(PORT_DIR)/$@/* $@

LICENSE:
	cp $(PORT_DIR)/src/lib/protobuf/LICENSE $@
