TARGET = iperf

IPERF_DIR := $(call select_from_ports,iperf)/src/app/iperf

LIBS += libc libm stdcxx posix

SRC_CC_COMPAT   = $(IPERF_DIR)/compat/Thread.c
SRC_CC_COMPAT  += $(IPERF_DIR)/compat/error.c
SRC_CC_COMPAT  += $(IPERF_DIR)/compat/delay.c
SRC_CC_COMPAT  += $(IPERF_DIR)/compat/gettimeofday.c
SRC_CC_COMPAT  += $(IPERF_DIR)/compat/inet_ntop.c
SRC_CC_COMPAT  += $(IPERF_DIR)/compat/inet_pton.c
SRC_CC_COMPAT  += $(IPERF_DIR)/compat/signal.c
SRC_CC_COMPAT  += $(IPERF_DIR)/compat/snprintf.c
SRC_CC_COMPAT  += $(IPERF_DIR)/compat/string.c

SRC_CC_IPERF    = $(IPERF_DIR)/src/Client.cpp
SRC_CC_IPERF   += $(IPERF_DIR)/src/Extractor.c
SRC_CC_IPERF   += $(IPERF_DIR)/src/isochronous.cpp
SRC_CC_IPERF   += $(IPERF_DIR)/src/Launch.cpp
SRC_CC_IPERF   += $(IPERF_DIR)/src/List.cpp
SRC_CC_IPERF   += $(IPERF_DIR)/src/Listener.cpp
SRC_CC_IPERF   += $(IPERF_DIR)/src/Locale.c
SRC_CC_IPERF   += $(IPERF_DIR)/src/PerfSocket.cpp
SRC_CC_IPERF   += $(IPERF_DIR)/src/ReportCSV.c
SRC_CC_IPERF   += $(IPERF_DIR)/src/ReportDefault.c
SRC_CC_IPERF   += $(IPERF_DIR)/src/Reporter.c
SRC_CC_IPERF   += $(IPERF_DIR)/src/Server.cpp
SRC_CC_IPERF   += $(IPERF_DIR)/src/Settings.cpp
SRC_CC_IPERF   += $(IPERF_DIR)/src/SocketAddr.c
SRC_CC_IPERF   += $(IPERF_DIR)/src/gnu_getopt.c
SRC_CC_IPERF   += $(IPERF_DIR)/src/gnu_getopt_long.c
SRC_CC_IPERF   += $(IPERF_DIR)/src/histogram.c
SRC_CC_IPERF   += $(IPERF_DIR)/src/main.cpp
SRC_CC_IPERF   += $(IPERF_DIR)/src/sockets.c
SRC_CC_IPERF   += $(IPERF_DIR)/src/stdio.c
SRC_CC_IPERF   += $(IPERF_DIR)/src/tcp_window_size.c
SRC_CC_IPERF   += $(IPERF_DIR)/src/pdfs.c

SRC_CC = $(SRC_CC_COMPAT) $(SRC_CC_IPERF)

INC_DIR += $(PRG_DIR)
INC_DIR += $(IPERF_DIR)/include

CC_OPT  += -DHAVE_CONFIG_H -DGENODE_BUILD

CC_WARN = -Wall -Wno-unused -Wno-maybe-uninitialized -Wno-format-truncation \
          -Wno-stringop-truncation -Wno-stringop-overflow

CC_CXX_WARN_STRICT =

vpath timer.cc $(PRG_DIR)
vpath %.c      $(IPERF_DIR)/compat
vpath %.c      $(IPERF_DIR)/src