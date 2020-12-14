content: src/app/scrcpy LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/scrcpy)

src/app/scrcpy:
	mkdir -p $@
	mkdir -p $@/sys/unix
	cp $(PORT_DIR)/src/app/scrcpy/app/src/sys/unix/command.c $@/sys/unix/.
	mkdir -p $@/app/src
	cp -r $(PORT_DIR)/src/app/scrcpy/app/src/* $@/app/src/.
	mkdir -p $@/util
	cp $(PORT_DIR)/src/app/scrcpy/app/src/util/*.c $@/util/.
	cp -r $(REP_DIR)/$@/* $@/

LICENSE:
	cp $(PORT_DIR)/src/app/scrcpy/LICENSE $@
