MIRROR_FROM_REP_DIR := lib/symbols/sdl_gfx

content: $(MIRROR_FROM_REP_DIR) include LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR  := $(call port_dir,$(REP_DIR)/ports/sdl_gfx)

include:
	mkdir -p $@
	cp -r $(PORT_DIR)/include/* $@/

LICENSE:
	cp $(PORT_DIR)/src/lib/sdl_gfx/LICENSE $@
