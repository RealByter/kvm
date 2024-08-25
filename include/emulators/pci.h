#ifndef PCI_H
#define PCI_H

#include "common.h"

void add_device(uint8_t bus, uint8_t device, uint8_t function, uint16_t vendor_id, uint16_t device_id);
void pci_handle(uint8_t direction, uint8_t size, uint16_t port, uint32_t count, uint8_t* base, uint64_t data_offset);

#endif