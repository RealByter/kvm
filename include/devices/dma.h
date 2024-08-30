#ifndef DMA_H
#define DMA_H

#include "io_manager.h"

void dma_handle_master(exit_io_info_t* io, uint8_t* base);
void dma_handle_slave(exit_io_info_t* io, uint8_t* base);

#endif