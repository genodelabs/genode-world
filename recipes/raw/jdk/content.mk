content: tzdb.dat hello.tar classes.tar

GENERATED_DIR := $(call port_dir,$(REP_DIR)/ports/jdk_generated)/src/app/jdk/bin

tzdb.dat:
	cp $(GENERATED_DIR)/$@ $@

hello.tar:
	cp $(GENERATED_DIR)/$@ $@

classes.tar:
	cp $(GENERATED_DIR)/$@ $@
