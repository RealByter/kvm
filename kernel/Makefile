# Compiler and linker settings
ASM = nasm
CC = gcc
LD = ld

# Flags
ASMFLAGS = -f elf32
CFLAGS = -m32 -c -ffreestanding -fno-pie -fno-stack-protector -nostdlib
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib

# Source files
ASM_SRC = bootloader.asm
C_SRC = main.c

# Object files
ASM_OBJ = $(ASM_SRC:.asm=.o)
C_OBJ = $(C_SRC:.c=.o)

# Output files
BOOTLOADER = bootloader.bin
DISK_IMAGE = kernel.elf

# Phony targets
.PHONY: all clean

# Default target
all: $(DISK_IMAGE)
	cp $(DISK_IMAGE) ../$(DISK_IMAGE)

# Build the bootable disk image
$(DISK_IMAGE): $(BOOTLOADER)
	dd if=/dev/zero of=$(DISK_IMAGE) bs=512 count=2880
	dd if=$(BOOTLOADER) of=$(DISK_IMAGE) conv=notrunc

# Link the bootloader and C code
$(BOOTLOADER): $(ASM_OBJ) $(C_OBJ)
	$(LD) $(LDFLAGS) $^ -o $@

# Compile assembly code
%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

# Compile C code
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

# Clean up build artifacts
clean:
	rm -f $(ASM_OBJ) $(C_OBJ) $(BOOTLOADER) $(DISK_IMAGE)

# Run the bootloader in QEMU
run: $(DISK_IMAGE)
	qemu-system-i386 -fda $(DISK_IMAGE)