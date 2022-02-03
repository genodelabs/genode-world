TARGET  = mp3_audio_sink
LIBS   += base libc libm libmpg123
SRC_CC += component.cc
INC_DIR = $(call select_from_repositories,src/app/raw_audio_sink)

CC_CXX_WARN_STRICT =
