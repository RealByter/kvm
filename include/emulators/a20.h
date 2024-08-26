#ifndef A20_H
#define A20_H

#include "common.h"

// void a20_init();
void a20_handle(uint8_t direction, uint8_t size, uint16_t port, uint32_t count, uint8_t* base, uint64_t data_offset);

#endif