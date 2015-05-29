BINARY = main
OBJS +=

LDSCRIPT = stm32.ld

include Makefile.include

install: main.bin
	st-flash write main.bin 0x08000000
