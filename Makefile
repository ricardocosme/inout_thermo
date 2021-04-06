all: thermo.elf

%.elf: %.cpp
	avr-g++ -std=c++20 -Os -mmcu=attiny85 -DF_CPU=1000000 -Wall -o $@ $< \
	-I../att85/include \
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
	avrdude -p t85 -c usbasp -P usb -U flash:w:$<

clean:
	rm -f *.elf *.s
