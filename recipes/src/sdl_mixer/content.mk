content: src/lib/sdl_mixer lib/mk LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/sdl_mixer)

src/lib/sdl_mixer:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/lib/sdl_mixer/* $@

lib/mk:
	mkdir -p $@
	cp $(REP_DIR)/$@/sdl_mixer.mk $@

LICENSE:
	cp $(PORT_DIR)/src/lib/sdl_mixer/COPYING $@
