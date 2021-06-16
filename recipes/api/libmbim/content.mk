content: include lib/symbols/libmbim LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libmbim)
GENERATED_PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libmbim_generated)

include:
	mkdir -p $@
	cp $(PORT_DIR)/include/libmbim-glib/* $@
	cp $(GENERATED_PORT_DIR)/include/libmbim-glib/* $@
	cp $(GENERATED_PORT_DIR)/src/lib/libmbim_generated/generated/* $@

lib/symbols/libmbim:
	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/src/lib/libmbim/COPYING $@
