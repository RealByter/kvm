#include "devices/cmos.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "log.h"

LOG_DEFINE("cmos");

static bool nmi_disabled = false;

// REMEMBER THAT 0xf register is whether or not RTC is powered on. right now it's off but maybe i'll want it on
#define NUM_REGISTERS 0xff
static uint8_t registers[NUM_REGISTERS] = {0};

static uint8_t cur_register = 0;

void cmos_init()
{
    uint16_t conventional_memory_kb = 640;
    registers[0x15] = conventional_memory_kb & 0xFF;
    registers[0x16] = (conventional_memory_kb >> 8) & 0xFF;
}

void cmos_handle(exit_io_info_t* io, uint8_t* base)
{
    // printf("data: %x\n", base[data_offset]);
    LOG_MSG("Handling cmos port: 0x%x, direction: 0x%x, size: 0x%x, count: 0x%x, data: 0x%lx", io->port, io->direction, io->size, io->count, base[io->data_offset]);
    switch (io->port)
    {
    case 0x70:
    {
        if (io->direction == KVM_EXIT_IO_OUT)
        {
            nmi_disabled = GET_BITS(base[io->data_offset], 7, 1);
            cur_register = GET_BITS(base[io->data_offset], 0, 7);
            printf("current register: %x\n", cur_register);
        }
        else 
        {
            unhandled(io);
        }
    }
    break;
    case 0x71:
    {
        if (io->direction == KVM_EXIT_IO_IN)
        {
            base[io->data_offset] = registers[cur_register];
        }
        else 
        {
            registers[cur_register] = base[io->data_offset];
        }
    }
    break;
    default:
        unhandled(io);
    }
}