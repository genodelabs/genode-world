TARGET = ubuntu_ui_toolkit.qmake_target

PORT_DIR := $(call select_from_ports,ubuntu-ui-toolkit)/src/lib/ubuntu-ui-toolkit

QT5_PORT_LIBS += libQt5Core libQt5Gui libQt5Network libQt5Test libQt5Widgets
QT5_PORT_LIBS += libQt5Qml libQt5QmlModels libQt5Quick
QT5_PORT_LIBS += libQt5Svg

GENODE_QMAKE_CFLAGS += -Wno-deprecated -Wno-deprecated-declarations -Wno-deprecated-copy

LIBS = qt5_qmake libc libm mesa stdcxx

INSTALL_LIBS = lib/libUbuntuGestures.lib.so \
               lib/libUbuntuMetrics.lib.so \
               lib/libUbuntuToolkit.lib.so \
               qml/Ubuntu/Components/libUbuntuComponents.lib.so \
               qml/Ubuntu/Components/Labs/libUbuntuComponentsLabs.lib.so \
               qml/Ubuntu/Components/Styles/libUbuntuComponentsStyles.lib.so \
               qml/Ubuntu/PerformanceMetrics/libUbuntuPerformanceMetrics.lib.so

BUILD_ARTIFACTS = $(notdir $(INSTALL_LIBS)) \
                  ubuntu-ui-toolkit_qml.tar

build: qmake_prepared.tag qt5_so_files

	@#
	@# run qmake
	@#

	$(VERBOSE)source env.sh && $(QMAKE) \
		-qtconf build_dependencies/mkspecs/$(QT_PLATFORM)/qt.conf \
		$(PORT_DIR)/ubuntu-sdk.pro \
		$(QT5_OUTPUT_FILTER)

	@#
	@# build
	@#

	$(VERBOSE)source env.sh && $(MAKE) sub-src $(QT5_OUTPUT_FILTER)

	@#
	@# install into local 'install' directory
	@#

	$(VERBOSE)$(MAKE) INSTALL_ROOT=$(CURDIR)/install sub-src-install_subtargets $(QT5_OUTPUT_FILTER)

	$(VERBOSE)ln -sf .$(CURDIR)/build_dependencies install/qt

	@#
	@# strip libs and create symlinks in 'bin' and 'debug' directories
	@#

	for LIB in $(INSTALL_LIBS); do \
		cd $(CURDIR)/install/qt/$$(dirname $${LIB}) && \
			$(OBJCOPY) --only-keep-debug $$(basename $${LIB}) $$(basename $${LIB}).debug && \
			$(STRIP) $$(basename $${LIB}) -o $$(basename $${LIB}).stripped && \
			$(OBJCOPY) --add-gnu-debuglink=$$(basename $${LIB}).debug $$(basename $${LIB}).stripped; \
		ln -sf $(CURDIR)/install/qt/$${LIB}.stripped $(PWD)/bin/$$(basename $${LIB}); \
		ln -sf $(CURDIR)/install/qt/$${LIB}.stripped $(PWD)/debug/$$(basename $${LIB}); \
		ln -sf $(CURDIR)/install/qt/$${LIB}.debug $(PWD)/debug/; \
	done

	@#
	@# create tar archives
	@#

	$(VERBOSE)tar chf $(PWD)/bin/ubuntu-ui-toolkit_qml.tar --exclude='*.lib.so' --transform='s/\.stripped//' -C install qt/qml

.PHONY: build

QT5_TARGET_DEPS = build
