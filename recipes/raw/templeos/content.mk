content: TempleOS.ISO TempleOS.vbox

ISO_URL = http://archive.org/download/TempleOS20171215/TempleOS.ISO

TempleOS.ISO: 
	wget $(ISO_URL) -O $@

TempleOS.vbox:
	cp $(REP_DIR)/recipes/raw/templeos/$@ $@
