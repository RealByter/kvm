#ifndef ELF_LOADER_H
#define ELF_LOADER_H

#include <stdint.h>
#include <stdlib.h>

uint32_t elf_load(int vm, uint8_t* code, size_t size);

#endif