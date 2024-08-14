CC = gcc
CFLAGS = -Wall -g -Iinclude
LDFLAGS = 

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:src/%.c=build/%.o)

TARGET = vmm

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

all: $(TARGET)

run: $(TARGET)
	./$(TARGET) ../karmiel-506-operatingsystem/os.iso

clean:
	rm -rf build $(TARGET)