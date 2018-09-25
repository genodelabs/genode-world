content: include lib/symbols/libconfig LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libconfig)

include:
	mkdir -p $@
	cp -r $(PORT_DIR)/include/libconfig.h $@

lib/symbols/libconfig:
	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/src/lib/libconfig/LICENSE $@
