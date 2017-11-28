content: lib/mk/snes9x_libretro.mk src/libretro/snes9x LICENSE

CORE_DIR := $(call port_dir,$(REP_DIR)/ports/snes9x-libretro)/src/libretro/snes9x

lib/mk/snes9x_libretro.mk:
	$(mirror_from_rep_dir)

src/libretro/snes9x:
	$(mirror_from_rep_dir)
	cp -r $(CORE_DIR)/* $@
	echo "LIBS = snes9x_libretro" > $@/target.mk

LICENSE:
	cp $(CORE_DIR)/docs/snes9x-license.txt LICENSE
