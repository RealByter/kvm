#ifndef EMULATORS_COMMON_H
#define EMULATORS_COMMON_H

#include <linux/kvm.h>
#include <stdint.h>

#define GET_BITS(value, start, length) (((value) >> (start)) & ((1U << (length)) - 1))
#define BUILD_UINT32(addr) \
    (((uint32_t)(*((uint8_t*)(addr) + 0)) << 24) | \
     ((uint32_t)(*((uint8_t*)(addr) + 1)) << 16) | \
     ((uint32_t)(*((uint8_t*)(addr) + 2)) << 8)  | \
     ((uint32_t)(*((uint8_t*)(addr) + 3))))

#define WRITE_UINT16(value, addr) \
    (*((uint8_t*)(addr) + 0)) = GET_BITS(value, 8, 8); \
    (*((uint8_t*)(addr) + 1)) = GET_BITS(value, 0, 8);

#endif