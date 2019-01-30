TARGET   = test-aunit

SRC_ADB  = \
	aunit_harness.adb \
	aunit_suite.adb \
	aunit-test_cases-tests.adb \
	aunit-test_cases-tests_fixtures.adb \
	aunit-test_cases-tests-suite.adb \
	aunit-test_fixtures-tests.adb \
	aunit-test_fixtures-tests_fixtures.adb \
	aunit-test_fixtures-tests-suite.adb \
	aunit-test_suites-tests.adb \
	aunit-test_suites-tests_fixtures.adb \
	aunit-test_suites-tests-suite.adb

SRC_CC   = startup.cc
LIBS     = libc ada libaunit

AUNIT_DIR = $(call select_from_ports,aunit)/aunit-gpl-2018-src

INC_DIR += $(AUNIT_DIR)/test/src $(PRG_DIR)
vpath %.adb $(AUNIT_DIR)/test/src
