content: src/lib/sdl2_mixer lib/mk LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/sdl2_mixer)

src/lib/sdl2_mixer:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/lib/sdl2_mixer/* $@

lib/mk:
	mkdir -p $@
	cp $(REP_DIR)/$@/sdl2_mixer.mk $@

LICENSE:
	cp $(PORT_DIR)/src/lib/sdl2_mixer/LICENSE.txt $@
