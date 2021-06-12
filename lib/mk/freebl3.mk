include $(call select_from_repositories,lib/mk/nss3.inc)

FREEBL_DIR := $(NSS_PORT_DIR)/nss/lib/freebl
LIBS       += libc

INC_DIR += $(FREEBL_DIR)/mpi \
           $(FREEBL_DIR)/verified/kremlin/include \
           $(FREEBL_DIR)/verified/kremlin/kremlib/dist/minimal \
           $(NSS_PORT_DIR)/nss/lib/softoken

SRC_CC += freeblver.c \
          ldvector.c \
          sysrand.c \
          sha_fast.c \
          md2.c \
          md5.c \
          sha512.c \
          cmac.c \
          alghmac.c \
          rawhash.c \
          arcfour.c \
          arcfive.c \
          crypto_primitives.c \
          blake2b.c \
          desblapi.c \
          des.c \
          drbg.c \
          chacha20poly1305.c \
          cts.c \
          ctr.c \
          blinit.c \
          fipsfreebl.c \
          gcm.c \
          hmacct.c \
          rijndael.c \
          aeskeywrap.c \
          camellia.c \
          dh.c \
          ec.c \
          ecdecode.c \
          pqg.c \
          dsa.c \
          rsa.c \
          rsapkcs.c \
          shvfy.c \
          tlsprfalg.c \
          jpake.c \
          mpi/mpprime.c \
          mpi/mpmontg.c \
          mpi/mplogic.c \
          mpi/mpi.c \
          mpi/mp_gf2m.c \
          mpi/mpcpucache.c \
          ecl/ecl.c \
          ecl/ecl_mult.c \
          ecl/ecl_gf.c \
          ecl/ecp_aff.c \
          ecl/ecp_jac.c \
          ecl/ecp_mont.c \
          ecl/ec_naf.c \
          ecl/ecp_jm.c \
          ecl/ecp_256.c \
          ecl/ecp_384.c \
          ecl/ecp_521.c \
          ecl/ecp_256_32.c \
          ecl/ecp_25519.c \
          ecl/ecp_secp384r1.c \
          ecl/ecp_secp521r1.c \
          ecl/curve25519_64.c \
          verified/Hacl_Poly1305_32.c \
          verified/Hacl_Chacha20.c \
          verified/Hacl_Chacha20Poly1305_32.c \
          verified/Hacl_Curve25519_51.c \
          deprecated/seed.c \
          deprecated/alg2268.c

CC_OPT += -DRIJNDAEL_INCLUDE_TABLES
CC_OPT += -DNSS_USE_64
CC_OPT += -DHAVE_INT128_SUPPORT
CC_OPT += -DMP_API_COMPATIBLE

LD_OPT += -Bsymbolic

vpath %.c $(FREEBL_DIR)

SHARED_LIB = yes

CC_CXX_WARN_STRICT =
