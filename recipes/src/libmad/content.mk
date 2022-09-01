MIRROR_FROM_REP_DIR = lib/import/import-libmad.mk lib/mk/libmad.mk

content: $(MIRROR_FROM_REP_DIR) src/lib/libmad LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libmad)

src/lib/libmad:
	mkdir -p src/lib/libmad
	cp -r $(PORT_DIR)/src/lib/libmad/* src/lib/libmad

LICENSE:
	cp $(PORT_DIR)/src/lib/libmad/COPYING $@
