content: tzdb.dat hello.tar classes.tar servlet.tar 'Genode\ -\ Genode\ News.htm'

GENERATED_DIR := $(call port_dir,$(REP_DIR)/ports/jdk_generated)/src/app/jdk/bin

tzdb.dat:
	cp $(GENERATED_DIR)/$@ $@

hello.tar:
	cp $(GENERATED_DIR)/$@ $@

classes.tar:
	cp $(GENERATED_DIR)/$@ $@

servlet.tar:
	cp $(GENERATED_DIR)/$@ $@

'Genode\ -\ Genode\ News.htm':
	cp $(GENERATED_DIR)/$@ $@
