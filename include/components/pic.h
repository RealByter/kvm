#ifndef PIC_H
#define PIC_H

#include "io_manager.h"

void pic_init_master();
void pic_init_slave();

void pic_handle_master(exit_io_info_t* io, uint8_t* base);
void pic_handle_slave(exit_io_info_t* io, uint8_t* base);

void pic_raise_interrupt(uint8_t irq);

#endif