TESTS_LDFLAGS_EXTRA = -lcmocka -lgcov
TESTS_CFLAGS_EXTRA = \
	-Isrc/utils \
	-Isrc/event_broker
TESTS_REQUIRED_SRCS := \
    src/event_broker/event.c