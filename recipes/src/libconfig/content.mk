MIRROR_FROM_REP_DIR = lib/import/import-libconfig.mk lib/mk/libconfig.mk

content: $(MIRROR_FROM_REP_DIR) src/lib/libconfig/target.mk LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libconfig)

src/lib/libconfig/target.mk:
	mkdir -p src/lib/libconfig/lib
	cp -r $(PORT_DIR)/src/lib/libconfig/lib/*.c \
	      $(PORT_DIR)/src/lib/libconfig/lib/*.h src/lib/libconfig/lib

LICENSE:
	cp $(PORT_DIR)/src/lib/libconfig/LICENSE $@
