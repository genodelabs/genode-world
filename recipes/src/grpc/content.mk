MIRROR_FROM_REP_DIR = lib/import/import-grpc.mk \
                      lib/import/import-protobuf.mk \
                      lib/mk/grpc.mk

content: $(MIRROR_FROM_REP_DIR) src/lib/grpc LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/protobuf_grpc)

src/lib/grpc:
	mkdir -p src/lib/grpc/src
	cp -r $(PORT_DIR)/src/lib/grpc/src/* \
		src/lib/grpc/src/
	mkdir -p src/lib/grpc/third_party
	cp -r $(PORT_DIR)/src/lib/grpc/third_party/* \
		src/lib/grpc/third_party/
	mkdir -p proto/
	cp -r $(PORT_DIR)/proto/* \
		proto/
	mkdir -p src/lib/grpc/include
	cp -r $(PORT_DIR)/src/lib/grpc/include/* \
		src/lib/grpc/include/

LICENSE:
	cp $(PORT_DIR)/src/lib/grpc/LICENSE $@
