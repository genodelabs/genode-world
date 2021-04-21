MIRROR_FROM_PORT_AND_REP_DIR := src/app/glmark2

content: $(MIRROR_FROM_PORT_AND_REP_DIR) LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/glmark2)

$(MIRROR_FROM_PORT_AND_REP_DIR):
	mkdir -p $(dir $@)
	cp -r $(PORT_DIR)/$@ $(dir $@)
	$(mirror_from_rep_dir)

#$(MIRROR_FROM_REP_DIR):
#	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/src/app/glmark2/COPYING $@
