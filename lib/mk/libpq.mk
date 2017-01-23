include $(REP_DIR)/lib/import/import-libpq.mk

LIBPQ_PORT_DIR := $(call select_from_ports,pgsql)
LIBPQ_SRC_DIR  := $(LIBPQ_PORT_DIR)/src/lib/postgres

CC_OPT += -DFRONTEND -DUNSAFE_STAT_OK

# files interfaces libpq
SRC_C += fe-auth.c\
	fe-connect.c\
	fe-exec.c\
	fe-misc.c\
	fe-print.c\
	fe-lobj.c\
	fe-protocol2.c\
	fe-protocol3.c\
	pqexpbuffer.c\
	fe-secure.c\
	libpq-events.c\
	chklocale.c\
	encnames.c\
	getpeereid.c\
	inet_net_ntop.c\
	ip.c\
	md5.c\
	noblock.c\
	pgstrcasecmp.c\
	pqsignal.c\
	strlcpy.c\
	thread.c\
	wchar.c\

#dummy files for sigwait
SRC_C += dummy_sig_method.c

#definition of vpath	  
vpath % $(LIBPQ_SRC_DIR)/src/interfaces/libpq
vpath % $(LIBPQ_SRC_DIR)/src/port
vpath % $(REP_DIR)/src/lib/libpq
vpath % $(LIBPQ_SRC_DIR)/src/backend/utils/mb
vpath % $(LIBPQ_SRC_DIR)/src/backend/libpq


LIBS += libc pthread

SHARED_LIB = yes

