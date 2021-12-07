content: src/app/iperf LICENSE

IPERF_CONTRIB_DIR := $(call port_dir,$(REP_DIR)/ports/iperf)/src/app/iperf

src/app/iperf:
	mkdir -p $@
	cp -r $(IPERF_CONTRIB_DIR)/* $@
	$(mirror_from_rep_dir)

LICENSE:
	cp $(IPERF_CONTRIB_DIR)/COPYING $@

