#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <linux/kvm.h>
#include <stdlib.h>
#include <stdio.h>
#include "io_manager.h"

#define PAGE_SIZE 0x1000

#define GET_BITS(value, start, length) (((value) >> (start)) & ((1U << (length)) - 1))
#define BUILD_UINT32(addr)                          \
    (((uint32_t)(*((uint8_t *)(addr) + 3)) << 24) | \
     ((uint32_t)(*((uint8_t *)(addr) + 2)) << 16) | \
     ((uint32_t)(*((uint8_t *)(addr) + 1)) << 8) |  \
     ((uint32_t)(*((uint8_t *)(addr) + 0))))

#define WRITE_UINT8(value, addr) (*((uint8_t *)(addr)) = value)
#define WRITE_UINT16(value, addr)                       \
    (*((uint8_t *)(addr) + 0)) = GET_BITS(value, 0, 8); \
    (*((uint8_t *)(addr) + 1)) = GET_BITS(value, 8, 8);
#define WRITE_UINT32(value, addr)                       \
    (*((uint8_t *)(addr) + 0)) = GET_BITS(value, 0, 8); \
    (*((uint8_t *)(addr) + 1)) = GET_BITS(value, 8, 8); \
    (*((uint8_t *)(addr) + 2)) = GET_BITS(value, 16, 8);  \
    (*((uint8_t *)(addr) + 3)) = GET_BITS(value, 24, 8);

int page_align_up(int address);
int page_align_down(int address);

void read_file(char *filename, uint8_t **buf, size_t *size);

void unhandled(exit_io_info_t* io);

#endif