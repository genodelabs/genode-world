MIRROR_FROM_REP_DIR := lib/import/import-libsndio.mk \
                       lib/symbols/libsndio
 
content: $(MIRROR_FROM_REP_DIR) include LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/sndio)

include:
	cp -r $(PORT_DIR)/$@ $@

LICENSE:
	echo "sndio is subject to the license specified in the source files" > $@
