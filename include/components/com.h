#ifndef COM_H
#define COM_H

#include "io_manager.h"

void com_init();
void com_handle(exit_io_info_t* io, uint8_t* base);

#endif