# Makefile for rgbmatrix of dpgs-dev

obj-m += rgbmatrix.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all: rgbmatrix recv

rgbmatrix:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

recv:
	g++ receiver.cpp -o recv

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -rf recv
