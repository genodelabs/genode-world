MIRROR_FROM_REP_DIR := src/qt6/webchannel/target.mk

content: $(MIRROR_FROM_REP_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/qt6_webchannel)

MIRROR_FROM_PORT_DIR := src/lib/qt6_webchannel

content: $(MIRROR_FROM_PORT_DIR)

$(MIRROR_FROM_PORT_DIR):
	mkdir -p $(dir $@)
	cp -r $(PORT_DIR)/$@ $(dir $@)

content: LICENSE

LICENSE:
	cp $(PORT_DIR)/src/lib/qt6_webchannel/LICENSES/LGPL-3.0-only.txt $@
