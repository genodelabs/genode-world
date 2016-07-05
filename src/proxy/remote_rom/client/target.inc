SRC_CC = main.cc 
TARGET = remote_rom_client

LIBS   += base config

INC_DIR += $(REP_DIR)/include/remote_rom

vpath main.cc $(REP_DIR)/src/proxy/remote_rom/client
