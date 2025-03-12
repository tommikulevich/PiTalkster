LDFLAGS_EXTRA = -pthread -lgpiod -lasound -lvosk -lcjson -lcurl
CFLAGS_EXTRA = \
	-Isrc/utils \
	-Isrc/event_broker \
	-Isrc/core \
	-Isrc/controls \
	-Isrc/display \
	-Isrc/audio_input \
	-Isrc/speech_to_text \
	-Isrc/llm