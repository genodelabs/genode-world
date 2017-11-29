content: src/app/chocolate-doom LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/chocolate-doom)

src/app/chocolate-doom:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/app/chocolate-doom/* $@
	cp -r $(REP_DIR)/src/app/chocolate-doom/* $@

LICENSE:
	cp $(PORT_DIR)/src/app/chocolate-doom/COPYING $@
