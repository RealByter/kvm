#include "components/pci.h"
#include "log.h"
#include "common.h"
#include <err.h>
#include <string.h>

LOG_DEFINE("pci");

#define CONFIG_ADDRESS 0xcf8
#define CONFIG_DATA 0xcfc

enum pci_config_space_fields
{
    VENDOR_ID_LOW,
    VENDOR_ID_HIGH,
    DEVICE_ID_LOW,
    DEVICE_ID_HIGH,
    COMMAND_LOW,
    COMMAND_HIGH,
    STATUS_LOW,
    STATUS_HIGH,
    REVISION_ID,
    PROG_IF,
    SUBCLASS,
    CLASS_CODE,
    CACHE_LINE_SIZE,
    LATENCY_TIMER,
    HEADER_TYPE,
    BIST,
    PCI_SUBSYSTEM_VENDOR_ID_LOW = 0x2c,
    PCI_SUBSYSTEM_VENDOR_ID_HIGH,
    SUBSYSTEM_ID_LOW,
    SUBSYSTEM_ID_HIGH,
    // BAR_START,
    // BAR_END = 0x27,
    // CIS_PTR_START,
    // CIS_PTR_END = 0x2B,
    // SUB_VENDOR_ID_START,
    // SUB_VENDOR_ID_END = 0x2F,
    // EXPANSION_ROM_BASE_START,
    // EXPANSION_ROM_BASE_END = 0x33,
    // CAP_PTR,
    // INTERRUPT_LINE = 0x3C,
    // INTERRUPT_PIN,
    PMCCFG_LOW = 0x50,
    PMCCFG_HIGH,
    DETURBO,
    DBC,
    AXC,
    DRAMR_LOW,
    DRAMR_HIGH,
    DRAMC,
    DRAMT,
    PAM0,
    PAM1,
    PAM2,
    PAM3,
    PAM4,
    PAM5,
    PAM6,
    DRB0,
    DRB1,
    DRB2,
    DRB3,
    DRB4,
    DRB5,
    DRB6,
    DRB7,
    FDHC,
    // TOM,
    MTT = 0x70,
    CLT,
    SMRAM,
    ERRCMD = 0x90,
    ERRSTS,
    // MBSC_LOW,
    // MBSC_HIGH,
};

#define VI_INTEL 0x8086

#define DI_I440FX 0x1237

#define PCI_BRIDGE 0

#define MAX_BUSES 256
#define MAX_DEVICES 32
#define MAX_FUNCTIONS 8

static uint32_t last_config_address = 0;
static uint8_t config_index = 0;
static uint8_t config_register = 0;

typedef struct
{
    uint32_t config_space[64];
} pci_device_t;

static pci_device_t devices[MAX_BUSES] = {(pci_device_t){{0}}}; // set all values in the config_space to 0

static inline void pci_set_config_u8(uint8_t device_index, enum pci_config_space_fields field, uint8_t value)
{
    devices[device_index].config_space[field / 4] = (devices[device_index].config_space[field / 4] & ~(0xFF << (8 * (field % 4)))) | (value << (8 * (field % 4)));
}

static inline uint8_t pci_get_config_u8(uint8_t device_index, enum pci_config_space_fields field)
{
    return (devices[device_index].config_space[field / 4] >> (8 * (field % 4))) & 0xFF;
}

static inline void pci_set_config_u16(uint8_t device_index, enum pci_config_space_fields field, uint16_t value)
{
    devices[device_index].config_space[field / 4] = (devices[device_index].config_space[field / 4] & ~(0xFFFF << (8 * (field % 4)))) | (value << (8 * (field % 4)));
}

static inline uint16_t pci_get_config_u16(uint8_t device_index, enum pci_config_space_fields field)
{
    return (devices[device_index].config_space[field / 4] >> (16 * (field % 4))) & 0xFFFF;
}

static inline void pci_set_config_u32(uint8_t device_index, enum pci_config_space_fields field, uint32_t value)
{
    devices[device_index].config_space[field / 4] = value;
}

static inline uint32_t pci_get_config_u32(uint8_t device_index, enum pci_config_space_fields field)
{
    return devices[device_index].config_space[field / 4];
}

void pci_init()
{
    pci_add_device(0, 0, 0, VI_INTEL, DI_I440FX);
    pci_set_config_u16(PCI_BRIDGE, COMMAND_LOW, 0x0006);
    pci_set_config_u16(PCI_BRIDGE, STATUS_LOW, 0x0280);
    pci_set_config_u8(PCI_BRIDGE, REVISION_ID, 0x2);
    pci_set_config_u8(PCI_BRIDGE, CLASS_CODE, 0x06);
    pci_set_config_u8(PCI_BRIDGE, DRB0, 0x01);
    pci_set_config_u16(PCI_BRIDGE, PCI_SUBSYSTEM_VENDOR_ID_LOW, 0x1af4);
    pci_set_config_u16(PCI_BRIDGE, SUBSYSTEM_ID_LOW, 0x1100);
    // pci_set_config_u8(PCI_BRIDGE, TOM, 0x01);

    pci_add_device(0, 2, 0, VI_INTEL, 0x0166);
    pci_set_config_u16(0 * 32 + 2 * 8 + 0, SUBCLASS, 0x0300);
}

void pci_add_device(uint8_t bus, uint8_t device, uint8_t function, uint16_t vendor_id, uint16_t device_id)
{
    uint8_t device_index = bus * 32 + device * 8 + function;
    pci_device_t *pci_device = &devices[device_index];
    printf("device index: %d\n", device_index);

    pci_set_config_u16(device_index, VENDOR_ID_LOW, vendor_id);
    printf("%x\n", pci_get_config_u16(device_index, VENDOR_ID_LOW));
    pci_set_config_u16(device_index, DEVICE_ID_LOW, device_id);
    printf("%x\n", pci_get_config_u16(device_index, DEVICE_ID_LOW));
    printf("%x\n", pci_get_config_u32(device_index, 0));
}

void pci_handle(exit_io_info_t *io, uint8_t *base)
{
    LOG_MSG("Handling pci port: 0x%x, direction: 0x%x, size: 0x%x, count: 0x%x, data: 0x%x", io->port, io->direction, io->size, io->count, READ_UINT32(base + io->data_offset));

    if (io->port == CONFIG_ADDRESS) // SET CONFIG ADDRESS AND REGISTER
    {
        if (io->direction == KVM_EXIT_IO_OUT)
        {
            uint32_t data = READ_UINT32(base + io->data_offset);
            printf("data: %x\n", data);
            last_config_address = data;
            uint32_t bus = (data >> 16) & 0xFF;
            uint32_t device = (data >> 11) & 0x1F;
            uint32_t function = (data >> 8) & 0x07;
            config_register = (data >> 2) & 0x3F;
            config_index = (bus * MAX_DEVICES * MAX_FUNCTIONS) + (device * MAX_FUNCTIONS) + function;
            printf("bus: %x, device: %x, function: %x, config register: %x, config index: %x\n", bus, device, function, config_register, config_index);
        }
        else
        {
            // uint32_t bus = (config_index / (MAX_DEVICES * MAX_FUNCTIONS));
            // uint32_t device = (config_index % (MAX_DEVICES * MAX_FUNCTIONS)) / MAX_FUNCTIONS;
            // uint32_t function = (config_index % (MAX_DEVICES * MAX_FUNCTIONS)) % MAX_FUNCTIONS;

            // base[io->data_offset] = (bus << 16) | (device << 11) | (function << 8) | (config_register << 2);
            printf("last: %x\n", last_config_address);
            WRITE_UINT32(last_config_address, base + io->data_offset);
        }
    }
    else if (io->port >= CONFIG_DATA && io->port <= CONFIG_DATA + 3) // GET/SET CONFIG DATA BASED ON PREVIOUS CONFIG ADDRESS AND REGISTER
    {
        if (io->direction == KVM_EXIT_IO_IN)
        {
            switch (io->size)
            {
            case sizeof(uint8_t):
            {
                uint8_t value = pci_get_config_u8(config_index, config_register * 4 + io->port - CONFIG_DATA);
                printf("value1: %x\n", value);
                WRITE_UINT8(value, base + io->data_offset);
                break;
            }
            case sizeof(uint16_t):
            {
                uint16_t value = pci_get_config_u16(config_index, config_register * 4 + io->port - CONFIG_DATA);
                printf("value2: %x\n", value);
                WRITE_UINT16(value, base + io->data_offset);
                break;
            }
            case sizeof(uint32_t):
            {
                uint32_t value = pci_get_config_u32(config_index, config_register * 4 + io->port - CONFIG_DATA);
                printf("value4: %x\n", value);
                WRITE_UINT32(value, base + io->data_offset);
                break;
            }
            }
        }
        else
        {
            switch (io->size)
            {
            case sizeof(uint8_t):
                pci_set_config_u8(config_index, config_register * 4 + io->port - CONFIG_DATA, READ_UINT8(base + io->data_offset));
                break;
            case sizeof(uint16_t):
                pci_set_config_u16(config_index, config_register * 4 + io->port - CONFIG_DATA, READ_UINT16(base + io->data_offset));
                break;
            case sizeof(uint32_t):
                pci_set_config_u32(config_index, config_register * 4 + io->port - CONFIG_DATA, READ_UINT32(base + io->data_offset));
                break;
            }
        }
    }
    else
    {
        unhandled(io);
    }
}