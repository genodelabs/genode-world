MIRROR_FROM_REP_DIR = lib/import/import-protobuf-c.mk lib/mk/protobuf-c.mk

content: $(MIRROR_FROM_REP_DIR) src/lib/protobuf-c/target.mk LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/protobuf-c)

src/lib/protobuf-c/target.mk:
	mkdir -p src/lib/protobuf-c/protobuf-c
	cp -r $(PORT_DIR)/src/lib/protobuf-c/protobuf-c/*.c \
		src/lib/protobuf-c/protobuf-c/

LICENSE:
	cp $(PORT_DIR)/src/lib/protobuf-c/LICENSE $@
