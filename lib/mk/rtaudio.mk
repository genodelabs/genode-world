include $(REP_DIR)/lib/import/import-rtaudio.mk

LIBS := stdcxx

SRC_CC := RtAudio.cpp

vpath %.cpp $(RTAUDIO_PORT_DIR)/src/lib/rtaudio
