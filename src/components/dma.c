#include "components/dma.h"
#include "common.h"
#include <stdbool.h>
#include <stdlib.h>

#define DMA_SLAVE_BASE 0x00
#define DMA_SLAVE_ADDR0 0x00
#define DMA_SLAVE_COUNT0 0x01
#define DMA_SLAVE_START_ADDR1 0x02
#define DMA_SLAVE_COUNT1 0x03
#define DMA_SLAVE_START_ADDR2 0x04
#define DMA_SLAVE_COUNT2 0x05
#define DMA_SLAVE_START_ADDR3 0x06
#define DMA_SLAVE_COUNT3 0x07

#define DMA_SLAVE_STATUS_COMMAND 0x08
#define DMA_SLAVE_REQUEST 0x09
#define DMA_SLAVE_SINGLE_CHANNEL_MASK 0x0A
#define DMA_SLAVE_MODE 0x0B
#define DMA_SLAVE_FLIPFLOP_RESET 0x0C
#define DMA_SLAVE_INTERMEDIATE_MASTER_RESET 0x0D
#define DMA_SLAVE_MASK_RESET 0x0E
#define DMA_SLAVE_MULTICHANNEL_MASK 0x0F

#define DMA_MASTER_BASE 0xC0
#define DMA_MASTER_ADDR4 0xC0
#define DMA_MASTER_COUNT4 0xC2
#define DMA_MASTER_ADDR5 0xC4
#define DMA_MASTER_COUNT5 0xC6
#define DMA_MASTER_ADDR6 0xC8
#define DMA_MASTER_COUNT6 0xCA
#define DMA_MASTER_ADDR7 0xCC
#define DMA_MASTER_COUNT7 0xCE
#define DMA_MASTER_STATUS_COMMAND 0xD0
#define DMA_MASTER_REQUEST 0xD2
#define DMA_MASTER_SINGLE_CHANNEL_MASK 0xD4
#define DMA_MASTER_MODE 0xD6
#define DMA_MASTER_FLIPFLOP_RESET 0xD8
#define DMA_MASTER_INTERMEDIATE_MASTER_RESET 0xDA
#define DMA_MASTER_MASK_RESET 0xDC
#define DMA_MASTER_MULTICHANNEL_MASK 0xDE

typedef enum
{
    DMA_ON_DEMAND,
    DMA_SINGLE_DMA,
    DMA_BLOCK_DMA,
    DMA_CASCADE,
} dma_mode_t;

typedef enum
{
    DMA_SELF_TEST,
    DMA_WRITE,
    DMA_READ,
    DMA_INVALID
} dma_transfer_t;

typedef struct
{
    dma_mode_t mode;
    bool down;
    bool auto_init;
    dma_transfer_t transfer_type;
    uint8_t selection;
    uint8_t mask;
    bool flip_flop;
    bool status;
} dma_config_t;

typedef struct
{
    dma_config_t config;
} dma_slave_t;

typedef struct
{
    dma_config_t config;
} dma_master_t;

static dma_slave_t dma_slave;
static dma_master_t dma_master;

// uses the "slave" variable from the dma_handle function to get the config struct
#define DMA_CONFIG_CONTROLLER ((dma_config_t *)((slave) ? &(dma_slave.config) : &(dma_master.config)))

static uint8_t flip_flop = 0;

void dma_handle(exit_io_info_t *io, uint8_t *base, bool slave)
{

    if (io->port == DMA_MASTER_INTERMEDIATE_MASTER_RESET || io->port == DMA_SLAVE_INTERMEDIATE_MASTER_RESET)
    {
        if (io->direction == EXIT_IO_OUT)
        {
            dma_config_t *config = DMA_CONFIG_CONTROLLER;
            config->flip_flop = false;
            config->status = 0;
            config->mask = 0x0f;
            return;
        }
    }
    else if (io->port == DMA_MASTER_MODE || io->port == DMA_SLAVE_MODE)
    {
        if (io->direction == EXIT_IO_OUT)
        {
            dma_config_t *config = DMA_CONFIG_CONTROLLER;
            uint8_t data = base[io->data_offset];
            config->mode = GET_BITS(data, 6, 2);
            config->down = GET_BITS(data, 5, 1);
            config->auto_init = GET_BITS(data, 4, 1);
            config->transfer_type = GET_BITS(data, 2, 2);
            config->selection = GET_BITS(data, 0, 2);
            return;
        }
    }
    else if (io->port == DMA_MASTER_SINGLE_CHANNEL_MASK || io->port == DMA_SLAVE_SINGLE_CHANNEL_MASK)
    {
        if (io->direction == EXIT_IO_OUT)
        {
            uint8_t data = base[io->data_offset];
            uint8_t sel = GET_BITS(data, 0, 2);
            uint8_t mask_on = GET_BITS(data, 3, 1);
            uint8_t* mask = &DMA_CONFIG_CONTROLLER->mask;
            *mask = mask_on ? (*mask | (1 << sel)) : (*mask & ~(1 << sel));
            if (!slave && sel == 0 && mask_on) // masking 4 on master cascades to 5, 6 and 7
            {
                dma_master.config.mask = 0x0f;
            }
            return;
        }
    }
    printf("DMA Port 0x%02x is unhandled for direction: %d\n", io->port, io->direction);
    exit(1);
}

void dma_handle_master(exit_io_info_t *io, uint8_t *base)
{
    dma_handle(io, base, false);
}

void dma_handle_slave(exit_io_info_t *io, uint8_t *base)
{
    dma_handle(io, base, true);
}