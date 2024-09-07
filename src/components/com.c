#include <stdio.h>
#include "components/com.h"
#include "common.h"

static FILE* com_file = NULL;

static uint8_t com_interrupt_enable = 0;

void com_init()
{
    com_file = fopen("com.txt", "w");
    if (com_file == NULL)
    {
        err(1, "Failed to open log file");
    }
}

void com_handle(exit_io_info_t* io, uint8_t* base)
{
    if (io->direction == EXIT_IO_OUT)
    {
        switch(io->port)
        {
            case 0x3f8:
                fprintf(com_file, "%c", base[io->data_offset]);
                fflush(com_file);
                break;
            case 0x3f9:
                com_interrupt_enable = base[io->data_offset];
                break;
            default:
                unhandled(io, base);
        }
    }
    else // EXIT_IO_IN
    {
        switch(io->port)
        {
            case 0x3f9:
                base[io->data_offset] = com_interrupt_enable;
                break;
            case 0x3fa:
                base[io->data_offset] = 0x02; // transmitter holding register empty
                break;
            default:
                unhandled(io, base);
        }
    }
}