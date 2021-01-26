content: src/include src/core src/lib src/timer lib/mk LICENSE

src/lib src/timer lib/mk:
	mkdir -p $@
	cp -r $(GENODE_DIR)/repos/base/$@/* $@
	cp -r $(GENODE_DIR)/repos/base-hw/$@/* $@

src/include src/core:
	mkdir -p $@
	cp -r $(GENODE_DIR)/repos/base/$@/* $@
	cp -r $(GENODE_DIR)/repos/base-hw/$@/* $@
	cp -r $(REP_DIR)/$@/* $@

LICENSE:
	cp $(GENODE_DIR)/LICENSE $@

content: lib/mk/spec/$(ARCH)/bootstrap-hw-$(BOARD).mk lib/mk/spec/$(ARCH)/core-hw-$(BOARD).mk

lib/mk/spec/$(ARCH)/bootstrap-hw-$(BOARD).mk lib/mk/spec/$(ARCH)/core-hw-$(BOARD).mk: lib/mk
	cp $(REP_DIR)/$@ $@

content: etc/specs.conf src/bootstrap

etc/specs.conf:
	mkdir -p etc
	cp -r $(GENODE_DIR)/repos/base-hw/$@ $@

src/bootstrap:
	cp -r $(GENODE_DIR)/repos/base-hw/$@ $@
	cp -r $(REP_DIR)/$@/* $@/

content: generalize_target_names remove_other_board_libs

generalize_target_names: lib/mk src/lib src/timer
	for spec in arm riscv x86_64; do \
	  mv lib/mk/spec/$$spec/ld-hw.mk lib/mk/spec/$$spec/ld.mk; \
	  done;
	sed -i "s/ld-hw/ld/"           src/lib/ld/hw/target.mk
	sed -i "s/hw_timer_drv/timer/" src/timer/hw/target.mk

remove_other_board_libs: lib/mk
	find lib/mk/spec -name core-hw-*.mk -o -name bootstrap-hw-*.mk |\
		grep -v "hw-$(BOARD).mk" | xargs rm -rf
