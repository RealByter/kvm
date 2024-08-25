#include "emulators/pci.h"
#include <err.h>

#define MAX_DEVICES 256

#define CONFIG_ADDRESS 0xcf8
#define CONFIG_DATA 0xcfc

#define VENDOR_ID_LOW 0
#define VENDOR_ID_HIGH 1
#define DEVICE_ID_LOW 2
#define DEVICE_ID_HIGH 3

uint32_t config_address = 0;

typedef struct
{
    uint8_t config_space[256];
} pci_device_t;

pci_device_t devices[MAX_DEVICES] = {(pci_device_t){{0}}}; // set all values in the config_space to 0

void pci_add_device(uint8_t bus, uint8_t device, uint8_t function, uint16_t vendor_id, uint16_t device_id)
{
    uint8_t device_index = bus * 32 + device * 8 + function;
    pci_device_t *pci_device = &devices[device_index];

    pci_device->config_space[VENDOR_ID_LOW] = vendor_id & 0xff;
    pci_device->config_space[VENDOR_ID_HIGH] = vendor_id >> 8;
    pci_device->config_space[DEVICE_ID_LOW] = device_id & 0xff;
    pci_device->config_space[DEVICE_ID_HIGH] = device_id >> 8;
}

void pci_handle(uint8_t direction, uint8_t size, uint16_t port, uint32_t count, uint8_t *base, uint64_t data_offset)
{
    getchar();
    switch (port)
    {
    case CONFIG_ADDRESS:
    {
        if (direction == KVM_EXIT_IO_OUT)
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
        if (direction == KVM_EXIT_IO_IN)
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