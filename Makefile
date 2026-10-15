MCU = atmega328p
F_CPU = 16000000UL

CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size

TARGET = build/main
SRC = src/main.c

CFLAGS = -mmcu=$(MCU) \
         -DF_CPU=$(F_CPU) \
         -std=c11 \
         -Os \
         -Wall \
         -Wextra \
         -Werror

all: $(TARGET).hex

build:
	mkdir -p build

$(TARGET).elf: $(SRC) | build
	$(CC) $(CFLAGS) $(SRC) -o $@
	$(SIZE) $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

clean:
	rm -rf build

.PHONY: all clean