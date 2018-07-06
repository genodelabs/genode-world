TARGET     ?= $(lastword $(subst /, ,$(PRG_DIR)))
NIMBLE_PKG ?= $(TARGET)
LIBS        = base libc

CC_CXX_WARN_STRICT =

$(TARGET): assemble.tag $(SHARED_LIBS)
	libs=$(LIB_CACHE_DIR); $(LD_CMD) $(wildcard src/nimcache/*.o) -o $(TARGET)
	ln -sf $(CURDIR)/$@ $(INSTALL_DIR)/$@

assemble.tag: copy.tag nim.cfg
	nimble --verbose cpp src/$(TARGET)
	@touch $@

nim.cfg:
	rm -f $@
	echo "-d:nimDebugDlOpen" >> $@
	echo "--os:genode" >> $@
	echo "--cpu:$(NIM_CPU)" >> $@
	echo "--noCppExceptions" >> $@
	echo "--noLinking" >> $@
	echo "--passC:\"$(CXX_DEF) $(CC_CXX_OPT) $(INCLUDES) -fpermissive\"" >> $@
	echo "$(NIM_OPT)" >> $@

NIMBLE_PATH := $(shell nimble path $(NIMBLE_PKG))

copy.tag:
	nimble install -n https://github.com/ehmry/nim-genode
	cp -avu $(PRG_DIR)/* .

.PHONY: nim.cfg
