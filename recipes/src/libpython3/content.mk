MIRROR_FROM_REP_DIR = lib/mk/python3.inc lib/mk/spec/x86_64/python3.mk

content: include $(MIRROR_FROM_REP_DIR) src/lib/python3 LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/python3)

include:
	cp -r $(REP_DIR)/include/python3 $@/

src/lib/python3:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/lib/python3/* $@
	cp    $(REP_DIR)/src/lib/python3/config.c $@

LICENSE:
	cp $(PORT_DIR)/src/lib/python3/LICENSE $@
