include $(REP_DIR)/lib/import/import-vncclient.mk

LIBS += libc zlib jpeg

INC_DIR += $(LIBVNCCLIENT_PORT_DIR)/src/lib/vnc/common

SRC_C := cursor.c \
         listen.c \
         rfbproto.c \
         sockets.c \
         vncviewer.c \
         minilzo.c \
         tls_none.c

SHARED_LIB = yes

vpath minilzo.c $(LIBVNCCLIENT_PORT_DIR)/src/lib/vnc/common
vpath %.c $(LIBVNCCLIENT_PORT_DIR)/src/lib/vnc/libvncclient
