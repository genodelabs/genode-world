MIRROR_FROM_REP_DIR := lib/symbols/sdl2_image lib/import/import-sdl2_image.mk

content: $(MIRROR_FROM_REP_DIR) include LICENSE FindSDL2_image.cmake

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR  := $(call port_dir,$(REP_DIR)/ports/sdl2_image)

include:
	mkdir -p $@
	cp -r $(PORT_DIR)/include/SDL2 $@/

LICENSE:
	cp $(PORT_DIR)/src/lib/sdl2_image/LICENSE.txt $@

FindSDL2_image.cmake:
	echo 'set(SDL2_IMAGE_FOUND True)' > $@
	echo 'set(SDL2_IMAGE_INCLUDE_DIR "$${CMAKE_CURRENT_LIST_DIR}/include/SDL2")' >> $@
	echo 'set(SDL2_IMAGE_LIBRARY ":sdl2_image.lib.so")' >> $@
	echo 'set(SDL2_IMAGE_LIBRARIES "$${SDL2_IMAGE_LIBRARY}")' >> $@
	echo 'add_library(SDL2_image::SDL2_image INTERFACE IMPORTED)' >> $@
	echo 'set_target_properties(SDL2_image::SDL2_image PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "$${SDL2_IMAGE_INCLUDE_DIR}")' >> $@
