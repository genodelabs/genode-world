MIRROR_FROM_REP_DIR := lib/symbols/glib
content: include $(MIRROR_FROM_REP_DIR) LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/glib)

include:
	mkdir -p $@
	cp -r $(PORT_DIR)/include/glib/* $@
	cp -r $(REP_DIR)/include/glib/* $@

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/src/lib/glib/COPYING $@
