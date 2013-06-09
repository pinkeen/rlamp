AVRDUDE_MCU = atmega8
AVRDUDE_PROGRAMMER = usbasp

AVRDUDE = avrdude
AVRDUDEOPTS =   -p $(AVRDUDE_MCU) \
        -c $(AVRDUDE_PROGRAMMER) 


FREQUENCY = 8000000
MCU = atmega8
CC = avr-gcc
CFLAGS = -mmcu=$(MCU) -Os -DF_CPU=$(FREQUENCY)L -Wall
OBJCOPY = avr-objcopy
OBJCOPYFLAGS = -j .text -j .data -O ihex 
PROJNAME = test
OBJECTS = main.o bbpwm.o rc5.o
LD = avr-gcc
LDFLAGS = -mmcu=$(MCU)

all: $(PROJNAME).hex
    avr-size $(PROJNAME).elf

$(PROJNAME).elf: $(OBJECTS) 
    $(LD) $(LDFLAGS) $(OBJECTS) -o $@

$(PROJNAME).hex: $(PROJNAME).elf
    $(OBJCOPY)  $(OBJCOPYFLAGS) $< $@

upload: $(PROJNAME).hex
    $(AVRDUDE) $(AVRDUDEOPTS) -U flash:w:$<

rebuild: clean all

clean:
    $(RM) $(PROJNAME) *.o *.elf *.hex *~

.PHONY: clean rebuild
