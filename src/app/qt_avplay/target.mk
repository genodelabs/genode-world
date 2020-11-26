QMAKE_PROJECT_FILE = $(REP_DIR)/src/app/qt_avplay/qt_avplay.pro

QMAKE_TARGET_BINARIES = qt_avplay

QT5_PORT_LIBS = libQt5Core libQt5Gui libQt5PrintSupport libQt5Widgets libQt5Xml libqgenode

LIBS = base libc libm mesa qt5_component stdcxx qoost libqgenodeviewwidget egl $(QT5_PORT_LIBS)

include $(call select_from_repositories,lib/import/import-qt5_qmake.mk)

QT5_GENODE_LIBS_APP += ld.lib.so libqgenodeviewwidget.lib.so libqgenode.lib.so egl.lib.so
QT5_GENODE_LIBS_APP := $(filter-out qt5_component.lib.so,$(QT5_GENODE_LIBS_APP))

qmake_prepared.tag: qmake_root/lib/ld.lib.so \
                    qmake_root/lib/libqgenodeviewwidget.lib.so \
                    qmake_root/lib/libqgenode.lib.so
