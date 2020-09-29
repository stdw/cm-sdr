PLATFORM=7220-5.7.1.9
TCPREFIX=mips-linux-
CC=$(TCPREFIX)gcc
OBJCOPY=$(TCPREFIX)objcopy
LDSCRIPT=$(PLATFORM)/cm-sdr.ld
CFLAGS=-march=mips32 -mabi=eabi -msoft-float -mno-abicalls -fno-builtin -nostdlib -nodefaultlibs -nostartfiles -T $(LDSCRIPT)

default: cm-sdr.bin

cm-sdr.elf: cm-sdr.c
	$(CC) cm-sdr.c -o $@ $(CFLAGS)

cm-sdr.bin: cm-sdr.elf
	$(OBJCOPY) -O binary -j .start -j .text -j .data -j .rodata $< $@


.PHONY: clean

clean:
	rm -f cm-sdr.elf cm-sdr.bin
