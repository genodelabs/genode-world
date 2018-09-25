content: src/app/umurmur LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/umurmur)

src/app/umurmur:
	mkdir -p $@
	cp $(PORT_DIR)/src/app/umurmur/src/* $@
	cp $(REP_DIR)/src/app/umurmur/* $@
	rm $@/umurmur.patch

LICENSE:
	cp $(PORT_DIR)/src/app/umurmur/LICENSE $@
