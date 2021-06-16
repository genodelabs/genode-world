MIRROR_FROM_REP_DIR = lib/import/import-libmbim.mk lib/mk/libmbim.mk src/lib/libmbim/config.h

content: $(MIRROR_FROM_REP_DIR) src/lib/libmbim/target.mk LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libmbim)
GENERATED_PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libmbim_generated)

src/lib/libmbim/target.mk:
	mkdir -p src/lib/libmbim
	mkdir -p src/lib/libmbim_generated
	cp -r $(PORT_DIR)/src/lib/libmbim/* src/lib/libmbim
	cp -r $(GENERATED_PORT_DIR)/src/lib/libmbim_generated/* src/lib/libmbim_generated
	echo "LIBS = libmbim" > $@

LICENSE:
	cp $(PORT_DIR)/src/lib/libmbim/COPYING $@
