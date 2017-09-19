content: include lib/symbols/fdk-aac LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/fdk-aac)

include:
	mkdir $@
	cp -r $(PORT_DIR)/include/fdk-aac/* $@/

lib/symbols/fdk-aac:
	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/src/lib/fdk-aac/NOTICE $@

