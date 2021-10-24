TARGET = iperf

IPERF_DIR := $(call select_from_ports,iperf)/src/app/iperf

LIBS += libc libm stdcxx posix

SRC_CC_COMPAT   = compat/Thread.c
SRC_CC_COMPAT  += compat/error.c
SRC_CC_COMPAT  += compat/delay.c
SRC_CC_COMPAT  += compat/gettimeofday.c
SRC_CC_COMPAT  += compat/inet_ntop.c
SRC_CC_COMPAT  += compat/inet_pton.c
SRC_CC_COMPAT  += compat/signal.c
SRC_CC_COMPAT  += compat/snprintf.c
SRC_CC_COMPAT  += compat/string.c

SRC_CC_IPERF    = src/Client.cpp
SRC_CC_IPERF   += src/Extractor.c
SRC_CC_IPERF   += src/isochronous.cpp
SRC_CC_IPERF   += src/Launch.cpp
SRC_CC_IPERF   += src/List.cpp
SRC_CC_IPERF   += src/Listener.cpp
SRC_CC_IPERF   += src/Locale.c
SRC_CC_IPERF   += src/PerfSocket.cpp
SRC_CC_IPERF   += src/ReportCSV.c
SRC_CC_IPERF   += src/ReportDefault.c
SRC_CC_IPERF   += src/Reporter.c
SRC_CC_IPERF   += src/Server.cpp
SRC_CC_IPERF   += src/Settings.cpp
SRC_CC_IPERF   += src/SocketAddr.c
SRC_CC_IPERF   += src/gnu_getopt.c
SRC_CC_IPERF   += src/gnu_getopt_long.c
SRC_CC_IPERF   += src/histogram.c
SRC_CC_IPERF   += src/main.cpp
SRC_CC_IPERF   += src/sockets.c
SRC_CC_IPERF   += src/stdio.c
SRC_CC_IPERF   += src/tcp_window_size.c
SRC_CC_IPERF   += src/pdfs.c

SRC_CC = $(SRC_CC_COMPAT) $(SRC_CC_IPERF)

INC_DIR += $(PRG_DIR)
INC_DIR += $(IPERF_DIR)/include

CC_OPT  += -DHAVE_CONFIG_H -DGENODE_BUILD

CC_WARN = -Wall -Wno-unused -Wno-maybe-uninitialized -Wno-format-truncation \
          -Wno-stringop-truncation -Wno-stringop-overflow

CC_CXX_WARN_STRICT =

vpath timer.cc $(PRG_DIR)
vpath %.cpp    $(IPERF_DIR)
vpath %.c      $(IPERF_DIR)
