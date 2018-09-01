content: TempleOS.ISO TempleOS.vbox

ISO_URL = http://www.templeos.org/TempleOS.ISO

TempleOS.ISO: 
	wget $(ISO_URL) -O $@

TempleOS.vbox:
	cp $(REP_DIR)/recipes/raw/templeos/$@ $@
