MIRROR_FROM_REP_DIR := src/qt5/webengine/target.inc \
                       src/qt5/webengine/spec/arm_64/target.mk \
                       src/qt5/webengine/spec/x86_64/target.mk

content: $(MIRROR_FROM_REP_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(GENODE_DIR)/repos/libports/ports/qt5)

MIRROR_FROM_PORT_DIR := src/lib/qt5/qtwebengine

content: $(MIRROR_FROM_PORT_DIR)

$(MIRROR_FROM_PORT_DIR):
	mkdir -p $(dir $@)
	cp -r $(PORT_DIR)/$@ $(dir $@)

content: LICENSE

LICENSE:
	cp $(PORT_DIR)/src/lib/qt5/LICENSE.LGPLv3 $@
