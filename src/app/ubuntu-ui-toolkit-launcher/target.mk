include $(call select_from_repositories,lib/import/import-ubuntu-ui-toolkit.mk)

ifeq ($(CONTRIB_DIR),)
QMAKE_PROJECT_FILE = $(PRG_DIR)/ubuntu-ui-toolkit-launcher.pro
else
QMAKE_PROJECT_FILE = $(UBUNTU_UI_TOOLKIT_PORT_DIR)/ubuntu-ui-toolkit-launcher/ubuntu-ui-toolkit-launcher.pro
endif

QMAKE_TARGET_BINARIES = ubuntu-ui-toolkit-launcher

QT5_PORT_LIBS = libQt5Core libQt5Gui libQt5Network libQt5Svg libQt5Test libQt5Widgets
QT5_PORT_LIBS += libQt5Qml libQt5Quick

LIBS = libc libm mesa qt5_component stdcxx $(QT5_PORT_LIBS)
LIBS += libUbuntuGestures libUbuntuMetrics libUbuntuToolkit

include $(call select_from_repositories,lib/import/import-qt5_qmake.mk)
