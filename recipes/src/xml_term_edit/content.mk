SRC_DIR := src/app/xml_term_edit

content: $(SRC_DIR) LICENSE

$(SRC_DIR):
	mkdir -p $@
	cp -r $(REP_DIR)/$@/* $@/
	cp \
		$(GENODE_DIR)/repos/os/src/app/cli_monitor/command_line.h \
		$(GENODE_DIR)/repos/os/src/app/cli_monitor/line_editor.h \
		$@/

LICENSE:
	cp $(GENODE_DIR)/LICENSE $@
