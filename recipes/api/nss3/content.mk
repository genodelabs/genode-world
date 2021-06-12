MIRROR_FROM_REP_DIR := lib/import/import-nss3.mk \
                       lib/symbols/nss3
 
content: $(MIRROR_FROM_REP_DIR) include LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/nss3)

include:
	cp -r $(PORT_DIR)/$@ $@
	cp -r $(REP_DIR)/include/nss $@/

LICENSE:
	cp $(PORT_DIR)/src/lib/nss/COPYING $@
