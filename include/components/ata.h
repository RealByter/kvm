#ifndef ATA_H
#define ATA_H

#include "io_manager.h"

void ata_init_disks(char* kernel_path, char* harddisk_path);
void ata_deinit_disks();

void ata_init_primary();
void ata_init_secondary();

void ata_handle_io_primary(exit_io_info_t* io, uint8_t* base);
void ata_handle_control_primary(exit_io_info_t* io, uint8_t* base);
void ata_handle_io_secondary(exit_io_info_t* io, uint8_t* base);
void ata_handle_control_secondary(exit_io_info_t* io, uint8_t* base);

#endif