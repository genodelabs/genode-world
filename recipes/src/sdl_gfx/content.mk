content: src/lib lib/mk LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/sdl_gfx)

src/lib:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/lib/sdl_gfx $@

lib/mk:
	mkdir -p $@
	cp $(REP_DIR)/$@/sdl_gfx.mk $@

LICENSE:
	cp $(PORT_DIR)/src/lib/sdl_gfx/LICENSE $@
