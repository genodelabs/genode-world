APP_DIR = src/app/grafx2

MIRROR_FROM_PORT_AND_REP_DIR := $(APP_DIR)

content: $(MIRROR_FROM_PORT_AND_REP_DIR) LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/grafx2)

$(MIRROR_FROM_PORT_AND_REP_DIR):
	mkdir -p $(dir $@)
	cp -r $(PORT_DIR)/$@ $(dir $@)
	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/$(APP_DIR)/doc/gpl-2.0.txt $@
