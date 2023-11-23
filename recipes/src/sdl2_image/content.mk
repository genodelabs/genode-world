content: src/lib/sdl2_image lib/mk LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/sdl2_image)

src/lib/sdl2_image:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/lib/sdl2_image/* $@

lib/mk:
	mkdir -p $@
	cp $(REP_DIR)/$@/sdl2_image.mk $@

LICENSE:
	cp $(PORT_DIR)/src/lib/sdl2_image/LICENSE.txt $@
