FALKON_DIR = $(call select_from_ports,falkon)/src/app/falkon

QMAKE_PROJECT_FILE = $(FALKON_DIR)/falkon.pro

QMAKE_TARGET_BINARIES = falkon libFalkon.lib.so

QT5_PORT_LIBS += libQt5Core libQt5Gui libQt5Network libQt5PrintSupport libQt5Sql libQt5Widgets
QT5_PORT_LIBS += libQt5Qml libQt5QmlModels libQt5Quick libQt5QuickWidgets
QT5_PORT_LIBS += libQt5Svg
QT5_PORT_LIBS += libQt5WebChannel
QT5_PORT_LIBS += libQt5WebEngine libQt5WebEngineCore libQt5WebEngineWidgets

LIBS = qt5_qmake libc libm mesa qt5_component stdcxx nss3 libcrypto

QT5_GENODE_LIBS_APP += libcrypto.lib.so nss3.lib.so

qmake_prepared.tag: $(addprefix build_dependencies/lib/,$(QT5_GENODE_LIBS_APP))

$(BUILD_BASE_DIR)/bin/falkon_theme.tar:
	$(VERBOSE)tar cf $@ --transform='s/themes/.local\/share\/falkon\/themes/' -C $(FALKON_DIR) themes/chrome

QT5_EXTRA_TARGET_DEPS = $(BUILD_BASE_DIR)/bin/falkon_theme.tar

BUILD_ARTIFACTS += falkon_theme.tar
