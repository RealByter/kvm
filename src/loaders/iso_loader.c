#include "loaders/iso_fs.h"
#include <stdio.h>
#include "loaders/iso_loader.h"
#include <string.h>
#include <err.h>
#include <stdlib.h>
#include "common.h"
#include <sys/mman.h>
#include "kvm.h"
#include <string.h>

#define VM_MEMORY_SIZE (16 * 1024 * 1024) // 16 MB

struct eltorito_validation_entry
{
    uint8_t header_id;   // 0x01 for validation entry
    uint8_t platform_id; // 0x00: 80x86, 0x01: PowerPC, 0x02: Mac
    uint16_t reserved;
    char id_string[24]; // "El Torito" string
    uint16_t checksum;
    uint8_t key55; // Must be 0x55
    uint8_t keyAA; // Must be 0xAA
} __attribute__((packed));

// Structure for the El Torito Boot Catalog default entry
struct eltorito_default_entry
{
    uint8_t bootable;      // 0x88 for bootable, 0x00 otherwise
    uint8_t media_type;    // 0x00: no emulation, 0x01: 1.2M floppy, etc.
    uint16_t load_segment; // Typically 0x07C0 for no emulation
    uint8_t system_type;   // 0x00 for no emulation
    uint8_t reserved;
    uint16_t sector_count; // Number of sectors to load
    uint32_t load_rba;     // Logical block address of the boot image
    uint8_t reserved2[20];
} __attribute__((packed));

struct eltorito_section_header
{
    uint8_t header_id; // 0x02 for section header
    uint8_t platform_id;
    uint16_t remaining_sections;
    char id_string[28];
};

#define VM_MEMORY_SIZE (16 * 1024 * 1024) // 16 MB

void validate_pvd(struct iso_primary_descriptor *pvd)
{
    if (strncmp(pvd->id, "CD001", 5))
        errx(1, "Invalid ISO image: incorrect id");
    if (pvd->version[0] != 1)
        errx(1, "Invalid ISO image: incorrect version");
    if (pvd->type[0] != ISO_VD_PRIMARY)
        errx(1, "Invalid ISO image: incorrect type");
}

void validate_validation_entry(struct eltorito_validation_entry *validation_entry)
{
    if (validation_entry->header_id != 0x01)
        errx(1, "Invalid eltorito format: incorrect header id");
    if (validation_entry->key55 != 0x55 || validation_entry->keyAA != 0xAA)
        errx(1, "Invalid eltorito format: incorrect keys");
    if (validation_entry->platform_id != 0x00)
        errx(1, "Invalid eltorito format: incorrect platform id");
}

void validate_default_entry(struct eltorito_default_entry *default_entry)
{
    if (default_entry->bootable != 0x88)
        errx(1, "Invalid eltorito format: not bootable");
    if (default_entry->media_type != 0x00)
        errx(1, "Invalid eltorito format: not a floppy");
}

void validate_bootloader(uint8_t *boot_image, size_t size)
{
    if (size < 512)
        errx(1, "Invalid bootloader: too small");

    if (boot_image[0] != 0xE8 && boot_image[1] != 0x00)
        errx(1, "Invalid bootloader: starts with invalid instruction");

    if (!memmem(boot_image, size, "no boot info", 12) &&
        !memmem(boot_image, size, "cdrom read fails", 16))
        errx(1, "Invalid bootloader: expected strings not found");
}

uint32_t iso_load(uint8_t* guest_memory, uint8_t *iso, size_t size)
{
    struct iso_primary_descriptor *pvd = (struct iso_primary_descriptor *)(iso + 0x10 * ISOFS_BLOCK_SIZE);
    validate_pvd(pvd);

    struct iso_boot_record *br = (struct iso_boot_record *)(iso + 0x11 * ISOFS_BLOCK_SIZE);
    uint32_t block_address = br->el_torito_catalog_block[0] | (br->el_torito_catalog_block[1] << 8) | (br->el_torito_catalog_block[2] << 16) | (br->el_torito_catalog_block[3] << 24);

    struct eltorito_validation_entry *validation_entry = (struct eltorito_validation_entry *)(iso + block_address * ISOFS_BLOCK_SIZE);
    validate_validation_entry(validation_entry);

    struct eltorito_default_entry *default_entry = (struct eltorito_default_entry *)(validation_entry + 1);
    validate_default_entry(default_entry);

    size_t bootloader_size = default_entry->sector_count * ISOFS_BLOCK_SIZE;

    uint8_t *bootloader_ptr = (iso + default_entry->load_rba * ISOFS_BLOCK_SIZE);
    validate_bootloader(bootloader_ptr, bootloader_size);

    uint32_t load_address = default_entry->load_segment ? default_entry->load_segment * 16 : 0x7C00;
    uint32_t mem_start_addr = page_align_down(load_address);
    uint32_t mem_end_addr = page_align_up(load_address + bootloader_size);
    uint32_t mem_size = mem_end_addr - mem_start_addr;
    uint32_t pad_start_size = load_address - mem_start_addr;
    uint32_t pad_end_size = mem_size - pad_start_size - bootloader_size;

    // void* guest_mem = mmap(mem_start_addr, mem_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // if(guest_mem == MAP_FAILED)
    //     err(1, "Failed to mmap guest memory for ELF segment");
    // memset(guest_mem, 0, pad_start_size);
    // memcpy(guest_mem + pad_start_size, bootloader_ptr, bootloader_size);
    // memset(guest_mem + pad_start_size + bootloader_size, 0, pad_end_size);

    // struct kvm_userspace_memory_region mem_region = {
    //     .slot = 0,
    //     .guest_phys_addr = mem_start_addr,
    //     .memory_size = mem_size,
    //     .userspace_addr = guest_mem,
    //     .flags = 0,
    // };

    // kvm_set_userspace_memory_region(vm, &mem_region);

    // void* guest_mem = mmap(0x7000, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // if(guest_mem == MAP_FAILED)
        // err(1, "Failed to mmap guest memory for ELF segment");
    // memset(guest_mem, 0, 0x1000);
    memcpy(guest_memory + 0x7c00, iso, 512);

    // struct kvm_userspace_memory_region mem_region = {
    //     .slot = 0,
    //     .guest_phys_addr = 0x7000,
    //     .memory_size = 0x1000,
    //     .userspace_addr = guest_mem,
    //     .flags = 0,
    // };

    // kvm_set_userspace_memory_region(vm, &mem_region);

    return 0x7c00;
}