#include <stdio.h>
#include <stdlib.h>
#include "device_manager.h"

#define MAX_PORT 0xFFF

static device_handle_t devices[MAX_PORT] = {0};

void device_manager_register(device_init_t init, device_handle_t handle, uint32_t start_port, uint32_t end_port)
{
    if (init != NULL)
    {
        init();
    }
    for (int i = start_port; i <= end_port; i++)
    {
        if (devices[i] != 0)
        {
            printf("Port 0x%d is already registered\n", i);
            exit(1);
        }
        devices[i] = handle;
    }
}

void device_manager_handle(exit_io_info_t *io, uint8_t* base)
{
    if (devices[io->port] == 0)
    {
        printf("Port 0x%x is not registered\n", io->port);
        exit(1);
    }
    devices[io->port](io, base);
}