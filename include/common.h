#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <linux/kvm.h>
#include <stdlib.h>
#include <stdio.h>

#define PAGE_SIZE 0x1000

#define GET_BITS(value, start, length) (((value) >> (start)) & ((1U << (length)) - 1))
#define BUILD_UINT32(addr)                          \
    (((uint32_t)(*((uint8_t *)(addr) + 0)) << 24) | \
     ((uint32_t)(*((uint8_t *)(addr) + 1)) << 16) | \
     ((uint32_t)(*((uint8_t *)(addr) + 2)) << 8) |  \
     ((uint32_t)(*((uint8_t *)(addr) + 3))))

#define WRITE_UINT16(value, addr)                       \
    (*((uint8_t *)(addr) + 0)) = GET_BITS(value, 8, 8); \
    (*((uint8_t *)(addr) + 1)) = GET_BITS(value, 0, 8);

int page_align_up(int address);
int page_align_down(int address);

void read_file(char *filename, uint8_t **buf, size_t *size);

#endif