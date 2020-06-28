# Makefile for JamesM's kernel tutorials.
# The C and C++ rules are already setup by default.
# The only one that needs changing is the assembler 
# rule, as we use nasm instead of GNU as.

OBJFILES=boot.o main.o monitor.o common.o descriptor_tables.o isr.o interrupt.o gdt.o timer.o

CFLAGS  = -Wall -fno-builtin -fno-stack-protector -nostdinc -nostdlib -ggdb3 -m32
ASFLAGS=-felf
LDFLAGS=-Tlink.ld -melf_i386

all: $(OBJFILES) link

clean:
	rm -f *~ *.o MiniOS.bin MiniOS.bin.dbg

link:
	ld $(LDFLAGS) -o MiniOS.bin $(OBJFILES)
	cp MiniOS.bin MiniOS.bin.dbg 
	strip MiniOS.bin

#подцель компиляции файлов на ассемблере
.s.o:
	nasm $(ASFLAGS) $<
