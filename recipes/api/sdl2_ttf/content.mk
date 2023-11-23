MIRROR_FROM_REP_DIR := lib/symbols/sdl2_ttf lib/import/import-sdl2_ttf.mk

content: $(MIRROR_FROM_REP_DIR) include LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR  := $(call port_dir,$(REP_DIR)/ports/sdl2_ttf)

include:
	mkdir -p $@
	cp -r $(PORT_DIR)/include/SDL2 $@/

LICENSE:
	cp $(PORT_DIR)/src/lib/sdl2_ttf/LICENSE.txt $@
