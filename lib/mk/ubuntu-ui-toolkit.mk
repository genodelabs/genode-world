include $(call select_from_repositories,lib/import/import-qt5_qmake.mk)

PORT_DIR := $(call select_from_ports,ubuntu-ui-toolkit)/src/lib/ubuntu-ui-toolkit

QT5_PORT_LIBS += libQt5Core libQt5Gui libQt5Network libQt5Test libQt5Widgets
QT5_PORT_LIBS += libQt5Qml libQt5QmlModels libQt5Quick
QT5_PORT_LIBS += libQt5Svg

GENODE_QMAKE_CFLAGS += -Wno-deprecated -Wno-deprecated-declarations -Wno-deprecated-copy

LIBS = libc libm mesa stdcxx $(QT5_PORT_LIBS)

INSTALL_LIBS = lib/libUbuntuGestures.lib.so \
               lib/libUbuntuMetrics.lib.so \
               lib/libUbuntuToolkit.lib.so \
               qml/Ubuntu/Components/libUbuntuComponents.lib.so \
               qml/Ubuntu/Components/Labs/libUbuntuComponentsLabs.lib.so \
               qml/Ubuntu/Components/Styles/libUbuntuComponentsStyles.lib.so \
               qml/Ubuntu/PerformanceMetrics/libUbuntuPerformanceMetrics.lib.so

BUILD_ARTIFACTS = $(notdir $(INSTALL_LIBS)) \
                  ubuntu-ui-toolkit_qml.tar

built.tag: qmake_prepared.tag

	@#
	@# run qmake
	@#

	$(VERBOSE)source env.sh && $(QMAKE) \
		-qtconf qmake_root/mkspecs/$(QMAKE_PLATFORM)/qt.conf \
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

	$(VERBOSE)ln -sf .$(CURDIR)/qmake_root install/qt

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

	@#
	@# mark as done
	@#

	$(VERBOSE)touch $@


ifeq ($(called_from_lib_mk),yes)
all: built.tag
endif
