content: include/libretro.h LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libretro)

include/libretro.h: $(PORT_DIR)/include/libretro.h
	mkdir $(dir $@)
	cp -r  $< $@

LICENSE: include/libretro.h
	head -n 22 include/libretro.h > $@
