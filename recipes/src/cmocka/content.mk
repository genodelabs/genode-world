MIRROR_FROM_REP_DIR := lib/mk/cmocka.mk
MIRROR_FROM_REP_DIR += lib/import/import-cmocka.mk

content: $(MIRROR_FROM_REP_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

content: src/lib/cmocka/target.mk LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/cmocka)

src/lib/cmocka:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/lib/cmocka/* $@/
	cp $(REP_DIR)/src/lib/cmocka/config.h $@/

src/lib/cmocka/target.mk: src/lib/cmocka
	echo "LIBS += cmocka" > $@

LICENSE:
	cp $(PORT_DIR)/src/lib/cmocka/doc/that_style/LICENSE $@
