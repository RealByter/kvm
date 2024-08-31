#include "devices/pic.h"

void pic_handle(exit_io_info_t* io, uint8_t* base, uint8_t slave)
{

}

void pic_handle_master(exit_io_info_t* io, uint8_t* base)
{
    pic_handle(io, base, 0);
}

void pic_handle_slave(exit_io_info_t* io, uint8_t* base)
{
    pic_handle(io, base, 0);
}