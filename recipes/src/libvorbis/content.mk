MIRROR_FROM_REP_DIR = lib/import/import-libvorbis.mk lib/mk/libvorbis.mk

content: $(MIRROR_FROM_REP_DIR) src/lib/libvorbis LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libvorbis)

src/lib/libvorbis:
	mkdir -p src/lib/libvorbis
	cp -r $(PORT_DIR)/src/lib/libvorbis/* src/lib/libvorbis

LICENSE:
	cp $(PORT_DIR)/src/lib/libvorbis/COPYING $@
