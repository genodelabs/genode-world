SYNERGY_SRC_DIR = $(call select_from_ports,synergy_micro)/src/lib/synergy_micro

TARGET   = synergy_input
INC_DIR += $(SYNERGY_SRC_DIR)
SRC_C    = uSynergy.c
SRC_CC   = main.cc
LIBS     = libc libc_lwip_nic_dhcp

vpath uSynergy.c $(SYNERGY_SRC_DIR)
vpath main.cc    $(PRG_DIR)
