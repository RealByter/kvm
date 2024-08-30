#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include <stdint.h>

#define EXIT_IO_IN 0
#define EXIT_IO_OUT 1
typedef struct
{
    uint8_t direction;
    uint8_t size;
    uint16_t port;
    uint32_t count;
    uint64_t data_offset;
} exit_io_info_t;

typedef void (*io_handle_t)(exit_io_info_t *io, uint8_t* base);
typedef void (*io_init_t)();

void io_manager_register(io_init_t init, io_handle_t handle, uint32_t start_port, uint32_t end_port);
void io_manager_handle(exit_io_info_t *io, uint8_t* base);

#endif