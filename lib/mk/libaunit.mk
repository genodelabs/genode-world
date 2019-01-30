include $(REP_DIR)/lib/import/import-libaunit.mk

SRC_ADS += \
	aunit-memory.ads \
	aunit-memory-utils.ads \
	aunit-options.ads \
	aunit-reporter.ads \
	aunit-tests.ads \
	ada_containers.ads \

SRC_ADB += \
	aunit.adb \
	aunit-assertions.adb \
	aunit-run.adb \
	aunit-simple_test_cases.adb \
	aunit-test_caller.adb \
	aunit-test_cases.adb \
	aunit-test_filters.adb \
	aunit-test_fixtures.adb \
	aunit-test_results.adb \
	aunit-test_suites.adb \
	aunit-reporter-text.adb \
	ada_containers-aunit_lists.adb \
	aunit-time_measure.adb

SOURCES = $(AUNIT_SRC_DIR)/include/aunit

vpath %.adb $(SOURCES)/framework
vpath %.adb $(SOURCES)/framework/staticmemory
vpath %.adb $(SOURCES)/framework/nocalendar
vpath %.adb $(SOURCES)/framework/fullexception
vpath %.adb $(SOURCES)/containers
vpath %.adb $(SOURCES)/reporters

vpath %.ads $(SOURCES)/framework
vpath %.ads $(SOURCES)/framework/staticmemory
vpath %.ads $(SOURCES)/framework/nocalendar
vpath %.ads $(SOURCES)/framework/fullexception
vpath %.ads $(SOURCES)/containers
vpath %.ads $(SOURCES)/reporters

LIBS += ada
SHARED_LIB = yes
