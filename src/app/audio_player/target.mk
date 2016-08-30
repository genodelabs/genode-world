TARGET   = audio_player
SRC_CC   = main.cc
INC_DIR += $(PRG_DIR)
LIBS     = libc avcodec avformat avutil avresample pthread
