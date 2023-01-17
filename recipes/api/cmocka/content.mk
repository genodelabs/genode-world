MIRROR_FROM_REP_DIR := lib/import/import-cmocka.mk
MIRROR_FROM_REP_DIR += lib/symbols/cmocka

content: $(MIRROR_FROM_REP_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/cmocka)

content: include/cmocka LICENSE

include/cmocka:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/lib/cmocka/include/*.h $@/

LICENSE:
	cp $(PORT_DIR)/src/lib/cmocka/doc/that_style/LICENSE $@
