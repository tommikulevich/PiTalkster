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

EXT_LIB_CFLAGS_EXTRA = \
	-Ilib/st7789 \
	-Ilib/st7789/interface \
	-Ilib/st7789/interface/inc