content: abuse.tar README

ABUSE_DIR := $(call port_dir,$(REP_DIR)/ports/abuse)/src/app/abuse

abuse.tar: $(ABUSE_DIR)/data
	tar cf $@ -C $< .

README: $(ABUSE_DIR)/COPYING
	cp $< $@
