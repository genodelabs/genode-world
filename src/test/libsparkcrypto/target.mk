TARGET = test-libsparkcrypto

SRC_ADB  = \
	lsc_internal_benchmark.adb \
	lsc_internal_suite.adb \
	lsc_internal_test_aes.adb \
	lsc_internal_test_aes_cbc.adb \
	lsc_internal_test_bignum.adb \
	lsc_internal_test_ec.adb \
	lsc_internal_test_hmac.adb \
	lsc_internal_test_ripemd160.adb \
	lsc_internal_test_sha1.adb \
	lsc_internal_test_sha2.adb \
	lsc_internal_test_shadow.adb \
	lsc_suite.adb \
	lsc_test_aes.adb \
	lsc_test_aes_cbc.adb \
	lsc_test_hmac_ripemd160.adb \
	lsc_test_hmac_sha1.adb \
	lsc_test_hmac_sha2.adb \
	lsc_test_ripemd160.adb \
	lsc_test_sha1.adb \
	lsc_test_sha2.adb \
	main.adb \
	openssl.adb \
	tests.adb \
	util.adb \
	util_tests.adb

SRC_CC = startup.cc
SRC_C  = libglue.c
OPT_C = -DDEBUG

LIBS = ada libaunit libsparkcryptofat libcrypto libc

LSC_TESTS_DIR := $(call select_from_ports,libsparkcrypto)/libsparkcrypto/tests

INC_DIR += $(LSC_TESTS_DIR)
CUSTOM_ADA_OPT += -gnatec=$(call select_from_ports,libsparkcrypto)/libsparkcrypto/build/pragmas.adc

vpath lsc_internal_benchmark.adb $(PRG_DIR)
vpath %.adb $(LSC_TESTS_DIR)
vpath %.ads $(LSC_TESTS_DIR)
vpath %.c   $(LSC_TESTS_DIR)
