MIRROR_FROM_REP_DIR = lib/import/import-protobuf.mk lib/mk/protobuf.mk

content: $(MIRROR_FROM_REP_DIR) src/lib/protobuf/target.mk LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/protobuf_grpc)

src/lib/protobuf/target.mk:
	mkdir -p src/lib/protobuf/src
	cp -r $(PORT_DIR)/src/lib/protobuf/src/* \
		src/lib/protobuf/src/
	echo "LIBS := protobuf" > $@

LICENSE:
	cp $(PORT_DIR)/src/lib/protobuf/LICENSE $@
