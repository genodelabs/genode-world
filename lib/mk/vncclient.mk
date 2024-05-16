include $(REP_DIR)/lib/import/import-vncclient.mk

LIBS += libc zlib jpeg

INC_DIR += $(LIBVNCCLIENT_PORT_DIR)/src/lib/vnc/common

SRC_C := libvncclient/cursor.c \
         libvncclient/listen.c \
         libvncclient/rfbproto.c \
         libvncclient/sockets.c \
         common/sockets.c \
         libvncclient/vncviewer.c \
         common/minilzo.c \
         common/turbojpeg.c \
         common/crypto_included.c \
         common/d3des.c \
         libvncclient/tls_none.c

SHARED_LIB = yes

vpath %.c $(LIBVNCCLIENT_PORT_DIR)/src/lib/vnc
