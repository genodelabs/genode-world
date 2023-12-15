MIRROR_FROM_REP_DIR := lib/symbols/sdl2 lib/import/import-sdl2.mk

content: $(MIRROR_FROM_REP_DIR) include LICENSE FindSDL2.cmake

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

SDL2_PORT_DIR  := $(call port_dir,$(REP_DIR)/ports/sdl2)

include:
	mkdir -p $@
	cp -r $(SDL2_PORT_DIR)/include/SDL2 $@/
	cp -r $(REP_DIR)/include/SDL2 $@/

LICENSE:
	cp $(SDL2_PORT_DIR)/src/lib/sdl2/COPYING.txt $@

FindSDL2.cmake:
	echo 'set(SDL2_FOUND True)' > $@
	echo 'set(SDL2_INCLUDE_DIR "$${CMAKE_CURRENT_LIST_DIR}/include/SDL2")' >> $@
	echo 'set(SDL2_LIBRARY ":sdl2.lib.so")' >> $@
	echo 'set(SDL2_LIBRARIES "$${SDL2_LIBRARY}")' >> $@
	echo 'add_library(SDL2::SDL2 INTERFACE IMPORTED)' >> $@
	echo 'add_library(SDL2::SDL2main ALIAS SDL2::SDL2)' >> $@
	echo 'set_target_properties(SDL2::SDL2 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "$${SDL2_INCLUDE_DIR}")' >> $@
