#
# \brief build support file for go
# \author Alexander Tormasov
# \date 2022 - 05 - 27
#

#to be defined in target.mk
# SRC_GO_PKG - list of go files to be compiled (e.g. "a.go b.go")
#            - OR directory with go package like github.com/woojiahao/tower-of-hanoi-solver
# GO_BUILD_LIB - if yes then force lib name for "-o", otherwise just a name of dir
# optional
# SRC_GO_PKG_MAIN - directory where go files with "package main" resides
# by default equal to SRC_GO_PKG if it is directory (or CURDIR where "go build ..." will run)
# single base packet downloaded with no-mod mode, others in mod
# mods stored in libgo; use goroot
# GOBUILD_VERBOSE can be set to -x in target.mk to show build process

ifeq ($(VERBOSE),)
GOBUILD_VERBOSE ?= -v 
endif

BUILDTAGS ?= seccomp

GOBUILD_FLAGS += $(GOBUILD_VERBOSE) -compiler gccgo

ifeq ($(GOBUILD_REBUILD_ALL),yes)
	GOBUILD_FLAGS += -a
endif

ifeq ($(filter-out %.go,x$(SRC_GO_PKG)),x$(SRC_GO_PKG))
	GO_BUILD_LIB ?= yes
endif

ifneq (x$(GO_BUILD_LIB),xyes)
	# assume set of .go files to be compiled
	TARGET ?= $(lastword $(subst /, ,$(PRG_DIR)))
	PKG    ?= $(TARGET)

	SRC_GO_PK := $(foreach s,$(SRC_GO_PKG),$(realpath $(PRG_DIR)/$s))
	SRC_GO_PKG_MAIN ?= $(SRC_GO_PKG)
	SRC_GO_BUILD_DIR = .

else
	# assume package name wihout .go substng
	# TARGET ?= lib$(lastword $(subst /, ,$(SRC_GO_PKG))).a
	TARGET ?= $(lastword $(subst /, ,$(PRG_DIR)))
	PKG    ?= $(SRC_GO_PKG)

	SRC_GO_PK := $(SRC_GO_PKG)
	SRC_GO_PKG_MAIN ?= $(SRC_GO_PKG)
	SRC_GO_BUILD_DIR = src/$(SRC_GO_PKG_MAIN)

$(PKG):
ifeq ($(VERBOSE),)
		echo Building $@
endif
		$(VERBOSE)mkdir -p src
		$(VERBOSE)source ./env.sh && GO111MODULE=off GOPATH=$(CURDIR) genode-go get -x -v -d $(SRC_GO_PKG_MAIN)
endif

STRIP_TARGET_CMD ?= $(STRIP) -o $(CURDIR)/$@ $(CURDIR)/$<

$(TARGET).stripped: $(TARGET)
		$(VERBOSE)$(STRIP_TARGET_CMD)

$(INSTALL_DIR)/$(TARGET): $(TARGET).stripped
		$(VERBOSE)ln -sf $(CURDIR)/$< $@

ifneq ($(DEBUG_DIR),)
$(DEBUG_DIR)/$(TARGET): $(TARGET)
		$(VERBOSE)ln -sf $(CURDIR)/$< $@
endif

GOROOT:=$(BUILD_BASE_DIR)/app/goroot

GOOS:=genode

ifeq ($(findstring x86_64,$(SPECS)),x86_64)
	GOARCH:=amd64
else ifeq ($(findstring arm_64,$(SPECS)),arm_64)
	GOARCH:=arm64
else
	$(error unsipported arch SPECS $(SPECS))
endif

# substitute $libs variable in generated LD_CMD and remove call to gcc++
CGO_LDFLAGS0 = $(subst $$libs,$(LIB_CACHE_DIR),$(filter-out $(firstword $(LD_CMD)) ,$(LD_CMD)))
CGO_LDFLAGS1 = $(subst =ld.lib.so,=ld_lib_so,$(CGO_LDFLAGS0))
CGO_LDFLAGS2 = $(patsubst %.lib.so, -l:%.lib.so,$(CGO_LDFLAGS1))
CGO_LDFLAGS  = $(subst =ld_lib_so,=ld.lib.so,$(CGO_LDFLAGS2))

MY_CUSTOM_GO = $(notdir $(CUSTOM_GO))

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
	$(VERBOSE)echo "export GOOS='$(GOOS)'" >> $@
	$(VERBOSE)echo "export GOARCH='$(GOARCH)'" >> $@
	$(VERBOSE)echo "export GCCGO='$(MY_CUSTOM_GO)'" >> $@
	$(VERBOSE)echo "export CGO_LDFLAGS='$(CGO_LDFLAGS) -L$(realpath .)'" >> $@
	$(VERBOSE)echo "export GOROOT='$(GOROOT)'" >> $@
	$(VERBOSE)echo "export GOPATH='$(LIBGO_PKG_BUILD):$(CURDIR)'" >> $@
	$(VERBOSE)echo "export PATH=$(GOROOT)/bin:${PATH}" >> $@
	$(VERBOSE)echo "export GCC='$(GCC)'" >> $@
	$(VERBOSE)echo "export GOC='$(GOC)'" >> $@
	$(VERBOSE)echo "export READELF='$(READELF)'" >> $@
	$(VERBOSE)echo "export AS='$(AS)'" >> $@
	$(VERBOSE)echo "export OBJCOPY='$(OBJCOPY)'" >> $@
	$(VERBOSE)echo "export OBJDUMP='$(OBJDUMP)'" >> $@
	$(VERBOSE)echo "export GDB='$(GDB)'" >> $@
	$(VERBOSE)echo "export GCC_EXEC_PREFIX='$(GOROOT)/toolchain/gcc/libgo/'" >> $@
	$(VERBOSE)echo "unset GOPROXY" >> $@


$(TARGET): env.sh $(SRC_GO_PK) $(SHARED_LIBS) $(GOROOT)/bin
	$(VERBOSE)for files in $(SRC_GO_PK); do \
		if [ -f $${files} ]; then \
			ln -sf $${files} .; \
		fi \
	done
	$(VERBOSE)if [ ! -f go.mod ]; then \
		source ./env.sh && genode-go mod init $(PKG); \
	fi
	$(VERBOSE)source ./env.sh && genode-go mod tidy -e
	$(VERBOSE)source ./env.sh && cd $(SRC_GO_BUILD_DIR) && \
		genode-go build $(GOBUILD_FLAGS) -o $(CURDIR)/$@ $(SRC_GO_PKG_MAIN)

LIBS   = base libc libm libgo libgo_support stdcxx
