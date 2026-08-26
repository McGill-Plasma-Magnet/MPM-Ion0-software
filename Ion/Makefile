CC := gcc

CFLAGS := -Wall -Wextra -Wpedantic -std=c11 -Ilib
LDFLAGS :=
LDLIBS := -lm

SRC_DIR := src
LIB_DIR := lib
EXAMPLE_DIR := examples
BUILD_DIR := build
BIN_DIR := bin

# --------------------------------------------------
# Library source files
# --------------------------------------------------

LIB_SRCS := \
	$(LIB_DIR)/hdc2080.c \
	$(LIB_DIR)/logs.c \
	$(LIB_DIR)/motors.c \
	$(LIB_DIR)/ms5607.c \
	$(LIB_DIR)/stepper.c \
	$(LIB_DIR)/thermistor.c

LIB_OBJS := $(patsubst $(LIB_DIR)/%.c,$(BUILD_DIR)/%.o,$(LIB_SRCS))

# --------------------------------------------------
# Main programs
# --------------------------------------------------

FLIGHT_SRCS := \
	$(SRC_DIR)/flightmain.c \
	$(SRC_DIR)/batteryheater.c

LOGS_SRCS := \
	$(SRC_DIR)/logsmain.c

FLIGHT_OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(FLIGHT_SRCS))
LOGS_OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(LOGS_SRCS))

# --------------------------------------------------
# Examples
# --------------------------------------------------

EXAMPLE_SRCS := $(wildcard $(EXAMPLE_DIR)/*.c)

EXAMPLE_BINS := $(patsubst \
	$(EXAMPLE_DIR)/%.c,\
	$(BIN_DIR)/%,\
	$(EXAMPLE_SRCS))

# --------------------------------------------------
# Default
# --------------------------------------------------

.PHONY: all
all: directories \
	$(BIN_DIR)/flightmain \
	$(BIN_DIR)/logsmain \
	examples

# --------------------------------------------------
# Main executables
# --------------------------------------------------

$(BIN_DIR)/flightmain: $(FLIGHT_OBJS) $(LIB_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BIN_DIR)/logsmain: $(LOGS_OBJS) $(LIB_OBJS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# --------------------------------------------------
# Library compilation
# --------------------------------------------------

$(BUILD_DIR)/%.o: $(LIB_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# --------------------------------------------------
# src/ compilation
# --------------------------------------------------

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# --------------------------------------------------
# Examples
# Each example gets linked against all library objects
# --------------------------------------------------

.PHONY: examples
examples: directories $(EXAMPLE_BINS)

$(BIN_DIR)/%: $(EXAMPLE_DIR)/%.c $(LIB_OBJS)
	$(CC) $(CFLAGS) $< $(LIB_OBJS) $(LDFLAGS) $(LDLIBS) -o $@

# --------------------------------------------------
# Directories
# --------------------------------------------------

.PHONY: directories
directories:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BIN_DIR)

# --------------------------------------------------
# Cleaning
# --------------------------------------------------

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(BIN_DIR)/*

.PHONY: rebuild
rebuild: clean all
