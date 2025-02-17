include includes_source.mk

PROJECT_NAME = piTalkster

TARGET ?= rpi
BUILD_DIR = build/$(TARGET)
SRC_DIR = src

CC = gcc

CFLAGS = -std=c17 \
		 -Wall -Wextra -Werror -Wpedantic -Wconversion -Wshadow \
         -Wformat=2 -Wstrict-aliasing=2 -Wnull-dereference -Wstack-usage=6144 \
         -D_FORTIFY_SOURCE=2 -fstack-protector-strong \
		 -O2 -g3 -DRPI_TARGET -D_DEFAULT_SOURCE -D_GNU_SOURCE \
		 -I$(SRC_DIR) $(CFLAGS_EXTRA) \
		 
LDFLAGS = -lm $(LDFLAGS_EXTRA)

SRCS = $(shell find $(SRC_DIR) -name '*.c' ! -name '*_test.c')
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

.PHONY: all clean test run

all: $(BUILD_DIR)/$(PROJECT_NAME)

$(BUILD_DIR)/$(PROJECT_NAME): $(OBJS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

test: CFLAGS += --coverage -O0
test: LDFLAGS += -lcmocka -lgcov
test:
	$(MAKE) -C tests TARGET=$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
	find . -name '*.gc*' -delete

run: $(BUILD_DIR)/$(PROJECT_NAME)
	./$(BUILD_DIR)/$(PROJECT_NAME)
