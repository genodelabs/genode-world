TARGET   = audio_player
SRC_CC   = main.cc
INC_DIR += $(PRG_DIR)
LIBS     := base libc avcodec avformat avutil avresample pthread
