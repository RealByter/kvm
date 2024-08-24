#include "emulators/pci.h"
#include <err.h>

#define CONFIG_ADDRESS 0xcf8
#define CONFIG_DATA 0xcfc

uint32_t config_address = 0;

void pci_handle(uint8_t direction, uint8_t size, uint16_t port, uint32_t count, uint8_t* base, uint64_t data_offset)
{
    getchar();
    switch (port)
    {
        case CONFIG_ADDRESS:
        {
            if(direction == KVM_EXIT_IO_OUT)
            {
                config_address = BUILD_UINT32(base + data_offset);
                printf("config_address: %b;%x\n", config_address, config_address);
            }  
            else
            {
                goto unhandled;
            }
        }
        break;
        case CONFIG_DATA:
        {
            if(direction == KVM_EXIT_IO_IN)
            {
                WRITE_UINT16(0, base + data_offset);
            }
            else
            {
                goto unhandled;
            }
        } 
        break;
    default:
    unhandled:
        printf("unhandled KVM_EXIT_IO: direction = 0x%x, size = 0x%x, port = 0x%x, count = 0x%x, offset = 0x%lx\n", direction, size, port, count, data_offset);
        exit(1);
    }
}