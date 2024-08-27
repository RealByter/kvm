#ifndef CMOS_H
#define CMOS_H

#include "common.h"
#include "device_manager.h"

void cmos_init();
void cmos_handle(exit_io_info_t* io, uint8_t* base);

#endif