#ifndef ATA_H
#define ATA_H

#include "io_manager.h"

void ata_handle_io_master(exit_io_info_t* io, uint8_t* base);
void ata_handle_control_master(exit_io_info_t* io, uint8_t* base);
void ata_handle_io_slave(exit_io_info_t* io, uint8_t* base);
void ata_handle_control_slave(exit_io_info_t* io, uint8_t* base);

#endif