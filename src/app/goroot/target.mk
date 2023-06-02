TARGET = goroot

GOOS:=genode
GOARCH:=$(ARCH)

GOROOT_PORT_DIR := $(call select_from_ports,goroot)
GOROOT_DIR      := $(GOROOT_PORT_DIR)/src/app/goroot

# need build libgo
LIBS += libgo

env.sh: $(PRG_DIR)/target.mk
	$(VERBOSE)rm -f $@
	$(VERBOSE)echo "unset CC CXX LD AR NM RANLIB STRIP CPPFLAGS"   >> $@
	$(VERBOSE)echo "unset LIBTOOLFLAGS GCCGO CFLAGS LDFLAGS LIBS"  >> $@
	$(VERBOSE)echo "export GOROOT='$(BUILD_BASE_DIR)/app/goroot'" >> $@
	# avoid most of the tests
	$(VERBOSE)echo "export GOTESTONLY='!'" >> $@

$(TARGET): prepare_goroot
	# FIX some files not allowed to be links during compilation
	$(VERBOSE)rm ./src/crypto/elliptic/p256_asm_table.bin
	$(VERBOSE)cp $(GOROOT_DIR)/src/crypto/elliptic/p256_asm_table.bin \
		./src/crypto/elliptic/p256_asm_table.bin
	$(VERBOSE)rm -rf ./src/cmd/vendor/github.com/google/pprof/internal/driver/html/
	$(VERBOSE)cp -r $(GOROOT_DIR)/src/cmd/vendor/github.com/google/pprof/internal/driver/html \
		./src/cmd/vendor/github.com/google/pprof/internal/driver/
	$(VERBOSE)source ./env.sh && cd src && ./all.bash && mv ../bin/go ../bin/genode-go

prepare_goroot: env.sh $(CURDIR)/toolchain/gcc/libgo/
	$(VERBOSE)cp -aprsf $(GOROOT_DIR)/* .

# prepare toolchain/libgo packages

TOOLS_BASE = $(realpath $(dir $(CROSS_DEV_PREFIX))/..)

$(CURDIR)/toolchain/gcc/libgo/: 
	@echo \*\*\* prepare local toolchain
	$(VERBOSE)mkdir -p $@
	$(VERBOSE)ln -sf $(TOOLS_BASE)/libexec $@../..
	$(VERBOSE)ln -sf $(TOOLS_BASE)/$(shell $(GCC) -dumpmachine) $@../..
	$(VERBOSE)cd $(LIBGO_PKG_BUILD) && find . -name \*.gox -printf '%h\n' | xargs -i mkdir -p $@\{}
	$(VERBOSE)cd $@ && find $(LIBGO_PKG_BUILD) -name \*.gox -printf '%p %P\n'  | xargs -n2 ln -s
