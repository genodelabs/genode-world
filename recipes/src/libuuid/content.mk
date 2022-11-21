MIRROR_FROM_REP_DIR = lib/import/import-libuuid.mk lib/mk/libuuid.mk

content: $(MIRROR_FROM_REP_DIR) src/lib/util-linux/libuuid/ src/lib/util-linux/lib/ src/lib/util-linux/include/

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/util-linux)

src/lib/util-linux/libuuid/:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/lib/util-linux/libuuid/* $@
	echo "LIBS := libuuid" > $@/target.mk

src/lib/util-linux/lib/:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/lib/util-linux/lib/* $@

src/lib/util-linux/include/:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/lib/util-linux/include/* $@
