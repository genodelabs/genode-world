TARGET = test-glucose
LIBS   = posix stdcxx zlib glucose
SRC_CC = main.cc

CC_OPT += -D__STDC_LIMIT_MACROS -D__STDC_FORMAT_MACROS

CC_CXX_WARN_STRICT =
