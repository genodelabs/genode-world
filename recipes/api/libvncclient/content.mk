MIRROR_FROM_REP_DIR := lib/symbols/vncclient \
                       lib/import/import-vncclient.mk

content: $(MIRROR_FROM_REP_DIR) include LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libvnc)

include:
	mkdir -p $@/rfb
	cp -r $(REP_DIR)/include/libvnc/* $@/.
	cp -r $(PORT_DIR)/src/lib/vnc/rfb/*.h $@/rfb/.

LICENSE:
	cp $(PORT_DIR)/src/lib/vnc/COPYING LICENSE
