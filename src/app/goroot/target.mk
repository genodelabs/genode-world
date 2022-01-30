TARGET = goroot

GOOS:=inno
GOARCH:=arm64

GOROOT_PORT_DIR := $(call select_from_ports,goroot)
GOROOT_DIR      := $(GOROOT_PORT_DIR)/src/app/goroot

env.sh: $(PRG_DIR)/target.mk
	$(VERBOSE)rm -f $@
	$(VERBOSE)echo "unset CC CXX LD AR NM RANLIB STRIP CPPFLAGS"   >> $@
	$(VERBOSE)echo "unset LIBTOOLFLAGS GCCGO CFLAGS LDFLAGS LIBS"  >> $@
	$(VERBOSE)echo "export GOROOT='$(BUILD_BASE_DIR)/app/goroot'" >> $@
	# avoid most of the tests
	$(VERBOSE)echo "export GOTESTONLY='!'" >> $@

$(TARGET): prepare_goroot
	$(VERBOSE)source ./env.sh && cd src && ./all.bash

prepare_goroot: env.sh
	$(VERBOSE)cp -aprs $(GOROOT_DIR)/* .
