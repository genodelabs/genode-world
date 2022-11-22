SRC_DIR                := src/lib/jsonc
PORT_DIR               := $(call port_dir,$(REP_DIR)/ports/jsonc)

MIRROR_FROM_REP_DIR    := lib/mk/jsonc.mk
MIRROR_FROM_REP_DIR    += lib/import/import-jsonc.mk

MIRROR_FROM_PORT       := src/lib/jsonc

content: $(MIRROR_FROM_REP_DIR) $(MIRROR_FROM_PORT) LICENSE

$(MIRROR_FROM_PORT):
	mkdir -p $(dir $@)
	cp -ar $(PORT_DIR)/$@ $@
	echo "LIBS := jsonc" > $@/target.mk

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/src/lib/jsonc/COPYING $@
