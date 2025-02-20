LDFLAGS_EXTRA = -pthread -lgpiod

CFLAGS_EXTRA = \
	-Isrc/utils \
	-Isrc/event_broker \
	-Isrc/core \
	-Isrc/controls \
	-Isrc/display

EXT_LIB_CFLAGS_EXTRA = \
	-Ilib/st7789 \
	-Ilib/st7789/interface \
	-Ilib/st7789/interface/inc