content: src/lib/sdl2_net lib/mk LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/sdl2_net)

src/lib/sdl2_net:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/lib/sdl2_net/* $@

lib/mk:
	mkdir -p $@
	cp $(REP_DIR)/$@/sdl2_net.mk $@

LICENSE:
	cp $(PORT_DIR)/src/lib/sdl2_net/LICENSE.txt $@
