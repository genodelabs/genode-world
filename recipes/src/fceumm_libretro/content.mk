content: lib/mk/fceumm_libretro.mk src/libretro/fceumm LICENSE

CORE_DIR := $(call port_dir,$(REP_DIR)/ports/fceumm-libretro)/src/libretro/fceumm

lib/mk/fceumm_libretro.mk:
	$(mirror_from_rep_dir)

src/libretro/fceumm:
	$(mirror_from_rep_dir)
	cp -r $(CORE_DIR)/* $@
	echo "LIBS = fceumm_libretro" > $@/target.mk

LICENSE:
	cp $(CORE_DIR)/Copying LICENSE
