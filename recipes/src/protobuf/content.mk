MIRROR_FROM_REP_DIR = lib/import/import-protobuf.mk \
                      lib/mk/protobuf.mk

content: $(MIRROR_FROM_REP_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

content:  port_files LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/protobuf_grpc)

port_files:
	mkdir -p src/lib/grpc/third_party/protobuf/src
	cp -r $(PORT_DIR)/src/lib/grpc/third_party/protobuf/src/* \
		src/lib/grpc/third_party/protobuf/src/

LICENSE:
	cp $(PORT_DIR)/src/lib/grpc/third_party/protobuf/LICENSE $@
