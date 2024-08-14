# Compiler and linker settings
ASM = nasm
CC = gcc
LD = ld

# Flags
ASMFLAGS = -f elf32
CFLAGS = -m32 -c -ffreestanding -fno-pie -fno-stack-protector -nostdlib
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib

# Source files
ASM_SRC = $(wildcard *.asm)
C_SRC = $(wildcard *.c)
S_SRC = $(wildcard *.s)

# Object files
ASM_OBJ = $(ASM_SRC:.asm=.o)
C_OBJ = $(C_SRC:.c=.o)
S_OBJ = $(S_SRC:.s=.o)

# Output files
KERNEL = kernel.elf
BOOTLOADER = bootloader.bin

# Default target
all: $(KERNEL)
	cp $(KERNEL) ../$(KERNEL)

# Build the kernel
$(KERNEL): $(ASM_OBJ) $(C_OBJ) $(S_OBJ)
	$(LD) $(LDFLAGS) $^ -o $@

# Compile assembly code (.asm files)
%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

# Compile C code
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

# Compile assembly code (.s files)
%.o: %.s
	$(ASM) $(ASMFLAGS) $< -o $@

# Clean up build artifacts
clean:
	rm -f $(ASM_OBJ) $(C_OBJ) $(S_OBJ) $(KERNEL)

# Run the kernel in QEMU
run: $(KERNEL)
	qemu-system-i386 -kernel $(KERNEL)