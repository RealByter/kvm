#include "elf_loader.h"
#include "../kvm/kvm.h"
#include <elf.h>
#include <sys/mman.h>

uint32_t elf_load(int vm, uint8_t *code, size_t size)
{
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)code;

    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0)
        errx(1, "Not a valid ELF file");

    if (ehdr->e_ident[EI_CLASS] != ELFCLASS32)
        errx(1, "Only 32-bit ELF is supported");

    Elf32_Phdr *phdr = (Elf32_Phdr *)(code + ehdr->e_phoff);
    uint32_t start_addr = 0xFFFFFFFF;
    uint32_t end_addr = 0;

    for (int i = 0; i < ehdr->e_phnum; i++)
    {
        if (phdr[i].p_type == PT_LOAD)
        {
            uint32_t mem_size = phdr[i].p_memsz;
            uint32_t start_vaddr = phdr[i].p_vaddr;
            uint32_t end_vaddr = start_addr + mem_size;

            if (start_vaddr < start_addr)
                start_addr = start_vaddr;
            if (end_vaddr > end_addr)
                end_addr = end_vaddr;
        }
    }

    uint32_t end_addr_aligned = page_align_up(end_addr);
    uint32_t start_addr_aligned = page_align_down(start_addr);
    uint32_t space_size = end_addr_aligned - start_addr_aligned;
    uint8_t *space = malloc(space_size);
    if (space == NULL)
        err(1, "Failed to allocate memory for the guest space");

    printf("start_addr = 0x%x, end_addr = 0x%x, space_size = 0x%x\n", start_addr_aligned, end_addr_aligned, space_size);

    for (int i = 0; i < ehdr->e_phnum; i++)
    {
        if (phdr[i].p_type == PT_LOAD)
        {
            // printf("Segment %d: %d bytes at 0x%x\n", i, phdr[i].p_memsz, phdr[i].p_vaddr);
            uint32_t mem_size = phdr[i].p_memsz;
            uint32_t file_size = phdr[i].p_filesz;
            uint32_t vaddr = phdr[i].p_vaddr;

            memcpy(space + vaddr - start_addr_aligned, code + phdr[i].p_offset, file_size);
            if (mem_size > file_size)
            {
                memset(space + vaddr - start_addr_aligned + file_size, 0, mem_size - file_size);
            }
        }
        printf("type: %x, mem_size: %x, file_size: %x, vaddr: %x\n", phdr[i].p_type, phdr[i].p_memsz, phdr[i].p_filesz, phdr[i].p_vaddr);
    }

    // Allocate and map the segment in guest memory
    void *guest_mem = mmap(start_addr_aligned, space_size + 0x1000, PROT_READ | PROT_WRITE,
                           MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (guest_mem == MAP_FAILED)
        err(1, "Failed to mmap guest memory for ELF segment");
    memcpy(guest_mem, space, space_size);

    struct kvm_userspace_memory_region mem_region = {
        .slot = 0,
        // .guest_phys_addr = start_addr_aligned,
        .guest_phys_addr = start_addr_aligned,
        .memory_size = space_size + 0x1000,
        .userspace_addr = (uint32_t)guest_mem,
        .flags = 0,
    };

    kvm_set_userspace_memory_region(vm, &mem_region);

    return ehdr->e_entry;
}