TARGET  = tor
LIBS    = libevent posix libssl libcrypto zlib pthread
LIBS   += libc_lwip_nic_dhcp
TOR_DIR = $(call select_from_ports,tor)/src/app/tor
INC_DIR = $(PRG_DIR) \
          $(TOR_DIR) \
          $(TOR_DIR)/src \
          $(TOR_DIR)/src/common \
          $(TOR_DIR)/src/ext \
          $(TOR_DIR)/src/ext/trunnel \
          $(TOR_DIR)/src/or \
          $(TOR_DIR)/src/trunnel
CC_OPT += -std=c99 -DOPENSSL_THREADS -DSHARE_DATADIR="\"/share\"" -DLOCALSTATEDIR="\"/tor\"" -DED25519_SUFFIX=_donna

SRC_C   = common/address.c \
          common/aes.c \
          common/backtrace.c \
          common/compat.c \
          common/compat_libevent.c \
          common/compat_pthreads.c \
          common/compat_threads.c \
          common/container.c \
          common/crypto.c \
          common/crypto_curve25519.c \
          common/crypto_ed25519.c \
          common/crypto_format.c \
          common/crypto_pwbox.c \
          common/crypto_s2k.c \
          common/di_ops.c \
          common/log.c \
          common/memarea.c \
          common/procmon.c \
          common/pubsub.c \
          common/sandbox.c \
          common/timers.c \
          common/torgzip.c \
          common/tortls.c \
          common/util.c \
          common/util_bug.c \
          common/util_format.c \
          common/util_process.c \
          common/workqueue.c \
          ext/csiphash.c \
          ext/ed25519/donna/ed25519_tor.c \
          ext/ed25519/ref10/blinding.c \
          ext/ed25519/ref10/fe_0.c \
          ext/ed25519/ref10/fe_1.c \
          ext/ed25519/ref10/fe_add.c \
          ext/ed25519/ref10/fe_cmov.c \
          ext/ed25519/ref10/fe_copy.c \
          ext/ed25519/ref10/fe_frombytes.c \
          ext/ed25519/ref10/fe_invert.c \
          ext/ed25519/ref10/fe_isnegative.c \
          ext/ed25519/ref10/fe_isnonzero.c \
          ext/ed25519/ref10/fe_mul.c \
          ext/ed25519/ref10/fe_neg.c \
          ext/ed25519/ref10/fe_pow22523.c \
          ext/ed25519/ref10/fe_sq.c \
          ext/ed25519/ref10/fe_sq2.c \
          ext/ed25519/ref10/fe_sub.c \
          ext/ed25519/ref10/fe_tobytes.c \
          ext/ed25519/ref10/ge_add.c \
          ext/ed25519/ref10/ge_double_scalarmult.c \
          ext/ed25519/ref10/ge_frombytes.c \
          ext/ed25519/ref10/ge_madd.c \
          ext/ed25519/ref10/ge_msub.c \
          ext/ed25519/ref10/ge_p1p1_to_p2.c \
          ext/ed25519/ref10/ge_p1p1_to_p3.c \
          ext/ed25519/ref10/ge_p2_0.c \
          ext/ed25519/ref10/ge_p2_dbl.c \
          ext/ed25519/ref10/ge_p3_0.c \
          ext/ed25519/ref10/ge_p3_dbl.c \
          ext/ed25519/ref10/ge_p3_to_cached.c \
          ext/ed25519/ref10/ge_p3_to_p2.c \
          ext/ed25519/ref10/ge_p3_tobytes.c \
          ext/ed25519/ref10/ge_precomp_0.c \
          ext/ed25519/ref10/ge_scalarmult_base.c \
          ext/ed25519/ref10/ge_sub.c \
          ext/ed25519/ref10/ge_tobytes.c \
          ext/ed25519/ref10/keyconv.c \
          ext/ed25519/ref10/keypair.c \
          ext/ed25519/ref10/open.c \
          ext/ed25519/ref10/sc_muladd.c \
          ext/ed25519/ref10/sc_reduce.c \
          ext/ed25519/ref10/sign.c \
          ext/keccak-tiny/keccak-tiny-unrolled.c \
          ext/readpassphrase.c \
          ext/timeouts/timeout.c \
          ext/trunnel/trunnel.c \
          or/addressmap.c \
          or/buffers.c \
          or/channel.c \
          or/channeltls.c \
          or/circpathbias.c \
          or/circuitbuild.c \
          or/circuitlist.c \
          or/circuitmux.c \
          or/circuitmux_ewma.c \
          or/circuitstats.c \
          or/circuituse.c \
          or/command.c \
          or/config.c \
          or/confparse.c \
          or/connection.c \
          or/connection_edge.c \
          or/connection_or.c \
          or/control.c \
          or/cpuworker.c \
          or/dircollate.c \
          or/directory.c \
          or/dirserv.c \
          or/dirvote.c \
          or/dns.c \
          or/dnsserv.c \
          or/entrynodes.c \
          or/ext_orport.c \
          or/fp_pair.c \
          or/geoip.c \
          or/hibernate.c \
          or/keypin.c \
          or/main.c \
          or/microdesc.c \
          or/networkstatus.c \
          or/nodelist.c \
          or/onion.c \
          or/onion_fast.c \
          or/onion_ntor.c \
          or/onion_tap.c \
          or/periodic.c \
          or/policies.c \
          or/reasons.c \
          or/relay.c \
          or/rendcache.c \
          or/rendclient.c \
          or/rendcommon.c \
          or/rendmid.c \
          or/rendservice.c \
          or/rephist.c \
          or/replaycache.c \
          or/router.c \
          or/routerkeys.c \
          or/routerlist.c \
          or/routerparse.c \
          or/routerset.c \
          or/scheduler.c \
          or/statefile.c \
          or/status.c \
          or/tor_main.c \
          or/torcert.c \
          or/transports.c \
          trunnel/ed25519_cert.c \
          trunnel/link_handshake.c \
          trunnel/pwbox.c

ifeq ($(filter-out $(SPECS),64bit),)
SRC_C +=  ext/curve25519_donna/curve25519-donna-c64.c
else
SRC_C +=  ext/curve25519_donna/curve25519-donna.c
endif # 64bit

vpath %.c $(TOR_DIR)/src
