########## V6 BLE Geiger TX (ATtiny2313 + nRF24L01+) ##########

TARGET = geiger_ble
MCU    = attiny2313
F_CPU  = 1000000UL
LFUSE  = 0x64
HFUSE  = 0xDF
EFUSE  = 0xFF

PROGRAMMER_TYPE = usbtiny
PROGRAMMER_ARGS =

CC      = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AVRSIZE = avr-size
AVRDUDE = avrdude

SOURCES = main.c
OBJECTS = $(SOURCES:.c=.o)

CFLAGS  = -Os -g -std=gnu99 -Wall -Wextra
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -ffunction-sections -fdata-sections
CPPFLAGS = -DF_CPU=$(F_CPU) -I.
LDFLAGS  = -Wl,--gc-sections -Wl,-Map,$(TARGET).map
TARGET_ARCH = -mmcu=$(MCU)

.PHONY: all size clean flash fuses flashUsbtiny flashIce

all: $(TARGET).hex size

%.o: %.c Makefile
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<

$(TARGET).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ -o $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

size: $(TARGET).elf
	$(AVRSIZE) --format=avr --mcu=$(MCU) $(TARGET).elf

clean:
	rm -f $(TARGET).elf $(TARGET).hex $(TARGET).map $(OBJECTS)

flash: $(TARGET).hex
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) -U flash:w:$<

flashUsbtiny: PROGRAMMER_TYPE = usbtiny
flashUsbtiny: flash

flashIce: PROGRAMMER_TYPE = atmelice_isp
flashIce: flash

fuses:
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) \
	           -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(EFUSE):m
