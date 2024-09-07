#include "common.h"

int page_align_up(int address)
{
    return (address + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}

int page_align_down(int address)
{
    return address & ~(PAGE_SIZE - 1);
}

uint32_t get_file_size(FILE* file)
{
    int ret = fseek(file, 0, SEEK_END);
    if (ret < 0)
    {
        err(1, "Failed to seek the file");
    }

    uint32_t size = ftell(file);

    ret = fseek(file, 0, SEEK_SET);
    if (ret < 0)
    {
        err(1, "Failed to seek the file");
    }

    return size;
}

void read_file(char *filename, uint8_t **buf, size_t *size)
{
    FILE *file = fopen(filename, "rb");
    if (file < 0)
    {
        err(1, "Failed to open the file");
    }

    *size = get_file_size(file);

    *buf = malloc(*size);
    if (*buf == NULL)
    {
        err(1, "Failed to allocate memory");
    }

    int ret = fread(*buf, 1, *size, file);
    if (ret < 0)
    {
        err(1, "Failed to read the file");
    }

    fclose(file);
}

void unhandled(exit_io_info_t* io, uint8_t* base)
{
    printf("Unhandled port: 0x%x, direction: 0x%x, size: 0x%x, count: 0x%x, data: 0x%x\n", io->port, io->direction, io->size, io->count, base[io->data_offset]); 
    exit(1);
}