#include "devices/dma.h"
#include <stdlib.h>

#define SLAVE_BASE 0x00
#define SLAVE_ADDR0 0x00
#define SLAVE_COUNT0 0x01
#define SLAVE_START_ADDR1 0x02
#define SLAVE_COUNT1 0x03
#define SLAVE_START_ADDR2 0x04
#define SLAVE_COUNT2 0x05
#define SLAVE_START_ADDR3 0x06
#define SLAVE_COUNT3 0x07

#define SLAVE_STATUS_COMMAND 0x08
#define SLAVE_REQUEST 0x09
#define SLAVE_SINGLE_CHANNEL_MASK 0x0A
#define SLAVE_MODE 0x0B
#define SLAVE_FLIPFLOP_RESET 0x0C
#define SLAVE_INTERMEDIATE_MASTER_RESET 0x0D
#define SLAVE_MASK_RESET 0x0E
#define SLAVE_MULTICHANNEL_MASK 0x0F
#define SLAVE_INDEX(n) ((n) - SLAVE_BASE)

#define MASTER_BASE 0xC0
#define MASTER_ADDR4 0xC0
#define MASTER_COUNT4 0xC2
#define MASTER_ADDR5 0xC4
#define MASTER_COUNT5 0xC6
#define MASTER_ADDR6 0xC8
#define MASTER_COUNT6 0xCA
#define MASTER_ADDR7 0xCC
#define MASTER_COUNT7 0xCE
#define MASTER_STATUS_COMMAND 0xD0
#define MASTER_REQUEST 0xD2
#define MASTER_SINGLE_CHANNEL_MASK 0xD4
#define MASTER_MODE 0xD6
#define MASTER_FLIPFLOP_RESET 0xD8
#define MASTER_INTERMEDIATE_MASTER_RESET 0xDA
#define MASTER_MASK_RESET 0xDC
#define MASTER_MULTICHANNEL_MASK 0xDE
#define MASTER_INDEX(n) ((n) - MASTER_BASE)

#define MAX_REGISTERS 0x0f

static uint8_t *slave_registers[MAX_REGISTERS] = {0};
static uint16_t *master_registers[MAX_REGISTERS] = {0};

static uint8_t master_flip_flop = 0;
static uint8_t mask = 0;

// void dma_handle(exit_io_info_t* io, uint8_t* base, uint8_t port_offset, uint8_t* registers)
// {

// }

void dma_handle_master(exit_io_info_t *io, uint8_t *base)
{
    // dma_handle(io, base, master_registers);
    switch (io->port)
    {
    case 0x00:
    {
        if (io->direction == EXIT_IO_OUT)
        {
            master_flip_flop = 0;
            master_registers[MASTER_INDEX(MASTER_STATUS_COMMAND)] = 0;
            mask |= 0xf0; // mask all master channels
        }
        else
        {
            goto dma_master_unhandled;
        }
    }
    default:
    {
    dma_master_unhandled:
        printf("DMA Port 0x%02x is unhandled for direction: %d\n", io->port, io->direction);
        exit(1);
    }
    }
}

void dma_handle_slave(exit_io_info_t *io, uint8_t *base)
{
    // dma_handle(io, base, slave_registers);
    switch (io->port)
    {
    case MASTER_INTERMEDIATE_MASTER_RESET:
    {
        if (io->direction == EXIT_IO_OUT)
        {
            printf("slave: write to master: %x\n", base[io->data_offset]);
        }
        else
        {
            goto dma_slave_unhandled;
        }
    }
    default:
    {
    dma_slave_unhandled:
        printf("DMA Port 0x%02x is unhandled for direction: %d\n", io->port, io->direction);
        exit(1);
    }
    }
}