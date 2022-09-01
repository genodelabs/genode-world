content: \
	src/lib/fdk-aac \
	lib/import/import-fdk-aac.mk \
	lib/mk/fdk-aac.inc \
	lib/mk/fdk-aac.mk \
	lib/mk/fdk-aac_sbrdec.mk \
	lib/mk/fdk-aac_sbrenc.mk \
	LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/fdk-aac)

include:
	mkdir $@
	cp -r $(PORT_DIR)/include/fdk-aac/* $@/

src/lib/fdk-aac:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/lib/fdk-aac/* $@

lib/mk/%:
	$(mirror_from_rep_dir)
lib/import/%:
	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/src/lib/fdk-aac/NOTICE $@
