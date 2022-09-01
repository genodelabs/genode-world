MIRROR_FROM_REP_DIR := lib/mk/libQt5WebChannel.mk

content: $(MIRROR_FROM_REP_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(GENODE_DIR)/repos/libports/ports/qt5)

MIRROR_FROM_PORT_DIR := src/lib/qt5/qtwebchannel

content: $(MIRROR_FROM_PORT_DIR)

$(MIRROR_FROM_PORT_DIR):
	mkdir -p $(dir $@)
	cp -r $(PORT_DIR)/$@ $(dir $@)

content: LICENSE

LICENSE:
	cp $(PORT_DIR)/src/lib/qt5/LICENSE.LGPLv3 $@
