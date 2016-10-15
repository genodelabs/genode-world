RTAUDIO_PORT_DIR := $(call select_from_ports,rtaudio)

CC_OPT += -D__GENODE_AUDIO__

INC_DIR += $(RTAUDIO_PORT_DIR)/include $(RTAUDIO_PORT_DIR)/include/RtAudio
