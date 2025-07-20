EXTRA_CFLAGS = -Wall

obj-m := gpio_driver.o
gpio_driver-objs := main.o gpio.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
