TARGET  = tox_dht_bootstrap
LIBS   += c-toxcore libc
SRC_CC += component.cc

TOX_PORT := $(call select_from_ports,c-toxcore)

INC_DIR += $(TOX_PORT)/src/lib/toxcore/other
