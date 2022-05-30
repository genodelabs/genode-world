content: src/app/morph-browser LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/morph-browser)

MIRROR_FROM_PORT_DIR := src/app/morph-browser

src/app/morph-browser:
	mkdir -p $@
	cp -r $(PORT_DIR)/$@/* $@/
	cp -r $(REP_DIR)/$@/* $@/

LICENSE:
	cp $(PORT_DIR)/src/app/morph-browser/COPYING $@
