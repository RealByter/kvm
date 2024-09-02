#ifndef PIC_H
#define PIC_H

#include "io_manager.h"

// Master
#define PIC_IRQ0 0x0
#define PIC_IRQ1 0x1
#define PIC_IRQ2 0x2
#define PIC_IRQ3 0x3
#define PIC_IRQ4 0x4
#define PIC_IRQ5 0x5
#define PIC_IRQ6 0x6
#define PIC_IRQ7 0x7
// Slave
#define PIC_IRQ8 0x8
#define PIC_IRQ9 0x9
#define PIC_IRQ10 0xa
#define PIC_IRQ11 0xb
#define PIC_IRQ12 0xc
#define PIC_IRQ13 0xd
#define PIC_IRQ14 0xe
#define PIC_IRQ15 0xf

void pic_init_master();
void pic_init_slave();

void pic_handle_master(exit_io_info_t* io, uint8_t* base);
void pic_handle_slave(exit_io_info_t* io, uint8_t* base);

void pic_raise_interrupt(uint8_t irq);

#endif