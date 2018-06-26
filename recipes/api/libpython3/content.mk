MIRROR_FROM_REP_DIR := lib/import/import-python3.mk \
                       lib/symbols/python3

content: include $(MIRROR_FROM_REP_DIR) LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/python3)

include:
	cp -r $(PORT_DIR)/$@ $@
	cp -r $(REP_DIR)/include/python3 $@/

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)


LICENSE:
	cp $(PORT_DIR)/src/lib/python3/LICENSE $@
