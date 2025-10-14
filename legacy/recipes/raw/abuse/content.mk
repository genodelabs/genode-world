content: abuse.tar README

ABUSE_DIR := $(call port_dir,$(REP_DIR)/ports/abuse)/src/app/abuse

include $(GENODE_DIR)/repos/base/recipes/content.inc

abuse.tar: $(ABUSE_DIR)/data
	$(TAR) -cf $@ -C $< .

README: $(ABUSE_DIR)/COPYING
	cp $< $@
