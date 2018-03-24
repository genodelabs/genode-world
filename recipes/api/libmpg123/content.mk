#
# libmpg123 has an API version number that this package
# should be pegged to, see MPG123_API_VERSION in mpg123.h
#

content: include lib/symbols/libmpg123 LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/mpg123)

include:
	cp -r $(PORT_DIR)/$@ $@

lib/symbols/libmpg123:
	$(mirror_from_rep_dir)

LICENSE:
	cp $(PORT_DIR)/src/lib/mpg123/COPYING $@
