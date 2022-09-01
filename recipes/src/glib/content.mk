MIRROR_FROM_REP_DIR  = lib/import/import-glib.mk lib/mk/glib.mk

content: $(MIRROR_FROM_REP_DIR) src/lib/glib LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/glib)

src/lib/glib:
	mkdir -p src/lib/glib
	cp -r $(REP_DIR)/src/lib/glib/*  $@
	cp -r $(PORT_DIR)/src/lib/glib/* $@

	# copy gio includes because glib.mk expects includes at
	#  $(REP_DIR)/include/glib/gio
	mkdir -p include/glib
	cp -r $(PORT_DIR)/include/glib/gio include/glib/

LICENSE:
	cp $(PORT_DIR)/src/lib/glib/COPYING $@
