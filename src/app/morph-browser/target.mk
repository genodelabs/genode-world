MORPH_BROWSER_PORT_DIR = $(call select_from_ports,morph-browser)/src/app/morph-browser

CMAKE_LISTS_DIR = $(MORPH_BROWSER_PORT_DIR)

CMAKE_TARGET_BINARIES = \
	install/usr/local/bin/morph-browser \
	install/usr/local/lib/qt5/qml/Morph/Web/libmorph-web-plugin.lib.so

QT5_PORT_LIBS = libQt5Core libQt5Gui libQt5Widgets libQt5Network libQt5Sql
QT5_PORT_LIBS += libQt5Qml libQt5QmlModels libQt5Quick
QT5_PORT_LIBS += libQt5WebEngineCore libQt5WebEngine libQt5WebChannel

LIBS = libc libm mesa qt5_component stdcxx nss3 $(QT5_PORT_LIBS)

include $(call select_from_repositories,lib/import/import-qt5_cmake.mk)

cmake_prepared.tag: cmake_root/lib/nss3.lib.so

$(BUILD_BASE_DIR)/bin/morph-browser_qml.tar: build_with_cmake
	ln -snf qt5 install/usr/local/lib/qt
	$(VERBOSE)tar cf $@ --exclude='*.lib.so' --transform='s/\.stripped//' -C install/usr/local/lib qt/qml
	$(VERBOSE)tar rf $@ -C install usr/local/share/morph-browser

$(TARGET): $(BUILD_BASE_DIR)/bin/morph-browser_qml.tar
