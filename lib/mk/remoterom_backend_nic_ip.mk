# \author Johannes Schlatow
# \date   2016-03-19

SRC_CC += backend/nic_ip/backend.cc 

LIBS   += base config net 

# include less specific configuration
include $(REP_DIR)/lib/mk/remoterom_backend.inc
