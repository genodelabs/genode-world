content: abuse.tar README

ABUSE_DIR := $(call port_dir,$(REP_DIR)/ports/abuse)/src/app/abuse

TAR_OPT := --owner=0 --group=0 --numeric-owner --mode='go=' --mtime='1970-01-01 00:00+00'

abuse.tar: $(ABUSE_DIR)/data
	tar $(TAR_OPT) -cf $@ -C $< .

README: $(ABUSE_DIR)/COPYING
	cp $< $@
