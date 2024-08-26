QMAKE_PROJECT_FILE = $(REP_DIR)/src/app/qt_avplay/qt_avplay.pro

QMAKE_TARGET_BINARIES = qt_avplay

QT5_PORT_LIBS = libQt5Core libQt5Gui libQt5PrintSupport libQt5Widgets libQt5Xml libqgenode

LIBS = qt5_qmake base libc libm mesa qt5_component stdcxx qoost libqgenodeviewwidget egl

QT5_COMPONENT_LIB_SO =

QT5_GENODE_LIBS_APP += ld.lib.so libqgenodeviewwidget.lib.so libqgenode.lib.so egl.lib.so

qmake_prepared.tag: $(addprefix build_dependencies/lib/,$(QT5_GENODE_LIBS_APP))
