MCU_TARGET=attiny85
STD_CXX=c++20
OPTIMIZE=-Os

CXX=avr-g++
CC=avr-gcc
OBJCOPY=avr-objcopy
OBJDUMP=avr-objdump
INCLUDE=-I../att85/include -I../ds18b20/include -I../avrcxx/include
CXXFLAGS=-std=$(STD_CXX) -g -mmcu=$(MCU_TARGET) -Wall $(OPTIMIZE) -DF_CPU=1000000 $(INCLUDE)

demos = thermo

all: $(demos:%=%.elf) $(demos:%=%.lst) $(demos:%=%.s)

%.d: %.cpp
	@set -e; rm -f $@; \
	$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

include $(demos:%=%.d)

%.s: %.cpp
	$(CXX) $(CXXFLAGS) -S $^

%.elf: %.o
	$(CXX) $(CXXFLAGS) -o $@ $^ ../avrcxx/src/avrcxx/watchdog/counter.cpp

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

.PHONY: flash_%
flash-%: %.hex
	avrdude -p t85 -c usbasp -P usb  -U flash:w:$<

size:
	avr-size $(demos:%=%.elf)

.PHONY: clean
clean:
	rm -f *.hex *.lst *.elf *.o *.d *.s

