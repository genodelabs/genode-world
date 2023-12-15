MIRROR_FROM_REP_DIR := lib/symbols/sdl2_mixer lib/import/import-sdl2_mixer.mk

content: $(MIRROR_FROM_REP_DIR) include LICENSE FindSDL2_mixer.cmake

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR  := $(call port_dir,$(REP_DIR)/ports/sdl2_mixer)

include:
	mkdir -p $@
	cp -r $(PORT_DIR)/include/SDL2 $@/

LICENSE:
	cp $(PORT_DIR)/src/lib/sdl2_mixer/LICENSE.txt $@

FindSDL2_mixer.cmake:
	echo 'set(SDL2_MIXER_FOUND True)' > $@
	echo 'set(SDL2_MIXER_INCLUDE_DIR "$${CMAKE_CURRENT_LIST_DIR}/include/SDL2")' >> $@
	echo 'set(SDL2_MIXER_LIBRARY ":sdl2_mixer.lib.so")' >> $@
	echo 'set(SDL2_MIXER_LIBRARIES "$${SDL2_MIXER_LIBRARY}")' >> $@
	echo 'add_library(SDL2_mixer::SDL2_mixer INTERFACE IMPORTED)' >> $@
	echo 'set_target_properties(SDL2_mixer::SDL2_mixer PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "$${SDL2_MIXER_INCLUDE_DIR}")' >> $@
