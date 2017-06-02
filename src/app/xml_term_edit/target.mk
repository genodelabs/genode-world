TARGET = xml_term_edit
SRC_CC = component.cc
LIBS   = base vfs

INC_DIR += $(PRG_DIR) $(call select_from_repositories,src/app/cli_monitor)
