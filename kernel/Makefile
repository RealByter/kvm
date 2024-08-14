# Compiler and tool settings
ASM = nasm
CC = gcc
LD = ld
GRUB_MKRESCUE = grub-mkrescue

# Flags
ASMFLAGS = -f elf32
CFLAGS = -m32 -c -ffreestanding -fno-pie -fno-stack-protector -nostdlib
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib

# Source files
ASM_SRC = bootloader.asm
C_SRC = main.c
KVM_SRC = kvm_iso_runner.c

# Object files
ASM_OBJ = $(ASM_SRC:.asm=.o)
C_OBJ = $(C_SRC:.c=.o)

# Output files
BOOTLOADER = bootloader.bin
KERNEL = main.bin
ISO = myos.iso
KVM_RUNNER = kvm_iso_runner

# Phony targets
.PHONY: all clean run

# Default target
all: $(ISO) $(KVM_RUNNER)

# Build the bootable ISO
$(ISO): $(BOOTLOADER) $(KERNEL)
	mkdir -p iso/boot/grub
	cp $(BOOTLOADER) iso/boot/
	cp $(KERNEL) iso/boot/
	$(GRUB_MKRESCUE) -o $(ISO) iso

# Compile and link the bootloader
$(BOOTLOADER): $(ASM_OBJ)
	$(LD) $(LDFLAGS) $^ -o $@

# Compile and link the kernel
$(KERNEL): $(C_OBJ)
	$(LD) $(LDFLAGS) $^ -o $@

# Compile assembly code
%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

# Compile C code
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

# Compile the KVM runner
$(KVM_RUNNER): $(KVM_SRC)
	$(CC) -o $@ $^

# Clean up build artifacts
clean:
	rm -f $(ASM_OBJ) $(C_OBJ) $(BOOTLOADER) $(KERNEL) $(ISO) $(KVM_RUNNER)
	rm -rf iso

# Run the OS in KVM
run: $(ISO) $(KVM_RUNNER)
	./$(KVM_RUNNER) $(ISO)