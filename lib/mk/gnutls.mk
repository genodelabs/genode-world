GNUTLS_DIR := $(call select_from_ports,gnutls)/src/lib/gnutls

LIBS += libc gmp nettle

SRC_C_LIB += lib/alert.c
SRC_C_LIB += lib/atfork.c
SRC_C_LIB += lib/auth.c
SRC_C_LIB += lib/auto-verify.c
SRC_C_LIB += lib/buffers.c
SRC_C_LIB += lib/cipher.c
SRC_C_LIB += lib/cipher-cbc.c
SRC_C_LIB += lib/cipher_int.c
SRC_C_LIB += lib/cert-cred.c
SRC_C_LIB += lib/cert-cred-x509.c
SRC_C_LIB += lib/cert-session.c
SRC_C_LIB += lib/constate.c
SRC_C_LIB += lib/crypto-api.c
SRC_C_LIB += lib/crypto-backend.c
SRC_C_LIB += lib/datum.c
SRC_C_LIB += lib/db.c
SRC_C_LIB += lib/debug.c
SRC_C_LIB += lib/dh.c
SRC_C_LIB += lib/dh-primes.c
SRC_C_LIB += lib/dh-session.c
SRC_C_LIB += lib/dtls.c
SRC_C_LIB += lib/dtls-sw.c
SRC_C_LIB += lib/ecc.c
SRC_C_LIB += lib/errors.c
SRC_C_LIB += lib/extv.c
SRC_C_LIB += lib/fips.c
SRC_C_LIB += lib/file.c
SRC_C_LIB += lib/fingerprint.c
SRC_C_LIB += lib/global.c
SRC_C_LIB += lib/gnutls_asn1_tab.c
SRC_C_LIB += lib/handshake.c
SRC_C_LIB += lib/handshake-checks.c
SRC_C_LIB += lib/handshake-tls13.c
SRC_C_LIB += lib/hash_int.c
SRC_C_LIB += lib/hello_ext.c
SRC_C_LIB += lib/hello_ext_lib.c
SRC_C_LIB += lib/iov.c
SRC_C_LIB += lib/kx.c
SRC_C_LIB += lib/mbuffers.c
SRC_C_LIB += lib/mem.c
SRC_C_LIB += lib/mpi.c
SRC_C_LIB += lib/ocsp-api.c
SRC_C_LIB += lib/pcert.c
SRC_C_LIB += lib/pin.c
SRC_C_LIB += lib/pk.c
SRC_C_LIB += lib/pkix_asn1_tab.c
SRC_C_LIB += lib/prf.c
SRC_C_LIB += lib/priority.c
SRC_C_LIB += lib/privkey.c
SRC_C_LIB += lib/profiles.c
SRC_C_LIB += lib/pubkey.c
SRC_C_LIB += lib/random.c
SRC_C_LIB += lib/record.c
SRC_C_LIB += lib/safe-memfuncs.c
SRC_C_LIB += lib/secrets.c
SRC_C_LIB += lib/session.c
SRC_C_LIB += lib/session_pack.c
SRC_C_LIB += lib/srp.c
SRC_C_LIB += lib/sslv2_compat.c
SRC_C_LIB += lib/state.c
SRC_C_LIB += lib/stek.c
SRC_C_LIB += lib/str.c
SRC_C_LIB += lib/str-iconv.c
SRC_C_LIB += lib/str-idna.c
SRC_C_LIB += lib/str-unicode.c
SRC_C_LIB += lib/supplemental.c
SRC_C_LIB += lib/system.c
SRC_C_LIB += lib/system_override.c
SRC_C_LIB += lib/tls-sig.c
SRC_C_LIB += lib/tls13-sig.c
SRC_C_LIB += lib/urls.c
SRC_C_LIB += lib/x509_b64.c

SRC_C_LIB_ACCELERATED += lib/accelerated/accelerated.c
SRC_C_LIB_ACCELERATED += lib/accelerated/cryptodev.c

SRC_C_LIB_ALGORITHMS += lib/algorithms/cert_types.c
SRC_C_LIB_ALGORITHMS += lib/algorithms/ciphersuites.c
SRC_C_LIB_ALGORITHMS += lib/algorithms/ecc.c
SRC_C_LIB_ALGORITHMS += lib/algorithms/groups.c
SRC_C_LIB_ALGORITHMS += lib/algorithms/mac.c
SRC_C_LIB_ALGORITHMS += lib/algorithms/ciphers.c
SRC_C_LIB_ALGORITHMS += lib/algorithms/kx.c
SRC_C_LIB_ALGORITHMS += lib/algorithms/protocols.c
SRC_C_LIB_ALGORITHMS += lib/algorithms/publickey.c
SRC_C_LIB_ALGORITHMS += lib/algorithms/secparams.c
SRC_C_LIB_ALGORITHMS += lib/algorithms/sign.c

SRC_C_LIB_AUTH += lib/auth/anon.c
SRC_C_LIB_AUTH += lib/auth/anon_ecdh.c
SRC_C_LIB_AUTH += lib/auth/cert.c
SRC_C_LIB_AUTH += lib/auth/dh_common.c
SRC_C_LIB_AUTH += lib/auth/dhe.c
SRC_C_LIB_AUTH += lib/auth/dhe_psk.c
SRC_C_LIB_AUTH += lib/auth/ecdhe.c
SRC_C_LIB_AUTH += lib/auth/psk.c
SRC_C_LIB_AUTH += lib/auth/psk_passwd.c
SRC_C_LIB_AUTH += lib/auth/rsa.c
SRC_C_LIB_AUTH += lib/auth/rsa_psk.c
SRC_C_LIB_AUTH += lib/auth/srp_kx.c
SRC_C_LIB_AUTH += lib/auth/srp_passwd.c
SRC_C_LIB_AUTH += lib/auth/srp_rsa.c
SRC_C_LIB_AUTH += lib/auth/srp_sb64.c
SRC_C_LIB_AUTH += lib/auth/vko_gost.c

SRC_C_LIB_EXT += lib/ext/alpn.c
SRC_C_LIB_EXT += lib/ext/client_cert_type.c
SRC_C_LIB_EXT += lib/ext/cookie.c
SRC_C_LIB_EXT += lib/ext/dumbfw.c
SRC_C_LIB_EXT += lib/ext/early_data.c
SRC_C_LIB_EXT += lib/ext/ec_point_formats.c
SRC_C_LIB_EXT += lib/ext/etm.c
SRC_C_LIB_EXT += lib/ext/ext_master_secret.c
SRC_C_LIB_EXT += lib/ext/heartbeat.c
SRC_C_LIB_EXT += lib/ext/key_share.c
SRC_C_LIB_EXT += lib/ext/max_record.c
SRC_C_LIB_EXT += lib/ext/post_handshake.c
SRC_C_LIB_EXT += lib/ext/pre_shared_key.c
SRC_C_LIB_EXT += lib/ext/psk_ke_modes.c
SRC_C_LIB_EXT += lib/ext/record_size_limit.c
SRC_C_LIB_EXT += lib/ext/safe_renegotiation.c
SRC_C_LIB_EXT += lib/ext/server_cert_type.c
SRC_C_LIB_EXT += lib/ext/server_name.c
SRC_C_LIB_EXT += lib/ext/session_ticket.c
SRC_C_LIB_EXT += lib/ext/signature.c
SRC_C_LIB_EXT += lib/ext/srp.c
SRC_C_LIB_EXT += lib/ext/srtp.c
SRC_C_LIB_EXT += lib/ext/status_request.c
SRC_C_LIB_EXT += lib/ext/supported_groups.c
SRC_C_LIB_EXT += lib/ext/supported_versions.c

SRC_C_LIB_EXTRAS += lib/extras/hex.c

SRC_C_LIB_INIH += lib/inih/ini.c

SRC_C_LIB_MINITASN1 += lib/minitasn1/coding.c
SRC_C_LIB_MINITASN1 += lib/minitasn1/decoding.c
SRC_C_LIB_MINITASN1 += lib/minitasn1/element.c
SRC_C_LIB_MINITASN1 += lib/minitasn1/gstr.c
SRC_C_LIB_MINITASN1 += lib/minitasn1/parser_aux.c
SRC_C_LIB_MINITASN1 += lib/minitasn1/structure.c
SRC_C_LIB_MINITASN1 += lib/minitasn1/version.c

SRC_C_LIB_NETTLE += lib/nettle/cipher.c
SRC_C_LIB_NETTLE += lib/nettle/init.c
SRC_C_LIB_NETTLE += lib/nettle/mac.c
SRC_C_LIB_NETTLE += lib/nettle/mpi.c
SRC_C_LIB_NETTLE += lib/nettle/pk.c
SRC_C_LIB_NETTLE += lib/nettle/prf.c
SRC_C_LIB_NETTLE += lib/nettle/rnd.c
SRC_C_LIB_NETTLE += lib/nettle/sysrng-getentropy.c
#SRC_C_LIB_NETTLE += lib/nettle/sysrng-netbsd.c
#SRC_C_LIB_NETTLE += lib/nettle/ecc/gmp-glue.c
SRC_C_LIB_NETTLE += lib/nettle/int/dsa-compute-k.c
SRC_C_LIB_NETTLE += lib/nettle/int/dsa-keygen-fips186.c
SRC_C_LIB_NETTLE += lib/nettle/int/dsa-validate.c
SRC_C_LIB_NETTLE += lib/nettle/int/ecdsa-compute-k.c
SRC_C_LIB_NETTLE += lib/nettle/int/mpn-base256.c
SRC_C_LIB_NETTLE += lib/nettle/int/provable-prime.c
SRC_C_LIB_NETTLE += lib/nettle/int/rsa-keygen-fips186.c
SRC_C_LIB_NETTLE += lib/nettle/int/tls1-prf.c

SRC_C_LIB_SYSTEM += lib/system/certs.c
# keys-dummy.c <-> keys-win.c?
SRC_C_LIB_SYSTEM += lib/system/keys-dummy.c
SRC_C_LIB_SYSTEM += lib/system/sockets.c
SRC_C_LIB_SYSTEM += lib/system/threads.c

SRC_C_LIB_TLS13 += lib/tls13/anti_replay.c
SRC_C_LIB_TLS13 += lib/tls13/certificate.c
SRC_C_LIB_TLS13 += lib/tls13/certificate_request.c
SRC_C_LIB_TLS13 += lib/tls13/certificate_verify.c
SRC_C_LIB_TLS13 += lib/tls13/early_data.c
SRC_C_LIB_TLS13 += lib/tls13/encrypted_extensions.c
SRC_C_LIB_TLS13 += lib/tls13/finished.c
SRC_C_LIB_TLS13 += lib/tls13/hello_retry.c
SRC_C_LIB_TLS13 += lib/tls13/key_update.c
SRC_C_LIB_TLS13 += lib/tls13/post_handshake.c
SRC_C_LIB_TLS13 += lib/tls13/psk_ext_parser.c
SRC_C_LIB_TLS13 += lib/tls13/session_ticket.c

SRC_C_LIB_UNISTRING := $(notdir $(wildcard $(GNUTLS_DIR)/lib/unistring/unictype/*.c))
SRC_C_LIB_UNISTRING += lib/unistring/uninorm/decompose-internal.c
SRC_C_LIB_UNISTRING += lib/unistring/uninorm/canonical-decomposition.c
SRC_C_LIB_UNISTRING += lib/unistring/uninorm/compat-decomposition.c
SRC_C_LIB_UNISTRING += lib/unistring/uninorm/composition.c
SRC_C_LIB_UNISTRING += lib/unistring/uninorm/decompose-internal.c
SRC_C_LIB_UNISTRING += lib/unistring/uninorm/decomposition.c
SRC_C_LIB_UNISTRING += lib/unistring/uninorm/decomposition-table.c
SRC_C_LIB_UNISTRING += lib/unistring/uninorm/nfc.c
SRC_C_LIB_UNISTRING += lib/unistring/uninorm/nfd.c
SRC_C_LIB_UNISTRING += lib/unistring/uninorm/nfkc.c
SRC_C_LIB_UNISTRING += lib/unistring/uninorm/nfkd.c
SRC_C_LIB_UNISTRING += lib/unistring/uninorm/u16-normalize.c
SRC_C_LIB_UNISTRING += lib/unistring/uninorm/u32-normalize.c
SRC_C_LIB_UNISTRING += lib/unistring/uninorm/u8-normalize.c
SRC_C_LIB_UNISTRING += lib/unistring/unistr/u16-cpy.c
SRC_C_LIB_UNISTRING += lib/unistring/unistr/u16-mbtoucr.c
SRC_C_LIB_UNISTRING += lib/unistring/unistr/u16-mbtouc-unsafe-aux.c
SRC_C_LIB_UNISTRING += lib/unistring/unistr/u16-mbtouc-unsafe.c
SRC_C_LIB_UNISTRING += lib/unistring/unistr/u16-to-u8.c
SRC_C_LIB_UNISTRING += lib/unistring/unistr/u16-uctomb-aux.c
SRC_C_LIB_UNISTRING += lib/unistring/unistr/u16-uctomb.c
SRC_C_LIB_UNISTRING += lib/unistring/unistr/u32-cpy.c
SRC_C_LIB_UNISTRING += lib/unistring/unistr/u32-mbtouc-unsafe.c
SRC_C_LIB_UNISTRING += lib/unistring/unistr/u32-to-u8.c
SRC_C_LIB_UNISTRING += lib/unistring/unistr/u32-uctomb.c
SRC_C_LIB_UNISTRING += lib/unistring/unistr/u8-check.c
SRC_C_LIB_UNISTRING += lib/unistring/unistr/u8-cpy.c
SRC_C_LIB_UNISTRING += lib/unistring/unistr/u8-mbtoucr.c
SRC_C_LIB_UNISTRING += lib/unistring/unistr/u8-mbtouc-unsafe-aux.c
SRC_C_LIB_UNISTRING += lib/unistring/unistr/u8-mbtouc-unsafe.c
SRC_C_LIB_UNISTRING += lib/unistring/unistr/u8-to-u16.c
SRC_C_LIB_UNISTRING += lib/unistring/unistr/u8-to-u32.c
SRC_C_LIB_UNISTRING += lib/unistring/unistr/u8-uctomb-aux.c
SRC_C_LIB_UNISTRING += lib/unistring/unistr/u8-uctomb.c

SRC_C_LIB_X509 += lib/x509/attributes.c
SRC_C_LIB_X509 += lib/x509/common.c
SRC_C_LIB_X509 += lib/x509/crl.c
SRC_C_LIB_X509 += lib/x509/crq.c
SRC_C_LIB_X509 += lib/x509/dn.c
SRC_C_LIB_X509 += lib/x509/extensions.c
SRC_C_LIB_X509 += lib/x509/hostname-verify.c
SRC_C_LIB_X509 += lib/x509/ip.c
SRC_C_LIB_X509 += lib/x509/key_decode.c
SRC_C_LIB_X509 += lib/x509/key_encode.c
SRC_C_LIB_X509 += lib/x509/krb5.c
SRC_C_LIB_X509 += lib/x509/mpi.c
SRC_C_LIB_X509 += lib/x509/name_constraints.c
SRC_C_LIB_X509 += lib/x509/ocsp.c
SRC_C_LIB_X509 += lib/x509/output.c
SRC_C_LIB_X509 += lib/x509/pkcs12.c
SRC_C_LIB_X509 += lib/x509/pkcs12_bag.c
SRC_C_LIB_X509 += lib/x509/pkcs12_encr.c
SRC_C_LIB_X509 += lib/x509/pkcs7-crypt.c
SRC_C_LIB_X509 += lib/x509/privkey.c
SRC_C_LIB_X509 += lib/x509/privkey_openssl.c
SRC_C_LIB_X509 += lib/x509/privkey_pkcs8.c
SRC_C_LIB_X509 += lib/x509/privkey_pkcs8_pbes1.c
SRC_C_LIB_X509 += lib/x509/prov-seed.c
SRC_C_LIB_X509 += lib/x509/sign.c
SRC_C_LIB_X509 += lib/x509/time.c
SRC_C_LIB_X509 += lib/x509/tls_features.c
SRC_C_LIB_X509 += lib/x509/verify.c
SRC_C_LIB_X509 += lib/x509/email-verify.c
SRC_C_LIB_X509 += lib/x509/verify-high.c
SRC_C_LIB_X509 += lib/x509/verify-high2.c
SRC_C_LIB_X509 += lib/x509/virt-san.c
SRC_C_LIB_X509 += lib/x509/x509.c
SRC_C_LIB_X509 += lib/x509/x509_dn.c
SRC_C_LIB_X509 += lib/x509/x509_ext.c
SRC_C_LIB_X509 += lib/x509/x509_write.c

SRC_C_GL_SYSTEM += gl/c-strcasecmp.c
SRC_C_GL_SYSTEM += gl/c-strncasecmp.c
SRC_C_GL_SYSTEM += gl/explicit_bzero.c
SRC_C_GL_SYSTEM += gl/hash.c
SRC_C_GL_SYSTEM += gl/hash-pjw-bare.c
SRC_C_GL_SYSTEM += gl/read-file.c
SRC_C_GL_SYSTEM += gl/secure_getenv.c
SRC_C_GL_SYSTEM += gl/strverscmp.c

SRC_C = $(SRC_C_LIB) $(SRC_C_LIB_ACCELERATED) $(SRC_C_LIB_ALGORITHMS) $(SRC_C_LIB_AUTH) $(SRC_C_LIB_EXT) $(SRC_C_LIB_EXTRAS) $(SRC_C_LIB_INIH) $(SRC_C_LIB_MINITASN1) $(SRC_C_LIB_NETTLE) $(SRC_C_LIB_SYSTEM) $(SRC_C_LIB_TLS13) $(SRC_C_LIB_UNISTRING) $(SRC_C_LIB_X509) $(SRC_C_GL_SYSTEM)

#vpath %.c $(GNUTLS_DIR)/lib
#vpath %.c $(GNUTLS_DIR)/lib/ext)

#vpath %.c $(GNUTLS_DIR)/lib/nettle)

vpath %.c $(GNUTLS_DIR)
vpath %.c $(GNUTLS_DIR)/lib/unistring/unictype
#vpath %.c $(GNUTLS_DIR)/gl
#vpath %.c $(GNUTLS_DIR)/lib/includes)
# vpath lib/x509/%.c $(GNUTLS_DIR/lib/x509)
# vpath lib/auth/%.c $(GNUTLS_DIR/lib/auth)

# vpath lib/algoritms/%.c $(GNUTLS_DIR/lib/algoritms)
# vpath lib/extras/%.c $(GNUTLS_DIR/lib/extras)
# vpath lib/accelerated/%.c $(GNUTLS_DIR/lib/accelerated)
# vpath lib/accelerated/x86/%.c $(GNUTLS_DIR/lib/accelerated/x86)


#INC_DIR += $(GNUTLS_DIR)/gl
#INC_DIR += $(GNUTLS_DIR)/lib

INC_DIR += $(GNUTLS_DIR)/lib
#INC_DIR += $(GNUTLS_DIR)/lib/system
INC_DIR += $(GNUTLS_DIR)/gl
INC_DIR += $(GNUTLS_DIR)/lib/accelerated
INC_DIR += $(GNUTLS_DIR)/lib/auth
INC_DIR += $(GNUTLS_DIR)/lib/extras
INC_DIR += $(GNUTLS_DIR)/lib/includes
INC_DIR += $(GNUTLS_DIR)/lib/nettle
INC_DIR += $(GNUTLS_DIR)/lib/nettle/int
INC_DIR += $(GNUTLS_DIR)/lib/minitasn1
INC_DIR += $(GNUTLS_DIR)/lib/unistring
INC_DIR += $(GNUTLS_DIR)/lib/x509
INC_DIR += $(REP_DIR)/src/lib/gnutls

# INC_DIR += \
#     $(GNUTLS_DIR) \
# 	$(REP_DIR)/src/lib/gnutls

CC_OPT = -DHAVE_CONFIG_H

SHARED_LIB = yes

#$(info $$SRC_C is [${SRC_C}])
#$(info $$SRC_O is [${SRC_O}])