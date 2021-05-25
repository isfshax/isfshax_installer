C_FILES  := $(foreach dir, ., $(wildcard $(dir)/*.c))
S_FILES  := $(foreach dir, ., $(wildcard $(dir)/*.S))

OBJS  := $(C_FILES:.c=.o)  $(S_FILES:.S=.o)

PREFIX  = $(DEVKITARM)/bin/arm-none-eabi
CC      = $(PREFIX)-gcc
LD      = $(PREFIX)-ld
STRIP   = $(PREFIX)-strip
OBJCOPY = $(PREFIX)-objcopy

ASFLAGS   = -marm -fomit-frame-pointer -mbig-endian -fshort-wchar -mcpu=arm926ej-s -march=armv5te -fno-zero-initialized-in-bss
CFLAGS   = -Os -Wall -fpic $(ASFLAGS)
LDFLAGS = -n -nostartfiles -Wl,-EB -L"$(DEVKITARM)/arm-none-eabi/lib"

all: elfloader.bin

elfloader.bin: elfloader.elf
	$(OBJCOPY) -S -O binary $^ $@

elfloader.elf: $(OBJS)
	$(CC) $(LDFLAGS) -T stub.ld $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@
%.o: %.S
	$(CC) $(ASFLAGS) -c $^ -o $@

clean:
	@rm -rf $(OBJS) elfloader.elf elfloader.bin
	@echo "Cleaned!"
