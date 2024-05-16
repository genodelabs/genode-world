content: src/app/sdl_vnc LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libvnc)

src/app/sdl_vnc:
	mkdir -p $@
	cp -r $(REP_DIR)/$@/* $@/.
	cp $(PORT_DIR)/src/lib/vnc/client_examples/SDLvncviewer.c $@/.

LICENSE:
	cp $(PORT_DIR)/src/lib/vnc/COPYING LICENSE
