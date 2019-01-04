MIRROR_FROM_REP_DIR  := src/app/qt_avplay
MIRROR_FROM_LIBPORTS := src/app/qt5/tmpl/target_defaults.inc \
                        src/app/qt5/tmpl/target_final.inc
MIRROR_FROM_OS       := include/init/child_policy.h

content: $(MIRROR_FROM_REP_DIR) $(MIRROR_FROM_LIBPORTS) $(MIRROR_FROM_OS) LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

$(MIRROR_FROM_LIBPORTS):
	mkdir -p $(dir $@)
	cp $(GENODE_DIR)/repos/libports/$@ $@

$(MIRROR_FROM_OS):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/os/$@ $(dir $@)

LICENSE:
	cp $(GENODE_DIR)/LICENSE $@
