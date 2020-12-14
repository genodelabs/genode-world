MIRROR_FROM_REP_DIR := lib/symbols/sdl2 lib/import/import-sdl2.mk

content: $(MIRROR_FROM_REP_DIR) include LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

SDL2_PORT_DIR  := $(call port_dir,$(REP_DIR)/ports/sdl2)

include:
	mkdir -p $@
	cp -r $(SDL2_PORT_DIR)/include/SDL2 $@/
	cp -r $(REP_DIR)/include/SDL2 $@/

LICENSE:
	cp $(SDL2_PORT_DIR)/src/lib/sdl2/COPYING.txt $@
