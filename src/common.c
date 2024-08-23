#include "common.h"

int page_align_up(int address)
{
    return (address + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}

int page_align_down(int address)
{
    return address & ~(PAGE_SIZE - 1);
}

void read_file(char *filename, uint8_t **buf, size_t *size)
{
    FILE *file = fopen(filename, "rb");
    if (file < 0)
    {
        err(1, "Failed to open the file");
    }

    int ret = fseek(file, 0, SEEK_END);
    if (ret < 0)
    {
        err(1, "Failed to seek the file");
    }

    *size = ftell(file);

    ret = fseek(file, 0, SEEK_SET);
    if (ret < 0)
    {
        err(1, "Failed to seek the file");
    }

    *buf = malloc(*size);
    if (*buf == NULL)
    {
        err(1, "Failed to allocate memory");
    }

    ret = fread(*buf, 1, *size, file);
    if (ret < 0)
    {
        err(1, "Failed to read the file");
    }

    fclose(file);
}