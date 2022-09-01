MIRROR_FROM_REP_DIR = \
	lib/import/import-libmpg123.mk \
	lib/mk/libmpg123.inc \
	lib/mk/spec/x86_64/libmpg123.mk \
	lib/mk/spec/arm/libmpg123.mk \

content: $(MIRROR_FROM_REP_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/mpg123)

content: src/lib/mpg123 LICENSE

src/lib/mpg123:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/lib/mpg123/*       $@
	cp    $(REP_DIR)/src/lib/mpg123/config.h $@

LICENSE:
	cp $(PORT_DIR)/src/lib/mpg123/COPYING $@
