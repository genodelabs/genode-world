MIRROR_FROM_REP_DIR := lib/symbols/sdl lib/import/import-sdl.mk \

MIRROR_FROM_LIBPORTS := lib/mk/mesa_api.mk

content: $(MIRROR_FROM_REP_DIR) $(MIRROR_FROM_LIBPORTS) include LICENSE

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

$(MIRROR_FROM_LIBPORTS):
	mkdir -p $(dir $@)
	cp $(GENODE_DIR)/repos/libports/$@ $@

SDL_PORT_DIR  := $(call port_dir,$(REP_DIR)/ports/sdl)
MESA_PORT_DIR := $(call port_dir,$(GENODE_DIR)/repos/libports/ports/mesa)

#
# The Mesa header files needed for SDL_OpenGL are copied as well.
#
include:
	mkdir -p $@
	cp -r $(SDL_PORT_DIR)/include/SDL $@/
	cp -r $(REP_DIR)/include/SDL $@/
	cp -r $(MESA_PORT_DIR)/include/* $@/
	cp -r $(GENODE_DIR)/repos/libports/include/EGL $@/

LICENSE:
	cp $(SDL_PORT_DIR)/src/lib/sdl/COPYING $@
