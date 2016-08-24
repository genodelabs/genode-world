LIBEVENT_DIR := $(call select_from_ports,libevent)/src/lib/libevent

SRC_C      = buffer.c \
             bufferevent.c \
             bufferevent_filter.c \
             bufferevent_openssl.c \
             bufferevent_pair.c \
             bufferevent_ratelim.c \
             bufferevent_sock.c \
             evdns.c \
             event.c \
             event_tagging.c \
             evmap.c \
             evrpc.c \
             evthread.c \
             evutil.c \
             evutil_rand.c \
             http.c \
             listener.c \
             log.c \
             poll.c \
             select.c \
             signal.c \
             strlcpy.c

INC_DIR   += $(LIBEVENT_DIR) $(LIBEVENT_DIR)/include
INC_DIR   += $(LIBEVENT_DIR)/include
INC_DIR   += $(REP_DIR)/include/libevent
LIBS      += libc libssl libcrypto
SHARED_LIB = yes

vpath %.c $(LIBEVENT_DIR)

