include includes_source.mk

PROJECT_NAME = piTalkster

TARGET ?= rpi
BUILD_DIR = build/$(TARGET)
SRC_DIR = src
EXT_LIB_DIR = lib

CC = gcc

CFLAGS = -std=c17 \
		 -Wall -Wextra -Werror -Wpedantic -Wconversion -Wshadow \
         -Wformat=2 -Wstrict-aliasing=2 -Wnull-dereference -Wstack-usage=6144 \
         -D_FORTIFY_SOURCE=2 -fstack-protector-strong \
         -O2 -g3 -DRPI_TARGET -D_DEFAULT_SOURCE -D_GNU_SOURCE \
         -I$(SRC_DIR) -I$(EXT_LIB_DIR) $(CFLAGS_EXTRA) $(EXT_LIB_CFLAGS_EXTRA)
EXT_LIB_CFLAGS = -std=c17 \
		 -O2 -g3 -D_DEFAULT_SOURCE -D_GNU_SOURCE \
		 -I$(EXT_LIB_DIR) $(EXT_LIB_CFLAGS_EXTRA)
LDFLAGS = -lm $(LDFLAGS_EXTRA) 

SRCS = $(shell find $(SRC_DIR) -name '*.c' ! -name '*_test.c')
LIB_SRCS = $(shell find $(EXT_LIB_DIR) -name '*.c')

OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
LIB_OBJS = $(patsubst $(EXT_LIB_DIR)/%.c,$(BUILD_DIR)/lib/%.o,$(LIB_SRCS))

DEPS = $(OBJS:.o=.d) $(LIB_OBJS:.o=.d)

.PHONY: all clean test run

all: $(BUILD_DIR)/$(PROJECT_NAME)

$(BUILD_DIR)/$(PROJECT_NAME): $(OBJS) $(LIB_OBJS)
	@echo "Building $(PROJECT_NAME)."
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "All is done."

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/lib/%.o: $(EXT_LIB_DIR)/%.c
	@echo "Compiling external library $<"
	@mkdir -p $(@D)
	@$(CC) $(EXT_LIB_CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

test: CFLAGS += --coverage -O0
test: LDFLAGS += -lcmocka -lgcov
test:
	$(MAKE) -C tests TARGET=$(TARGET)

clean:
	@echo "Cleaning."
	@rm -rf $(BUILD_DIR)
	@find . -name '*.gc*' -delete

run: $(BUILD_DIR)/$(PROJECT_NAME)
	@echo "Running."
	@./$(BUILD_DIR)/$(PROJECT_NAME)
	