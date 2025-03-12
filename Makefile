# ===========================
# === PiTalkster Makefile ===
# ===========================

include mk/includes_src.mk
include mk/includes_lib.mk
include mk/includes_tests.mk

PROJECT_NAME := piTalkster

TARGET ?= rpi

BUILD_DIR := build/$(TARGET)
SRC_DIR := src
TESTS_DIR := tests
LIB_DIR := lib

CC := gcc

SRCS := $(shell find $(SRC_DIR) -type f -name '*.c')
LIB_SRCS := $(shell find $(LIB_DIR) -type f -name '*.c')
TESTS_SRCS := $(shell find $(TESTS_DIR) -type f -name 'test_*.c')

OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
LIB_OBJS := $(LIB_SRCS:$(LIB_DIR)/%.c=$(BUILD_DIR)/$(LIB_DIR)/%.o)
TESTS_REQUIRED_OBJS := $(TESTS_REQUIRED_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/$(TESTS_DIR)/%.o)
TESTS_BINS := $(TESTS_SRCS:$(TESTS_DIR)/%.c=$(BUILD_DIR)/$(TESTS_DIR)/%)

DEPS := $(OBJS:.o=.d) $(LIB_OBJS:.o=.d) $(TESTS_OBJS:.o=.d)

-include $(DEPS)

CFLAGS := -std=c17 -Wall -Wextra -Werror -Wpedantic -Wconversion -Wshadow \
	-Wformat=2 -Wstrict-aliasing=2 -Wnull-dereference -Wstack-usage=6144 \
	-D_FORTIFY_SOURCE=2 -fstack-protector-strong -O2 -g3 \
	-D_DEFAULT_SOURCE -D_GNU_SOURCE \
	-I$(SRC_DIR) $(CFLAGS_EXTRA) -I$(LIB_DIR) $(LIB_CFLAGS_EXTRA)
LIB_CFLAGS = -std=c17 \
	-O2 -g3 -D_DEFAULT_SOURCE -D_GNU_SOURCE \
	-I$(LIB_DIR) $(LIB_CFLAGS_EXTRA)
TESTS_CFLAGS := -std=c17 -Wall -Wextra -Werror -Wpedantic -Wshadow \
	-Wformat=2 -Wnull-dereference -O0 -g3 --coverage -DUNIT_TESTS \
	-I$(TESTS_DIR) $(TESTS_CFLAGS_EXTRA)

LDFLAGS := $(LDFLAGS_EXTRA) $(LIB_LDFLAGS_EXTRA)
TESTS_LDFLAGS := $(TESTS_LDFLAGS_EXTRA) 

.PHONY: all clean test run

all: $(BUILD_DIR)/$(PROJECT_NAME)

$(BUILD_DIR)/$(PROJECT_NAME): $(OBJS) $(LIB_OBJS)
	@echo "Linking $(PROJECT_NAME)"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Build complete."

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/$(LIB_DIR)/%.o: $(LIB_DIR)/%.c
	@echo "Compiling external library: $<"
	@mkdir -p $(@D)
	@$(CC) $(LIB_CFLAGS) -MMD -MP -c $< -o $@ 

$(BUILD_DIR)/$(TESTS_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling (for tests) $<"
	@mkdir -p $(@D)
	@$(CC) $(TESTS_CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/$(TESTS_DIR)/%.o: $(TESTS_DIR)/%.c
	@echo "Compiling test: $<"
	@mkdir -p $(@D)
	@$(CC) $(TESTS_CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/$(TESTS_DIR)/%: $(BUILD_DIR)/$(TESTS_DIR)/%.o $(TESTS_REQUIRED_OBJS)
	@echo "Linking test binary: $@"
	@mkdir -p $(@D)
	@$(CC) $(TESTS_CFLAGS) $^ -o $@ $(TESTS_LDFLAGS)

test: $(TESTS_BINS)
	@for test in $(TESTS_BINS); do \
		echo "\nRunning $$test:"; \
		./$$test || exit 1; \
	done

run: all
	@echo "Running $(PROJECT_NAME)"
	@./$(BUILD_DIR)/$(PROJECT_NAME)

clean:
	@rm -rf $(BUILD_DIR)
	@find . -type f -name '*.gcda' -delete
	@find . -type f -name '*.gcno' -delete
	@find . -type f -name '*.gcov' -delete
	@echo "Clean complete."
