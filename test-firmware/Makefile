all : program.hex burn program.lst
CC = avr-gcc

PROCESSOR=atmega168pb
PROGRAMCODE=m168
CFLAGS = -Os -mmcu=$(PROCESSOR) -DF_CPU=8000000UL -I. -Iusbdrv
ASFLAGS = $(CFLAGS) -x assembler-with-cpp
SRCS :=  main.c util.c 

CFLAGS:=$(CFLAGS) -DHAS_SAMPLES
SRCS:= $(SRCS) samples.c

samples.c : sampleenc
	./sampleenc > samples.c

sampleenc : sampleenc.c
	gcc -o $@ $^

program.elf :$(SRCS)
	avr-gcc -I -mmcu=$(PROCESSOR) $(CFLAGS) -Wl,-Map,program.map -o $@ $^ -L /usr/lib64/binutils/avr/2.19.1

program.hex : program.elf
	avr-objcopy -j .text -j .data -O ihex program.elf program.hex 

program.lst : $(SRCS)
	avr-gcc -c -g -Wa,-a,-ad $(CFLAGS) $^ > $@

burn : program.hex
	sudo avrdude -c usbtiny -p $(PROGRAMCODE) -U flash:w:program.hex -F


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

