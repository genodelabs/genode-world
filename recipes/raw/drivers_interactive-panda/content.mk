content: drivers.config input_filter.config en_us.chargen special.chargen

drivers.config input_filter.config:
	cp $(REP_DIR)/recipes/raw/drivers_interactive-panda/$@ $@

en_us.chargen special.chargen:
	cp $(GENODE_DIR)/repos/os/src/server/input_filter/$@ $@
