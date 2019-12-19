content: src/app/avconv lib/import LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libav)

lib/import:
	mkdir -p $@
	cp $(REP_DIR)/$@/import-av.inc $@

src/app/avconv:
	mkdir -p $@
	cp $(PORT_DIR)/src/lib/libav/avconv.c \
	   $(PORT_DIR)/src/lib/libav/avconv_opt.c \
	   $(PORT_DIR)/src/lib/libav/avconv_filter.c \
	   $(PORT_DIR)/src/lib/libav/cmdutils.* $@
	cp $(REP_DIR)/src/app/avconv/* $@
	cp $(REP_DIR)/src/lib/libav/config.h $@
	rm $@/avconv.patch

LICENSE:
	cp $(PORT_DIR)/src/lib/libav/LICENSE $@
