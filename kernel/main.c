#include <stdint.h>
#include "isr.h"

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

void main() {
    const char* message = "Hello from C!";
    uint16_t serial_port = 0x3f8; // COM1

    // // Initialize the serial port (optional, if not already done)
    // outb(serial_port + 1, 0x00);    // Disable all interrupts
    // outb(serial_port + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    // outb(serial_port + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    // outb(serial_port + 1, 0x00);    //                  (hi byte)
    // outb(serial_port + 3, 0x03);    // 8 bits, no parity, one stop bit
    // outb(serial_port + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    // outb(serial_port + 4, 0x0B);    // IRQs enabled, RTS/DSR set

    for (int i = 0; message[i] != '\0'; i++) {
        outb(serial_port, message[i]);
    }

    isr_install();

    while(1);
}