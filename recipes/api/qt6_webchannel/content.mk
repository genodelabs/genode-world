PORT_DIR := $(call port_dir,$(GENODE_DIR)/repos/libports/ports/qt6_api)

MIRROR_LIB_SYMBOLS := libQt6WebChannel \
                      libQt6WebChannelQuick

content: $(MIRROR_LIB_SYMBOLS)

$(MIRROR_LIB_SYMBOLS):
	mkdir -p lib/symbols
	cp $(PORT_DIR)/src/lib/qt6_api/lib/symbols/$@ lib/symbols/

content: LICENSE

LICENSE:
	cp $(PORT_DIR)/src/lib/qt6_api/LICENSE.LGPL3 $@
