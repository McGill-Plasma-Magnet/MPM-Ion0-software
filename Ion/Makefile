CC := gcc

CFLAGS := -Wall -Wextra -Wpedantic -std=c11 -Ilib
LDFLAGS :=
LDLIBS := -lm -lgpiod

SRC_DIR := src
LIB_DIR := lib
EXAMPLE_DIR := examples
BUILD_DIR := build
BIN_DIR := bin

LIB_SRCS := \
	$(LIB_DIR)/hdc2080.c \
	$(LIB_DIR)/logs.c \
	$(LIB_DIR)/motors.c \
	$(LIB_DIR)/ms5607.c \
	$(LIB_DIR)/stepper.c \
	$(LIB_DIR)/thermistor.c

LIB_OBJS := $(patsubst $(LIB_DIR)/%.c,$(BUILD_DIR)/%.o,$(LIB_SRCS))

FLIGHT_OBJ := $(BUILD_DIR)/flightmain.o
BATTERY_OBJ := $(BUILD_DIR)/batteryheater.o
LOGS_OBJ := $(BUILD_DIR)/logsmain.o

EXAMPLE_SRCS := $(wildcard $(EXAMPLE_DIR)/*.c)
EXAMPLE_BINS := $(patsubst $(EXAMPLE_DIR)/%.c,$(BIN_DIR)/%,$(EXAMPLE_SRCS))

.PHONY: all clean rebuild examples directories

all: directories \
	$(BIN_DIR)/flightmain \
	$(BIN_DIR)/batteryheater \
	$(BIN_DIR)/logsmain \
	examples

$(BIN_DIR)/flightmain: $(FLIGHT_OBJ) $(LIB_OBJS)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(BIN_DIR)/batteryheater: $(BATTERY_OBJ) $(LIB_OBJS)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(BIN_DIR)/logsmain: $(LOGS_OBJ) $(LIB_OBJS)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(LIB_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

examples: directories $(EXAMPLE_BINS)

$(BIN_DIR)/%: $(EXAMPLE_DIR)/%.c $(LIB_OBJS)
	$(CC) $(CFLAGS) $< $(LIB_OBJS) $(LDFLAGS) $(LDLIBS) -o $@

directories:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(BIN_DIR)/*

rebuild: clean all
