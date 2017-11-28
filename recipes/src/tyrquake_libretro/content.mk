content: lib/mk/tyrquake_libretro.mk src/libretro/tyrquake LICENSE

CORE_DIR := $(call port_dir,$(REP_DIR)/ports/tyrquake-libretro)/src/libretro/tyrquake

lib/mk/tyrquake_libretro.mk:
	$(mirror_from_rep_dir)

src/libretro/tyrquake:
	$(mirror_from_rep_dir)
	cp -r $(CORE_DIR)/* $@
	echo "LIBS = tyrquake_libretro" > $@/target.mk

LICENSE:
	cp $(CORE_DIR)/gnu.txt LICENSE
