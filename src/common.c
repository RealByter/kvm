#include "common.h"

int page_align_up(int address)
{
    return (address + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}

int page_align_down(int address)
{
    return address & ~(PAGE_SIZE - 1);
}