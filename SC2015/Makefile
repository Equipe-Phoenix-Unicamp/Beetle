PORT = /dev/cu.usbmodem00076611
FILE = main

CC_avr = avr-gcc
OBJCPY_avr = avr-objcopy
F_avr = -mmcu=atmega88p -Os

all: $(FILE).o $(FILE).hex
	avrdude -p atmega88pb -P $(PORT) -c avrispv2 -U flash:w:$(FILE).hex
	
%.o: %.c
	$(CC_avr) $(F_avr) $(INCLUDE_avr) $^ -o $@
	
%.hex: %.o
	$(OBJCPY_avr) -O ihex -R .eeprom $^ $@

clean:
	rm *.o *.hex