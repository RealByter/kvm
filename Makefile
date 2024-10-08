CC = gcc
CFLAGS = -Wall -g -Iinclude -I/usr/include/SDL2 
LDFLAGS = -lSDL2 -lSDL2_ttf

# Find all .c files recursively under src/ directory
SRCS = $(shell find src -name '*.c')

# Generate corresponding .o files in the build/ directory, preserving directory structure
OBJS = $(SRCS:src/%.c=build/%.o)

DISK_IMAGE = os.iso
TARGET = vmm

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

# Rule to compile .c files into .o files in the build/ directory, preserving directory structure
build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

all: $(TARGET)	

run: $(DISK_IMAGE) $(TARGET)
	(cd kernel && make all)
	# ./$(TARGET) ./seabios/out/bios.bin ../karmiel-506-operatingsystem/os.iso harddisk.img
	./$(TARGET) ./seabios/out/bios.bin ./xv6-public/xv6.img harddisk.img

clean:
	rm -rf build $(TARGET)
