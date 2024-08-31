#include "devices/dma.h"
#include "common.h"
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

typedef enum
{
    ON_DEMAND,
    SINGLE_DMA,
    BLOCK_DMA,
    CASCADE,
} dma_mode_t;

typedef enum
{
    SELF_TEST,
    WRITE,
    READ,
    INVALID
} dma_transfer_t;

typedef struct
{
    dma_mode_t mode;
    uint8_t down;
    uint8_t auto_init;
    dma_transfer_t transfer_type;
    uint8_t selection;
    uint8_t mask;
    uint8_t flip_flop;
    uint8_t status;
} dma_config_t;

typedef struct
{
    dma_config_t config;
} dma_slave_t;

typedef struct
{
    dma_config_t config;
} dma_master_t;

static dma_slave_t slave_controller;
static dma_master_t master_controller;

// uses the "slave" variable from the dma_handle function to get the config struct which will always be in the beginning
#define CONFIG_CONTROLLER ((dma_config_t *)((slave) ? &slave_controller : &master_controller))

static uint8_t flip_flop = 0;

void dma_handle(exit_io_info_t *io, uint8_t *base, uint8_t slave)
{

    if (io->port == MASTER_INTERMEDIATE_MASTER_RESET || io->port == SLAVE_INTERMEDIATE_MASTER_RESET)
    {
        if (io->direction == EXIT_IO_OUT)
        {
            dma_config_t *config = CONFIG_CONTROLLER;
            config->flip_flop = 0;
            config->status = 0;
            config->mask = 0x0f;
            return;
        }
    }
    else if (io->port == MASTER_MODE || io->port == SLAVE_MODE)
    {
        if (io->direction == EXIT_IO_OUT)
        {
            dma_config_t *config = CONFIG_CONTROLLER;
            uint8_t data = base[io->data_offset];
            config->mode = GET_BITS(data, 6, 2);
            config->down = GET_BITS(data, 5, 1);
            config->auto_init = GET_BITS(data, 4, 1);
            config->transfer_type = GET_BITS(data, 2, 2);
            config->selection = GET_BITS(data, 0, 2);
            return;
        }
    }
    else if (io->port == MASTER_SINGLE_CHANNEL_MASK || io->port == SLAVE_SINGLE_CHANNEL_MASK)
    {
        if (io->direction == EXIT_IO_OUT)
        {
            uint8_t data = base[io->data_offset];
            uint8_t sel = GET_BITS(data, 0, 2);
            uint8_t mask_on = GET_BITS(data, 3, 1);
            uint8_t* mask = &CONFIG_CONTROLLER->mask;
            *mask = mask_on ? (*mask | (1 << sel)) : (*mask & ~(1 << sel));
            if (!slave && sel == 0 && mask_on) // masking 4 on master cascades to 5, 6 and 7
            {
                master_controller.config.mask = 0x0f;
            }
            return;
        }
    }
    printf("DMA Port 0x%02x is unhandled for direction: %d\n", io->port, io->direction);
    exit(1);
}

void dma_handle_master(exit_io_info_t *io, uint8_t *base)
{
    dma_handle(io, base, 0);
}

void dma_handle_slave(exit_io_info_t *io, uint8_t *base)
{
    dma_handle(io, base, 1);
}