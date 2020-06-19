include $(call select_from_repositories,src/app/qt5/tmpl/target_defaults.inc)

include $(call select_from_repositories,src/app/qt5/tmpl/target_final.inc)

LIBS += qt5_qgenodeviewwidget qoost base

REP_INC_DIR += include/qt5/qpa_genode

CC_CXX_WARN_STRICT =
