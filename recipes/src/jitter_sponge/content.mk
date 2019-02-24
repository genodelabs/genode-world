SRC_DIR = src/server/jitter_sponge
include $(GENODE_DIR)/repos/base/recipes/src/content.inc

MIRROR_FROM_REP_DIR = \
	include/spec/x86_64/world/rdrand.h \
	include/world/rdrand.h \
	lib/import/import-libkeccak.mk \
	lib/mk/libkeccak.inc \
	lib/mk/spec/32bit/libkeccak.mk \
	lib/mk/spec/64bit/libkeccak.mk \
	src/lib/keccak \

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

XKCP_PORT_DIR := $(call port_dir,$(REP_DIR)/ports/xkcp)
MIRROR_FROM_XKCP = generic32 generic64

generic32: $(XKCP_PORT_DIR)/generic32
	cp -r $< $@

generic64: $(XKCP_PORT_DIR)/generic64
	cp -r $< $@

content: $(MIRROR_FROM_REP_DIR) $(MIRROR_FROM_XKCP)
