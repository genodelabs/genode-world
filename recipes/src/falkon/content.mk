content: src/app/falkon LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/falkon)

MIRROR_FROM_PORT_DIR := src/app/falkon

src/app/falkon:
	mkdir -p $@
	cp -r $(PORT_DIR)/$@/* $@/
	cp -r $(REP_DIR)/$@/* $@/

LICENSE:
	cp $(PORT_DIR)/src/app/falkon/COPYING $@
