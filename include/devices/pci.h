#ifndef PCI_H
#define PCI_H

#include <stdint.h>
#include "common.h"
#include "io_manager.h"

void pci_init();
// void pci_add_device(uint8_t bus, uint8_t device, uint8_t function, uint16_t vendor_id, uint16_t device_id);
void pci_handle(exit_io_info_t* io, uint8_t* base);


// void pci_set_config_u8(uint8_t device_index, enum pci_config_space_registers reg, uint8_t value);
// void pci_get_config_u8(uint8_t device_index, enum pci_config_space_registers reg, uint8_t* value);

// void pci_set_config_u16(uint8_t device_index, enum pci_config_space_registers reg, uint16_t value);
// void pci_get_config_u16(uint8_t device_index, enum pci_config_space_registers reg, uint16_t* value);

// void pci_set_config_u32(uint8_t device_index, enum pci_config_space_registers reg, uint32_t value);
// void pci_get_config_u32(uint8_t device_index, enum pci_config_space_registers reg, uint32_t* value);

#endif