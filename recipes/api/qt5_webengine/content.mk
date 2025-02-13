PORT_DIR := $(call port_dir,$(GENODE_DIR)/repos/libports/ports/qt5)

MIRROR_SYMBOLS := src/lib/qt5/genode/api/lib/symbols/libQt5WebEngine \
                  src/lib/qt5/genode/api/lib/symbols/libQt5WebEngineCore \
                  src/lib/qt5/genode/api/lib/symbols/libQt5WebEngineWidgets \

content: $(MIRROR_SYMBOLS)

$(MIRROR_SYMBOLS):
	mkdir -p lib/symbols
	cp $(PORT_DIR)/$@ lib/symbols

content: LICENSE

LICENSE:
	cp $(PORT_DIR)/src/lib/qt5/LICENSE.LGPLv3 $@
