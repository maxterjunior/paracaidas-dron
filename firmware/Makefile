# Firmware paracaídas M400 — PIC18F25K22, MPLAB XC8 v2.x / v3.x
#
#   make            compila build/paracaidas.hex
#   make BENCH=1    con comandos de banco por UART (ARM / DISARM / FIRE) — NO volar con esto
#   make test       pruebas de la lógica de detección en la PC (gcc o clang)
#   make clean

XC8    ?= xc8-cc
CC     ?= gcc
MCU    := 18F25K22
BUILD  := build

SRC := src/main.c src/board.c src/config_bits.c src/detector.c \
       lib/hal/tick.c lib/hal/i2c.c lib/hal/uart.c lib/hal/adc.c \
       lib/bmi088/bmi088.c
INC := -Isrc -Ilib/hal -Ilib/bmi088

XC8FLAGS := -mcpu=$(MCU) -O2 -std=c99 -Wall $(INC)
ifdef BENCH
XC8FLAGS += -DBENCH_COMMANDS=1
endif

.PHONY: all test clean

all: $(BUILD)/paracaidas.hex

$(BUILD)/paracaidas.hex: $(SRC) $(wildcard src/*.h lib/*/*.h)
	@mkdir -p $(BUILD)
	$(XC8) $(XC8FLAGS) $(SRC) -o $(BUILD)/paracaidas.elf
	@echo "-> $(BUILD)/paracaidas.hex"

test:
	@mkdir -p $(BUILD)
	$(CC) -std=c99 -Wall -Wextra -O2 -Isrc test/test_detector.c src/detector.c -lm -o $(BUILD)/test_detector
	./$(BUILD)/test_detector

clean:
	rm -rf $(BUILD)
