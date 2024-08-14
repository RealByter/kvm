CC = gcc
CFLAGS = -Wall -g -Iinclude
LDFLAGS = 

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:src/%.c=build/%.o)

DISK_IMAGE = kernel.elf
TARGET = vmm

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

all: $(TARGET)

$(DISK_IMAGE):
	(cd kernel && make all)

run: $(DISK_IMAGE) $(TARGET)
	./$(TARGET) $(DISK_IMAGE)

clean:
	rm -rf build $(TARGET)