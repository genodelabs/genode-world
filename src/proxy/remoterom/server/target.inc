SRC_CC = main.cc 
TARGET = remoterom_server

LIBS   += base server config 

INC_DIR += $(REP_DIR)/include/remoterom

vpath main.cc $(REP_DIR)/src/util/remoterom/server
