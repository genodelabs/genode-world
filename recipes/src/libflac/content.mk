MIRROR_FROM_REP_DIR = lib/import/import-libFLAC.mk lib/mk/libFLAC.mk

content: $(MIRROR_FROM_REP_DIR) src/lib/flac/target.mk LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/flac)

src/lib/flac/target.mk:
	mkdir -p src/lib/flac
	cp -r $(PORT_DIR)/src/lib/flac/* src/lib/flac

LICENSE:
	cp $(PORT_DIR)/src/lib/flac/COPYING.Xiph $@
