MIRROR_FROM_REP_DIR  = lib/import/import-glib.mk lib/mk/glib.mk src/lib/glib

content: $(MIRROR_FROM_REP_DIR) src/lib/glib/target.mk LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/glib)

src/lib/glib/target.mk:
	mkdir -p src/lib/glib
	cp -r $(PORT_DIR)/src/lib/glib/* src/lib/glib

	# copy gio includes because glib.mk expects includes at
	#  $(REP_DIR)/include/glib/gio
	mkdir -p include/glib
	cp -r $(PORT_DIR)/include/glib/gio include/glib/
	echo "LIBS = glib" > $@

LICENSE:
	cp $(PORT_DIR)/src/lib/glib/COPYING $@
