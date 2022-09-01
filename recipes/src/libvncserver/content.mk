content: src/lib/vnc lib/import lib/mk LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libvnc)

src/lib/vnc:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/lib/vnc/* $@

lib/import:
	mkdir -p $@
	cp $(REP_DIR)/lib/import/import-vncserver.mk $@

lib/mk:
	mkdir -p $@
	cp $(addprefix $(REP_DIR)/$@/,vncserver.mk) $@

LICENSE:
	cp $(PORT_DIR)/src/lib/vnc/README LICENSE
