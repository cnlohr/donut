all : program.hex burn program.lst
CC = avr-gcc

PROCESSOR=atmega168pb
PROGRAMCODE=m168
CFLAGS = -Os -mmcu=$(PROCESSOR) -DF_CPU=8000000UL -I. -Iusbdrv
ASFLAGS = $(CFLAGS) -x assembler-with-cpp
SRCS :=  main.c util.c 
REQS := calced_speeds.c samples.c

samples.c : sampleenc
	cp ./mag.raw sample0.raw
	cp ./fest.raw sample1.raw
	./sampleenc > samples.c

sampleenc : sampleenc.c
	gcc -o $@ $^

speedcalc : speedcalc.c
	gcc -o $@ $^ -lm

calced_speeds.c : speedcalc
	./speedcalc > $@

program.elf :$(SRCS) $(REQS)
	avr-gcc -I -mmcu=$(PROCESSOR) $(CFLAGS) -Wl,-Map,program.map -o $@ $(SRCS) -L /usr/lib64/binutils/avr/2.19.1

program.hex : program.elf
	avr-objcopy -j .text -j .data -O ihex program.elf program.hex 

program.lst : $(SRCS) $(REQS)
	avr-gcc -c -g -Wa,-a,-ad $(CFLAGS) $(SRCS) > $@

burn : program.hex
	sudo avrdude -c usbtiny -p $(PROGRAMCODE) -U flash:w:program.hex -F -B 4


burnall : burn burnfuses

readfuses :
	sudo avrdude -c usbtiny -p $(PROGRAMCODE) -U hfuse:r:high.txt:b -U lfuse:r:low.txt:b

#careful here...
#burnfuses :
#	sudo avrdude -c usbtiny -p $(PROGRAMCODE) -U lfuse:w:0xE6:m -U hfuse:w:0xDF:m

burnfuses :
	sudo avrdude -c usbtiny -p $(PROGRAMCODE) -U lfuse:w:0xFF:m -U hfuse:w:0xD6:m -F

clean : 
	rm -f *~ high.txt low.txt program.hex program.map program.elf $(OBJS) *.o usbdrv/*.o usbdrv/*~ program.lst

serial :
	stty -F /dev/ttyUSB0 4800 -parenb -echo -cstopb

