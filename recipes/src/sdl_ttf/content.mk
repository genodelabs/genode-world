content: src/lib/sdl_ttf lib/mk LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/sdl_ttf)

src/lib/sdl_ttf:
	mkdir -p $@
	cp $(PORT_DIR)/src/lib/sdl_ttf/*.c $@
	cp $(PORT_DIR)/src/lib/sdl_ttf/*.h $@

lib/mk:
	mkdir -p $@
	cp $(REP_DIR)/$@/sdl_ttf.mk $@

LICENSE:
	cp $(PORT_DIR)/src/lib/sdl_ttf/COPYING $@
