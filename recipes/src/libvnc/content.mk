content: src/lib/vnc/target.mk lib/import lib/mk LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libvnc)

src/lib/vnc:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/lib/vnc/* $@

lib/import:
	mkdir -p $@
	cp $(REP_DIR)/lib/import/import-vncclient.mk $@

lib/mk:
	mkdir -p $@
	cp $(addprefix $(REP_DIR)/$@/,vncclient.mk) $@

src/lib/vnc/target.mk: src/lib/vnc
	echo "LIBS += vncclient" > $@

LICENSE:
	cp $(PORT_DIR)/src/lib/vnc/README LICENSE
