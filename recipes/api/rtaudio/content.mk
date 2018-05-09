MIRROR_FROM_PORT_DIR = include src/lib/rtaudio

content: $(MIRROR_FROM_PORT_DIR) lib/mk/rtaudio.mk lib/import/import-rtaudio.mk LICENSE
PORT_DIR := $(call port_dir,$(REP_DIR)/ports/rtaudio)

$(MIRROR_FROM_PORT_DIR):
	mkdir -p $(dir $@)
	cp -r $(PORT_DIR)/$@ $(dir $@)

lib/mk/rtaudio.mk:
	$(mirror_from_rep_dir)
lib/import/import-rtaudio.mk:
	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/src/lib/rtaudio/readme $@
