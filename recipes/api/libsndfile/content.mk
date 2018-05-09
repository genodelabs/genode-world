content: include lib/symbols/libsndfile LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libsndfile)

include:
	mkdir -p $@
	cp -r $(PORT_DIR)/include/libsndfile/* $@

lib/symbols/libsndfile:
	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/src/lib/libsndfile/COPYING $@
