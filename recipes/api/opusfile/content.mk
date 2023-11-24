PORT_DIR := $(call port_dir,$(REP_DIR)/ports/opusfile)

MIRROR_FROM_REP_DIR := lib/mk/opusfile.mk \
                       lib/import/import-opusfile.mk

include:
	cp -r $(PORT_DIR)/$@/opusfile $@

src:
	cp -r $(PORT_DIR)/src $@

content: $(MIRROR_FROM_REP_DIR) include src LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/src/lib/opusfile/COPYING $@

