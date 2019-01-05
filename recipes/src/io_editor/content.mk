MIRROR_FROM_LIBPORTS = src/app/qt5/tmpl

MIRROR_FROM_REP_DIR := src/app/io_editor

content: $(MIRROR_FROM_LIBPORTS) $(MIRROR_FROM_REP_DIR) LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

$(MIRROR_FROM_LIBPORTS):
	mkdir -p $(@)
	cp $(GENODE_DIR)/repos/libports/$@/* $@

LICENSE:
	cp $(GENODE_DIR)/LICENSE $@
