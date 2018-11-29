content: include lib/symbols/libmad LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libmad)

include:
	mkdir -p $@
	cp $(PORT_DIR)/$@/libmad/mad.h $@

lib/symbols/libmad:
	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/src/lib/libmad/COPYING $@
