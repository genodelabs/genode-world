
TARGET ?= $(lastword $(subst /, ,$(PRG_DIR)))
PKG    ?= $(TARGET)

SRC_GO_PK := $(foreach s,$(SRC_GO_PKG),$(realpath $(PRG_DIR)/$s))

GOOS=inno
GOARCH=amd64


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
	$(VERBOSE)echo "export GOROOT=/var/services/homes/admin/gen/goroot" >> $@
	$(VERBOSE)echo "export PATH=/var/services/homes/admin/gen/goroot/bin:${PATH}" >> $@


$(TARGET): env.sh $(SRC_GO_PKG) $(SHARED_LIBS)
	$(VERBOSE)for files in $(SRC_GO_PK); do \
		ln -sf $${files} .; \
	done
	$(VERBOSE)source ./env.sh && go mod init $(PKG)
	$(VERBOSE)source ./env.sh && go mod tidy
	$(VERBOSE)source ./env.sh && go build -x -compiler gccgo -a -o $@ $(SRC_GO_PK)

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
