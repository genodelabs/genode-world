content: src/lib/sdl_ttf/target.mk lib/mk LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/sdl_ttf)

src/lib/sdl_ttf:
	mkdir -p $@
	cp $(PORT_DIR)/src/lib/sdl_ttf/*.c $@
	cp $(PORT_DIR)/src/lib/sdl_ttf/*.h $@

src/lib/sdl_ttf/target.mk: src/lib/sdl_ttf
	echo "LIBS += sdl_ttf" > $@

lib/mk:
	mkdir -p $@
	cp $(REP_DIR)/$@/sdl_ttf.mk $@

LICENSE:
	cp $(PORT_DIR)/src/lib/sdl_ttf/COPYING $@
