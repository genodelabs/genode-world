content: src/app/sdl2_playmus LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/sdl2_mixer)

src/app/sdl2_playmus:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/lib/sdl2_mixer/playmus.c $@
	cp -r $(REP_DIR)/src/app/sdl2_playmus/* $@

LICENSE:
	cp $(PORT_DIR)/src/lib/sdl2_mixer/LICENSE.txt $@
