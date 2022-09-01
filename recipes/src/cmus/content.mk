APP_DIR := src/app/cmus

MIRROR_FROM_PORT_AND_REP_DIR := $(APP_DIR)

MIRROR_FROM_REP_DIR := \
	lib/mk/cmus_ip_flac.mk \
	lib/mk/cmus_ip_mad.mk \
	lib/mk/cmus_ip_opus.mk \
	lib/mk/cmus_ip_vorbis.mk \
	lib/mk/cmus_op_oss.mk

OPUSFILE_PORT_DIR := $(call port_dir,$(REP_DIR)/ports/opusfile)

MIRROR_FROM_REP_DIR += \
	lib/import/import-opusfile.mk \
	lib/mk/opusfile.mk

content: $(MIRROR_FROM_PORT_AND_REP_DIR) $(MIRROR_FROM_REP_DIR) LICENSE \
         src/lib/opusfile remove-git

src/lib/opusfile:
	mkdir -p include
	cp $(OPUSFILE_PORT_DIR)/include/opusfile/opusfile.h include
	mkdir -p src/lib/opusfile
	cp -a $(OPUSFILE_PORT_DIR)/src/lib/opusfile src/lib

remove-git: $(MIRROR_FROM_PORT_DIR)
	@for dir in .git; do \
		rm -rf src/app/cmus/$$dir; \
	done

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/cmus)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

$(MIRROR_FROM_PORT_AND_REP_DIR):
	mkdir -p $(dir $@)
	cp -r $(PORT_DIR)/$@ $(dir $@)
	$(mirror_from_rep_dir)

LICENSE:
	(echo "cmus is subject to the license specified in src/cmus/app/COPYING;" \
	 echo "opusfile is subject to the license specified in src/lib/opusfile/COPYING"; )> $@
