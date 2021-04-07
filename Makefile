mcu=attiny85
dev=t85

all: thermo.elf

%.elf: %.cpp
	avr-g++ -std=c++20 -Os -mmcu=$(mcu) -Wall -o $@ $< \
	-I../ssd1306/include \
	-I../ds18b20/include \
	-I../avrSLEEP/include \
	-I../avrIO/include \
	-I../avrWDT/include \
	-I../avrINT/include
	avr-size $@

%.s: %.elf
	avr-objdump -d $< > $@
	cat $@

.PHONY: clean flash

flash: thermo.elf
	avrdude -p $(dev) -c usbasp -P usb -U flash:w:$<

clean:
	rm -f *.elf *.s
