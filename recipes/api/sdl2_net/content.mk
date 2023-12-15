MIRROR_FROM_REP_DIR := lib/symbols/sdl2_net lib/import/import-sdl2_net.mk

content: $(MIRROR_FROM_REP_DIR) include LICENSE FindSDL2_net.cmake

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR  := $(call port_dir,$(REP_DIR)/ports/sdl2_net)

include:
	mkdir -p $@
	cp -r $(PORT_DIR)/include/SDL2 $@/

LICENSE:
	cp $(PORT_DIR)/src/lib/sdl2_net/LICENSE.txt $@

FindSDL2_net.cmake:
	echo 'set(SDL2_NET_FOUND True)' > $@
	echo 'set(SDL2_NET_INCLUDE_DIR "$${CMAKE_CURRENT_LIST_DIR}/include/SDL2")' >> $@
	echo 'set(SDL2_NET_LIBRARY ":sdl2_net.lib.so")' >> $@
	echo 'set(SDL2_NET_LIBRARIES "$${SDL2_NET_LIBRARY}")' >> $@
	echo 'add_library(SDL2_net::SDL2_net INTERFACE IMPORTED)' >> $@
	echo 'set_target_properties(SDL2_net::SDL2_net PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "$${SDL2_NET_INCLUDE_DIR}")' >> $@
