# \author Johannes Schlatow
# \date   2016-03-19

SRC_CC += backend/nic_ip/backend.cc 

LIBS   += base net

# include less specificuration
include $(REP_DIR)/lib/mk/remote_rom_backend.inc
