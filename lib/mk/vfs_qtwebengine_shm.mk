SRC_CC = vfs.cc

VFS_DIR = $(REP_DIR)/src/lib/vfs/qtwebengine_shm
INC_DIR += $(VFS_DIR)
vpath %.cc $(VFS_DIR)

LD_OPT  += --version-script=$(VFS_DIR)/symbol.map

SHARED_LIB = yes
