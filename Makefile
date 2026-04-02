CROSS_GCC := $(HOME)/opt/cross/bin/i686-elf-gcc
CROSS_AS  := $(HOME)/opt/cross/bin/i686-elf-as

INCLUDES := -I src/intf
CFLAGS := $(INCLUDES) -ffreestanding -Wall -nostdlib

