TARGET = sgdisk

LIBS += libc stdcxx posix libuuid popt ncurses

GPTFDISK_PORT_DIR := $(call select_from_ports,gptfdisk)
GPTFDISK_DIR      := $(GPTFDISK_PORT_DIR)/src/app/gptfdisk

SRC_CC := attributes.cc
SRC_CC += basicmbr.cc
SRC_CC += bsd.cc
SRC_CC += crc32.cc
SRC_CC += diskio-unix.cc
SRC_CC += diskio.cc
SRC_CC += gpt.cc
SRC_CC += gptcl.cc
SRC_CC += gptcurses.cc
SRC_CC += gptpart.cc
SRC_CC += gpttext.cc
SRC_CC += guid.cc
SRC_CC += mbr.cc
SRC_CC += mbrpart.cc
SRC_CC += parttypes.cc
SRC_CC += sgdisk.cc
SRC_CC += support.cc


CC_CXX_OPT += -fpermissive

# too much to cope with...
CC_WARN =

vpath %.cc       $(REP_DIR)/src/app/gptfdisk
vpath %.cc       $(GPTFDISK_DIR)

CC_CXX_WARN_STRICT =
