#include <stdio.h>
#include <stdlib.h>
#include "io_manager.h"

#define MAX_PORT 0xFFF

static io_handle_t ios[MAX_PORT] = {0};

void io_manager_register(io_init_t init, io_handle_t handle, uint32_t start_port, uint32_t end_port)
{
    if (init != NULL)
    {
        init();
    }
    for (int i = start_port; i <= end_port; i++)
    {
        if (ios[i] != 0)
        {
            printf("Port 0x%d is already registered\n", i);
            exit(1);
        }
        ios[i] = handle;
    }
}

void io_manager_handle(exit_io_info_t *io, uint8_t* base)
{
    if (ios[io->port] == 0)
    {
        printf("Port 0x%x is not registered\n", io->port);
        exit(1);
    }
    ios[io->port](io, base);
}