#ifndef ISO_LOADER_H
#define ISO_LOADER_H

#include <stdint.h>
#include <stdlib.h>

uint32_t iso_load(uint8_t* guest_memory, uint8_t* code, size_t size);

#endif