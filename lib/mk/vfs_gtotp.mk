SRC_CC = vfs.cc
vpath %.cc $(REP_DIR)/src/lib/vfs/gtotp
SHARED_LIB = yes

LIBS += cryptopp stdcxx