#ifndef PCI_H
#define PCI_H

#include "common.h"

void pci_handle(uint8_t direction, uint8_t size, uint16_t port, uint32_t count, uint8_t* base, uint64_t data_offset);

#endif