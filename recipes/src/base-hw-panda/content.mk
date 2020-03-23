BOARD = panda

content: src/include src/core src/lib src/timer lib/mk LICENSE

src/include src/core src/lib src/timer lib/mk:
	mkdir -p $@
	cp -r $(GENODE_DIR)/repos/base/$@/* $@
	cp -r $(GENODE_DIR)/repos/base-hw/$@/* $@

LICENSE:
	cp $(GENODE_DIR)/LICENSE $@

content: lib/mk/spec/arm_v7/bootstrap-hw-panda.mk lib/mk/spec/arm_v7/core-hw-panda.mk

lib/mk/spec/arm_v7/bootstrap-hw-panda.mk lib/mk/spec/arm_v7/core-hw-panda.mk: lib/mk
	cp $(REP_DIR)/$@ $@

content: etc/specs.conf src/bootstrap

etc/specs.conf src/bootstrap:
	mkdir -p etc
	mkdir -p src
	cp -r $(GENODE_DIR)/repos/base-hw/$@ $@

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

content: enable_board_spec

enable_board_spec: etc/specs.conf
	echo "SPECS += panda" >> etc/specs.conf
