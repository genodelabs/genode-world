content: src/lib/sdl2 lib/mk LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/sdl2)

src/lib/sdl2:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/lib/sdl2/* $@
	cp -r $(REP_DIR)/src/lib/sdl2/* $@

lib/mk:
	mkdir -p $@
	cp $(REP_DIR)/$@/sdl2.mk $@

LICENSE:
	cp $(PORT_DIR)/src/lib/sdl2/COPYING.txt $@
