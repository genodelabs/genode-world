MIRROR_FROM_REP_DIR  := src/app/qt_avplay
MIRROR_FROM_OS       := include/init/child_policy.h

content: $(MIRROR_FROM_REP_DIR) $(MIRROR_FROM_OS) LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

$(MIRROR_FROM_OS):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/os/$@ $(dir $@)

LICENSE:
	cp $(GENODE_DIR)/LICENSE $@
