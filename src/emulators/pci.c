#include "emulators/pci.h"
#include "log.h"
#include "common.h"
#include <err.h>
#include <string.h>

LOG_DEFINE("pci");

#define CONFIG_ADDRESS 0xcf8
#define CONFIG_DATA 0xcfc

enum pci_config_space_registers
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

uint8_t config_index = 0;
uint8_t config_register = 0;

typedef struct
{
    uint8_t config_space[256];
} pci_device_t;

pci_device_t devices[MAX_BUSES] = {(pci_device_t){{0}}}; // set all values in the config_space to 0

static inline void pci_set_config_u8(uint8_t device_index, enum pci_config_space_registers reg, uint8_t value)
{
    devices[device_index].config_space[reg] = value;
}

static inline void pci_get_config_u8(uint8_t device_index, enum pci_config_space_registers reg, uint8_t *value)
{
    *value = devices[device_index].config_space[reg];
}

static inline void pci_set_config_u16(uint8_t device_index, enum pci_config_space_registers reg, uint16_t value)
{
    devices[device_index].config_space[reg] = value & 0xFF;
    devices[device_index].config_space[reg + 1] = (value >> 8) & 0xFF;
}

static inline void pci_get_config_u16(uint8_t device_index, enum pci_config_space_registers reg, uint16_t *value)
{
    *value = devices[device_index].config_space[reg] |
             (devices[device_index].config_space[reg + 1] << 8);
}

static inline void pci_set_config_u32(uint8_t device_index, enum pci_config_space_registers reg, uint32_t value)
{
    devices[device_index].config_space[reg] = value & 0xFF;
    devices[device_index].config_space[reg + 1] = (value >> 8) & 0xFF;
    devices[device_index].config_space[reg + 2] = (value >> 16) & 0xFF;
    devices[device_index].config_space[reg + 3] = (value >> 24) & 0xFF;
}

static inline void pci_get_config_u32(uint8_t device_index, enum pci_config_space_registers reg, uint32_t *value)
{
    *value = devices[device_index].config_space[reg] |
             (devices[device_index].config_space[reg + 1] << 8) |
             (devices[device_index].config_space[reg + 2] << 16) |
             (devices[device_index].config_space[reg + 3] << 24);
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
}

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
    LOG_MSG("Handling pci port: 0x%x, direction: 0x%x, size: 0x%x, count: 0x%x, data: 0x%x", port, direction, size, count, BUILD_UINT32(base + data_offset));

    if (port == CONFIG_ADDRESS) // SET CONFIG ADDRESS AND REGISTER
    {
        if (direction == KVM_EXIT_IO_OUT)
        {
            uint32_t data = BUILD_UINT32(base + data_offset);
            uint32_t bus = (data >> 16) & 0xFF;
            uint32_t device = (data >> 11) & 0x1F;
            uint32_t function = (data >> 8) & 0x07;
            config_register = (data >> 2) & 0x3F;
            config_index = (bus * MAX_DEVICES * MAX_FUNCTIONS) + (device * MAX_FUNCTIONS) + function;
            printf("config_address: %b;%x, config_register: %b;%x\n", config_index, config_index, config_register, config_register);
        }
        else
        {
            printf("unhandled KVM_EXIT_IO: direction = 0x%x, size = 0x%x, port = 0x%x, count = 0x%x, offset = 0x%lx\n", direction, size, port, count, data_offset);
            exit(1);
        }
    }
    if (port >= CONFIG_DATA && port <= CONFIG_DATA + 3) // GET/SET CONFIG DATA BASED ON PREVIOUS CONFIG ADDRESS AND REGISTER
    {
        if (direction == KVM_EXIT_IO_IN)
        {
            uint32_t value;
            memcpy(&value, &devices[config_index].config_space[config_register + port - CONFIG_DATA], size);
            printf("value: %x\n", value);
            memcpy(base + data_offset, &devices[config_index].config_space[config_register + port - CONFIG_DATA], size);
        }
        else
        {
            uint32_t additional_offset = port - CONFIG_DATA;
            memcpy(&devices[config_index].config_space[config_register + additional_offset], base + data_offset, size);
        }
    }
}