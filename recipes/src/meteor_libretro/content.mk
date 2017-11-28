content: lib/mk/meteor_libretro.mk src/libretro/meteor LICENSE

CORE_DIR := $(call port_dir,$(REP_DIR)/ports/meteor-libretro)/src/libretro/meteor

lib/mk/meteor_libretro.mk:
	$(mirror_from_rep_dir)

src/libretro/meteor:
	$(mirror_from_rep_dir)
	cp -r $(CORE_DIR)/* $@
	echo "LIBS = meteor_libretro" > $@/target.mk

LICENSE:
	cp $(CORE_DIR)/COPYING LICENSE
