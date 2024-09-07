#ifndef SEABIOS_LOG_H
#define SEABIOS_LOG_H

#include <stdio.h>
#include "io_manager.h"

void seabios_log_init();
void seabios_log_handle(exit_io_info_t* io, uint8_t* base);

FILE* seabios_log_get_file();

#endif