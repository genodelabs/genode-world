MORPH_BROWSER_PORT_DIR = $(call select_from_ports,morph-browser)/src/app/morph-browser

CMAKE_LISTS_DIR = $(MORPH_BROWSER_PORT_DIR)

CMAKE_TARGET_BINARIES = \
	install/usr/local/bin/morph-browser \
	install/usr/local/lib/qt5/qml/Morph/Web/libmorph-web-plugin.lib.so

QT5_PORT_LIBS = libQt5Core libQt5Gui libQt5Widgets libQt5Network libQt5Sql
QT5_PORT_LIBS += libQt5Qml libQt5QmlModels libQt5Quick
QT5_PORT_LIBS += libQt5WebEngineCore libQt5WebEngine libQt5WebChannel

LIBS = qt5_cmake libc libm mesa qt5_component stdcxx nss3

cmake_prepared.tag: build_dependencies/lib/nss3.lib.so

$(BUILD_BASE_DIR)/bin/morph-browser_qml.tar: build_with_cmake
	ln -snf qt5 install/usr/local/lib/qt
	$(VERBOSE)tar cf $@ --exclude='*.lib.so' --transform='s/\.stripped//' -C install/usr/local/lib qt/qml
	$(VERBOSE)tar rf $@ -C install usr/local/share/morph-browser

QT5_EXTRA_TARGET_DEPS = $(BUILD_BASE_DIR)/bin/morph-browser_qml.tar

BUILD_ARTIFACTS += morph-browser_qml.tar
