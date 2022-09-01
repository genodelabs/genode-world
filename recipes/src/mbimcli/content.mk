content: src/app/mbimcli src/lib/libmbim LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libmbim)

src/app/mbimcli:
	mkdir -p $@
	cp -r $(REP_DIR)/$@/* $@/.
	cp $(PORT_DIR)/src/lib/libmbim/src/mbimcli/*.c $@/.

src/lib/libmbim:
	mkdir -p $@/src/
	cp -r $(REP_DIR)/$@/* $@/
	cp -r $(PORT_DIR)/src/lib/libmbim/src/mbimcli $@/src/
	cp -r $(PORT_DIR)/src/lib/libmbim/src/common $@/src/

LICENSE:
	cp $(PORT_DIR)/src/lib/libmbim/COPYING $@
