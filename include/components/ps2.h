#ifndef PS2_H
#define PS2_H

#include "io_manager.h"

void ps2_init();
void ps2_handle(exit_io_info_t* io, uint8_t* base);

#endif