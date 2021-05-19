MIRROR_FROM_REP_DIR := lib/symbols/vncserver \
                       lib/import/import-vncserver.mk

content: $(MIRROR_FROM_REP_DIR) include LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libvnc)

include:
	mkdir -p $@/rfb $@/x11vnc
	cp -r $(REP_DIR)/include/libvnc/* $@/.
	cp -r $(PORT_DIR)/src/lib/vnc/rfb/*.h $@/rfb/.
	cp -r $(PORT_DIR)/src/lib/vnc/x11vnc/nox11.h $@/x11vnc/.

LICENSE:
	cp $(PORT_DIR)/src/lib/vnc/README LICENSE
