#include <stdio.h>
#include <stddef.h>
#include "components/seabios_log.h"

FILE *seabios_log_file;

void seabios_log_init()
{
    seabios_log_file = fopen("bios_log.txt", "w");
    if (seabios_log_file == NULL)
        err(1, "Failed to open log file");
}

void seabios_log_handle(exit_io_info_t *io, uint8_t *base)
{
    if (io->direction == EXIT_IO_OUT)
    {
        fwrite(base + io->data_offset, 1, io->count, seabios_log_file);
        for (int i = 0; i < io->count; i++)
        {
            if (base[io->data_offset + i] == '\n')
            {
                fflush(seabios_log_file);
            }
        }
    }
    else
    {
        base[io->data_offset] = 0xe9; // ack
    }
}