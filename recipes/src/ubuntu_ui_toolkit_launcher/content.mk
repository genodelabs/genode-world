content: src/app/ubuntu-ui-toolkit-launcher LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/ubuntu-ui-toolkit)

src/app/ubuntu-ui-toolkit-launcher:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/lib/ubuntu-ui-toolkit/ubuntu-ui-toolkit-launcher/* $@/
	cp -r $(REP_DIR)/src/app/ubuntu-ui-toolkit-launcher/* $@/

LICENSE:
	cp $(PORT_DIR)/src/lib/ubuntu-ui-toolkit/COPYING $@
