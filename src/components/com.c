#include <stdio.h>
#include "components/com.h"

void com_handle(exit_io_info_t* io, uint8_t* base)
{
    if (io->direction == EXIT_IO_OUT)
    {
        printf("%c", base[io->data_offset]);
        fflush(stdout);
    }
}