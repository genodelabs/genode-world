content: src/lib/sdl_image lib/mk LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/sdl_image)

src/lib/sdl_image:
	mkdir -p $@
	cp $(PORT_DIR)/src/lib/sdl_image/*.c $@
	cp $(PORT_DIR)/src/lib/sdl_image/*.h $@

lib/mk:
	mkdir -p $@
	cp $(REP_DIR)/$@/sdl_image.mk $@

LICENSE:
	cp $(PORT_DIR)/src/lib/sdl_image/COPYING $@
