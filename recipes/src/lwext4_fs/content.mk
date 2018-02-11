PORT_DIR := $(call port_dir,$(REP_DIR)/ports/lwext4)

MIRROR_FROM_REP_DIR := lib/mk/lwext4.mk \
                       lib/import/import-lwext4.mk \
                       src/server/lwext4_fs \
                       include/lwext4

content: $(MIRROR_FROM_REP_DIR) src/lib/lwext4

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

src/lib/lwext4:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/lib/lwext4/* $@
	cp -r $(REP_DIR)/src/lib/lwext4/* $@

content: LICENSE
LICENSE:
	( echo "Lwext4 is subject to GNU General Public License version 2, see:"; \
	  echo "  src/lib/lwext4/LICENSE" ) > $@
