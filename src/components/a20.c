#include "components/a20.h"
#include "log.h"

LOG_DEFINE("a20");

uint8_t bus = 0;

void a20_handle(exit_io_info_t *io, uint8_t *base)
{
    if (io->direction == KVM_EXIT_IO_IN)
    {
        base[io->data_offset] = bus;
    }
    else
    {
        bus = base[io->data_offset];
    }
}