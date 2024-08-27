#include "devices/seabios_info.h"
#include <stdio.h>
#include <common.h>

static char *sig = "QEMO";
static int i = 0;

void seabios_info_handle(exit_io_info_t *io, uint8_t *base)
{
    if (io->port == 0x510)
    {
        if (io->direction == EXIT_IO_OUT)
        {
            printf("set to value: %x\n", base[io->data_offset]);
        }
        else
        {
            unhandled(io);
        }
    }
    else if (io->port == 0x511)
    {
        if (io->direction == EXIT_IO_IN)
        {
            for (int j = 0; j < io->count; j++)
            {
                base[io->data_offset + j] = sig[i + j];
                if (sig[i + j] == '\n')
                {
                    i = 0;
                }
                else
                {
                    i++;
                }
            }
        }
        else
        {
            unhandled(io);
        }
    }
}