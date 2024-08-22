#include "emulators/a20.h"

uint8_t bus = 0;

void a20_handle(uint8_t direction, uint8_t size, uint16_t port, uint32_t count, uint8_t *base, uint64_t data_offset)
{
    switch (port)
    {
    case 0x92:
    {
        if(direction == KVM_EXIT_IO_IN)
        {
            base[data_offset] = bus;
        }
        else 
        {
            bus = base[data_offset];
        }
    }
    break;
    default:
    unhandled:
        printf("unhandled KVM_EXIT_IO: direction = 0x%x, size = 0x%x, port = 0x%x, count = 0x%x, offset = 0x%lx\n", direction, size, port, count, data_offset);
        exit(1);
    }
}