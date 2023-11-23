content: src/lib/sdl2_ttf lib/mk LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/sdl2_ttf)

src/lib/sdl2_ttf:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/lib/sdl2_ttf/* $@

lib/mk:
	mkdir -p $@
	cp $(REP_DIR)/$@/sdl2_ttf.mk $@

LICENSE:
	cp $(PORT_DIR)/src/lib/sdl2_ttf/LICENSE.txt $@
