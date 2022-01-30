
TARGET ?= $(lastword $(subst /, ,$(PRG_DIR)))
PKG    ?= $(TARGET)

SRC_GO_PK := $(foreach s,$(SRC_GO_PKG),$(realpath $(PRG_DIR)/$s))

GOROOT:=$(BUILD_BASE_DIR)/app/goroot

GOOS:=inno

ifeq ($(findstring x86_64,$(SPECS)),x86_64)
GOARCH:=amd64
else ifeq ($(findstring arm_64,$(SPECS)),arm_64)
GOARCH:=arm64
else
$(error unsipported arch SPECS $(SPECS))
endif

# substitute $libs variable in generated LD_CMD and remove call to gcc++
CGO_LDFLAGS = $(subst $$libs,$(LIB_CACHE_DIR),$(filter-out $(firstword $(LD_CMD)) ,$(LD_CMD)))


env.sh: $(PRG_DIR)/target.mk
	$(VERBOSE)rm -f $@
	$(VERBOSE)echo "export CC='$(CC)'" >> $@
	$(VERBOSE)echo "export CXX='$(CXX)'" >> $@
	$(VERBOSE)echo "export LD='$(CUSTOM_LD)'" >> $@
	$(VERBOSE)echo "export AR='$(AR)'" >> $@
	$(VERBOSE)echo "export NM='$(NM)'" >> $@
	$(VERBOSE)echo "export RANLIB='$(RANLIB)'" >> $@
	$(VERBOSE)echo "export STRIP='$(STRIP)'" >> $@
	$(VERBOSE)echo "export CPPFLAGS='$(CPPFLAGS)'" >> $@
	$(VERBOSE)echo "export CFLAGS='$(CFLAGS)'" >> $@
	$(VERBOSE)echo "export CXXFLAGS='$(CXXFLAGS)'" >> $@
	$(VERBOSE)echo "export LDFLAGS='$(LDFLAGS)'" >> $@
	$(VERBOSE)echo "export LIBS='$(LDLIBS)'" >> $@
	$(VERBOSE)echo "export LIBTOOLFLAGS='$(LIBTOOLFLAGS)'" >> $@
	$(VERBOSE)echo "export PS1='<go_build>'" >> $@
	$(VERBOSE)echo "export GOOS='$(GOOS)'" >> $@
	$(VERBOSE)echo "export GOARCH='$(GOARCH)'" >> $@
	$(VERBOSE)echo "export GCCGO='$(CUSTOM_GO)'" >> $@
	$(VERBOSE)echo "export CGO_LDFLAGS='$(CGO_LDFLAGS)'" >> $@
	$(VERBOSE)echo "export GOROOT='$(GOROOT)'" >> $@
	$(VERBOSE)echo "export GOPATH='$(LIBGO_PKG_BUILD)'" >> $@
	$(VERBOSE)echo "export PATH=$(GOROOT)/bin:${PATH}" >> $@


$(TARGET): env.sh $(SRC_GO_PKG) $(SHARED_LIBS) $(GOROOT)/bin
	$(VERBOSE)for files in $(SRC_GO_PK); do \
		ln -sf $${files} .; \
	done
	$(VERBOSE)if [ ! -f go.mod ]; then \
		source ./env.sh && go mod init $(PKG); \
	fi
	$(VERBOSE)source ./env.sh && go mod tidy
	$(VERBOSE)source ./env.sh && go build -x -compiler gccgo -gccgoflags 'all=$(CUSTOM_GO_FLAGS)' -a -o $@ $(SRC_GO_PK)

STRIP_TARGET_CMD ?= $(STRIP) -o $@ $<

$(TARGET).stripped: $(TARGET)
	$(VERBOSE)$(STRIP_TARGET_CMD)

$(INSTALL_DIR)/$(TARGET): $(TARGET).stripped
	$(VERBOSE)ln -sf $(CURDIR)/$< $@

ifneq ($(DEBUG_DIR),)
$(DEBUG_DIR)/$(TARGET): $(TARGET)
	$(VERBOSE)ln -sf $(CURDIR)/$< $@
endif

LIBS   = base libc libm libgo libgo_support stdcxx
