MIRROR_FROM_REP_DIR = \
	lib/mk/gsm10.mk \
	lib/mk/g72x.mk \
	lib/mk/alac.mk \
	lib/mk/libsndfile.mk \
	lib/import/import-libsndfile.mk \
	src/lib/libsndfile/config.h

content: $(MIRROR_FROM_REP_DIR) src/lib/libsndfile/src LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libsndfile)

src/lib/libsndfile/src:
	mkdir -p src/lib/libsndfile
	cp -r $(PORT_DIR)/src/lib/libsndfile/* src/lib/libsndfile

LICENSE:
	cp $(PORT_DIR)/src/lib/libsndfile/COPYING $@
