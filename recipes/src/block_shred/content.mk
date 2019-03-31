PORT_DIR = $(call port_dir,$(GENODE_DIR)/repos/libports/ports/pcg-c)

pcg_srcs = $(notdir $(wildcard $(PORT_DIR)/src/lib/pcg-c/src/*-$(1).c))

MIRROR_FROM_PORT_DIR := $(addprefix src/lib/pcg-c/src/,\
	$(call pcg_srcs,8) \
	$(call pcg_srcs,16) \
	$(call pcg_srcs,32) \
	$(call pcg_srcs,64) \
	$(call pcg_srcs,128) ) \
	include/pcg-c/pcg_variants.h

MIRROR_FROM_REP_DIR = include/world/rdrand.h

content: $(MIRROR_FROM_PORT_DIR) $(MIRROR_FROM_REP_DIR)

$(MIRROR_FROM_PORT_DIR):
	mkdir -p $(dir $@)
	cp -r $(PORT_DIR)/$@ $@

content:
	mkdir -p lib/mk lib/import include/pcg-c
	cp $(GENODE_DIR)/repos/libports/lib/mk/libpcg_random.mk lib/mk
	cp $(GENODE_DIR)/repos/libports/lib/import/import-libpcg_random.mk lib/import
	cp $(GENODE_DIR)/repos/libports/include/pcg-c/genode_inttypes.h include/pcg-c

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

SRC_DIR = src/app/block_shred
include $(GENODE_DIR)/repos/base/recipes/src/content.inc
