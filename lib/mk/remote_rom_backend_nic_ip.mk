# \author Johannes Schlatow
# \date   2016-03-19

SRC_CC += backend/nic_ip/client.cc backend/nic_ip/server.cc
INC_DIR += $(REP_DIR)/src/lib/remote_rom/backend/nic_ip

LIBS   += base net

# include less specificuration
include $(REP_DIR)/lib/mk/remote_rom_backend.inc
