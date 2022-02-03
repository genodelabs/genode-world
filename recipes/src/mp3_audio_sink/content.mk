SRC_DIR = src/app/mp3_audio_sink
include $(GENODE_DIR)/repos/base/recipes/src/content.inc

content: src/app/raw_audio_sink/magic_ring_buffer.h

src/app/raw_audio_sink/magic_ring_buffer.h:
	mkdir -p $(dir $@)
	cp $(REP_DIR)/src/app/raw_audio_sink/magic_ring_buffer.h $@
