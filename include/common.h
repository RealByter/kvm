#ifndef COMMON_H
#define COMMON_H

#define PAGE_SIZE 0x1000

int page_align_up(int address);
int page_align_down(int address);

#endif