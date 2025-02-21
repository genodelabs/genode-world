MIRROR_FROM_REP_DIR := src/qt6/webengine/target.inc \
                       src/qt6/webengine/spec/arm_64/target.mk \
                       src/qt6/webengine/spec/x86_64/target.mk

content: $(MIRROR_FROM_REP_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/qt6_webengine)

MIRROR_FROM_PORT_DIR := src/lib/qt6_webengine

content: $(MIRROR_FROM_PORT_DIR)

$(MIRROR_FROM_PORT_DIR):
	mkdir -p $(dir $@)
	cp -r $(PORT_DIR)/$@ $(dir $@)

content: LICENSE

LICENSE:
	cp $(PORT_DIR)/src/lib/qt6_webengine/LICENSES/LGPL-3.0-only.txt $@
