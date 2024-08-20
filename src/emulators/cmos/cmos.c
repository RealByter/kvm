#include "cmos.h"
#include <stdio.h>
#include <stdbool.h>

bool nmi_disabled = false;

// REMEMBER THAT 0xf register is whether or not RTC is powered on. right now it's off but maybe i'll want it on
#define NUM_REGISTERS 50
uint8_t registers[NUM_REGISTERS] = {1};

uint8_t cur_register;

void cmos_handle(uint8_t direction, uint8_t size, uint16_t port, uint32_t count, uint8_t *base, uint64_t data_offset)
{
    switch (port)
    {
    case 0x70:
    {
        if (direction == KVM_EXIT_IO_OUT)
        {
            nmi_disabled = GET_BITS(base[data_offset], 7, 1);
            cur_register = GET_BITS(base[data_offset], 0, 7);
            printf("current register: %x\n", cur_register);
        }
        else 
        {
            goto unhandled;
        }
    }
    break;
    case 0x71:
    {
        if (direction == KVM_EXIT_IO_IN)
        {
            base[data_offset] = registers[cur_register];
        }
        else 
        {
            goto unhandled;
        }
        cur_register = 0x0d;
    }
    break;
    default:
        unhandled:
        printf("unhandled KVM_EXIT_IO: direction = 0x%x, size = 0x%x, port = 0x%x, count = 0x%x, offset = 0x%lx\n", direction, size, port, count, data_offset);
    }
}