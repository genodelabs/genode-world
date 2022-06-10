TARGET = chronyd

CHRONY_DIR := $(call select_from_ports,chrony)/src/app/chronyd

LIBS += base libc libm stdcxx posix nettle gnutls gmp

SRC_C  = addrfilt.c
SRC_C += array.c
SRC_C += clientlog.c
SRC_C += cmac_nettle.c
SRC_C += cmdmon.c
SRC_C += cmdparse.c
SRC_C += conf.c
SRC_C += hash_nettle.c
SRC_C += hwclock.c
SRC_C += keys.c
SRC_C += local.c
SRC_C += logging.c
SRC_C += main.c
SRC_C += manual.c
SRC_C += memory.c
SRC_C += nameserv.c
SRC_C += nameserv_async.c
SRC_C += ntp_auth.c
SRC_C += ntp_core.c
SRC_C += ntp_ext.c
SRC_C += ntp_io.c
SRC_C += ntp_sources.c
SRC_C += nts_ke_client.c
SRC_C += nts_ke_server.c
SRC_C += nts_ke_session.c
SRC_C += nts_ntp_auth.c
SRC_C += nts_ntp_client.c
SRC_C += nts_ntp_server.c
SRC_C += pktlength.c
SRC_C += reference.c
SRC_C += regress.c
SRC_C += rtc.c
SRC_C += samplefilt.c
SRC_C += sched.c
SRC_C += siv_nettle.c
SRC_C += smooth.c
SRC_C += socket.c
SRC_C += sources.c
SRC_C += sourcestats.c
SRC_C += stubs.c
SRC_C += sys.c
SRC_C += sys_null.c
SRC_C += sys_posix.c
SRC_C += sys_timex.c
SRC_C += tempcomp.c
SRC_C += util.c

SRC_C += genode_stubs.c
SRC_C += set_time_helper_c.c
SRC_CC += set_time_helper.cc

INC_DIR += $(CHRONY_DIR)
INC_DIR += $(CHRONY_DIR)/include
INC_DIR += $(REP_DIR)/src/app/chrony

CC_OPT  += -DHAVE_CONFIG_H -DGENODE_BUILD

CC_WARN = -Wall -Wno-unused -Wno-maybe-uninitialized -Wno-format-truncation \
          -Wno-stringop-truncation -Wno-stringop-overflow

CC_CXX_WARN_STRICT =

vpath genode_stubs.c      $(REP_DIR)/src/app/chrony
vpath set_time_helper_c.c $(REP_DIR)/src/app/chrony
vpath set_time_helper.cc  $(REP_DIR)/src/app/chrony
vpath %.c                 $(CHRONY_DIR)
