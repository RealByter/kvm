#include <linux/kvm.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <err.h>
#include <elf.h>
#include <fcntl.h>
#include "kvm.h"
#include "elf_loader.h"
#include "iso_loader.h"

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

void print_regs(int vcpu)
{
    struct kvm_regs regs;
    int ret = ioctl(vcpu, KVM_GET_REGS, &regs);
    if (ret == -1)
        err(1, "KVM_GET_REGS");
    printf("rip = 0x%llx, rsp = 0x%llx, rflags = 0x%llx\n"
           "rax = 0x%llx, rbx = 0x%llx, rcx = 0x%llx, rdx = 0x%llx\n"
           "rsi = 0x%llx, rdi = 0x%llx, r8 = 0x%llx, r9 = 0x%llx\n"
           "r10 = 0x%llx, r11 = 0x%llx, r12 = 0x%llx, r13 = 0x%llx\n"
           "r14 = 0x%llx, r15 = 0x%llx\n",
           regs.rip, regs.rsp, regs.rflags,
           regs.rax, regs.rbx, regs.rcx, regs.rdx,
           regs.rsi, regs.rdi, regs.r8, regs.r9,
           regs.r10, regs.r11, regs.r12, regs.r13,
           regs.r14, regs.r15);
}

#define NUM_HANDLERS 256
#define HANDLE_SIZE 4
#define IVT_SIZE (NUM_HANDLERS * HANDLE_SIZE)
#define HANDLER_START 0xf000

void setup_ivt(uint8_t* guest_memory)
{
    uint8_t int13[] = {0xe6, 0x01, 0xcf, 0x00};

    // uint8_t *guest_ivt = mmap(0, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // if (guest_ivt == MAP_FAILED)
        // err(1, "Failed to map IVT");
    // memset(guest_ivt, 0, 0x1000);
    for (int i = 0; i < NUM_HANDLERS * HANDLE_SIZE; i += HANDLE_SIZE)
    {
        uint16_t offset = HANDLER_START + i;
        uint16_t segment = HANDLER_START >> 4;

        guest_memory[i] = offset & 0xff;
        guest_memory[i + 1] = offset >> 8 & 0xff;
        guest_memory[i + 2] = segment & 0xff;
        guest_memory[i + 3] = segment >> 8 & 0xff;
    }
    // struct kvm_userspace_memory_region ivt_mem = {
    //     .slot = 2,
    //     .guest_phys_addr = 0,
    //     .memory_size = 0x1000,
    //     .userspace_addr = (uint64_t)guest_ivt,
    //     .flags = 0};
    // kvm_set_userspace_memory_region(vm, &ivt_mem);

    // uint8_t *guest_handlers = mmap(0xf000, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // if (guest_handlers == MAP_FAILED)
        // err(1, "Failed to map handlers");
    // memset(guest_handlers, 0, 0x1000);
    for (int i = 0; i < NUM_HANDLERS * HANDLE_SIZE; i += HANDLE_SIZE)
    {
        memcpy(guest_memory + 0xf000 + i, int13, sizeof(int13));
    }

    // struct kvm_userspace_memory_region handlers_mem = {
    //     .slot = 3,
    //     .guest_phys_addr = 0xf000,
    //     .memory_size = 0x1000,
    //     .userspace_addr = (uint64_t)guest_handlers,
    //     .flags = 0};
    // kvm_set_userspace_memory_region(vm, &handlers_mem);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        errx(1, "Usage: %s <filename>", argv[0]);
    }

    int kvm, ret, vm, vcpu;
    struct kvm_run *run;

    kvm = kvm_open();
    kvm_verify_version(kvm);

    vm = kvm_create_vm(kvm);
    uint8_t* guest_memory = mmap(0, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    uint8_t *buf;
    size_t size;
    read_file(argv[1], &buf, &size);

    uint32_t entry = iso_load(guest_memory, buf, size);

    setup_ivt(guest_memory);

    vcpu = kvm_create_vcpu(vm);
    run = kvm_map_run(kvm, vcpu);

    struct kvm_sregs sregs;
    kvm_get_sregs(vcpu, &sregs);
    sregs.cs.base = 0;
    sregs.cs.selector = 0;
    sregs.ds.base = 0;
    sregs.ds.selector = 0;
    sregs.es.base = 0;
    sregs.es.selector = 0;
    sregs.fs.base = 0;
    sregs.fs.selector = 0;
    sregs.gs.base = 0;
    sregs.gs.selector = 0;
    sregs.ss.base = 0;
    sregs.ss.selector = 0;
    sregs.cr0 &= ~(1 << 0);
    sregs.idt.base = 0;
    sregs.idt.limit = 0x3ff;
    kvm_set_sregs(vcpu, &sregs);

    struct kvm_regs regs;
    kvm_get_regs(vcpu, &regs);
    regs.rip = entry;
    regs.rflags = 0x202;
    regs.rsp = 0x2000;
    kvm_set_regs(vcpu, &regs);

    // void *guest_stack = mmap(0x1000, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // if (guest_stack == MAP_FAILED)
    //     err(1, "Failed to mmap guest stack");
    // memset(guest_stack, 0, 0x1000);

    // struct kvm_userspace_memory_region stack_region = {
    //     .slot = 1,
    //     .guest_phys_addr = 0x1000,
    //     .memory_size = 0x1000,
    //     .userspace_addr = guest_stack,
    //     .flags = 0,
    // };
    // kvm_set_userspace_memory_region(vm, &stack_region);

    struct kvm_userspace_memory_region guest_space = {
        .slot = 0,
        .guest_phys_addr = 0,
        .memory_size = 0x10000,
        .userspace_addr = guest_memory,
        .flags = 0
    };
    kvm_set_userspace_memory_region(vm, &guest_space);

    struct kvm_interrupt int10 = {
        .irq = 0x13
    };
    ret = ioctl(vcpu, KVM_INTERRUPT, &int10);
    if (ret == -1)
    {
        err(1, "Failed to interrupt");
    }

    ret = ioctl(vm, KVM_CHECK_EXTENSION, KVM_CAP_INTERNAL_ERROR_DATA);
    if (ret == -1)
    {
        err(1, "Failed to check KVM_CAP_INTERNAL_ERROR_DATA");
    }
    // struct kvm_enable_cap cap = {
        // .cap = KVM_CAP_INTERNAL_ERROR_DATA,
        // .flags = 0,
    // };
    // ret = ioctl(vm, KVM_ENABLE_CAP, &cap);
    // if (ret == -1)
    // {
        // err(1, "Failed to enable KVM_CAP_INTERNAL_ERROR_DATA");
    // }

    while (1)
    {
        kvm_run(vcpu);
        print_regs(vcpu);
        kvm_get_regs(vcpu, &regs);
        // for (int i = regs.rsp - 0x1000; i < 0x1000; i++)
        // {
            // printf("0x%x: 0x%x\n", i, *(uint8_t *)(guest_memory + 0x1000 + i));
        // }
        switch (run->exit_reason)
        {
        case KVM_EXIT_HLT:
            puts("KVM_EXIT_HLT");
            // return 0;
            break;
        case KVM_EXIT_IO:
            printf("KVM_EXIT_IO: direction = 0x%x, size = 0x%x, port = 0x%x, count = 0x%x, offset = 0x%x\n", run->io.direction, run->io.size, run->io.port, run->io.count, run->io.data_offset);
            if (run->io.direction == KVM_EXIT_IO_OUT && run->io.size == 1 && run->io.port == 0x3f8 && run->io.count == 1)
            {
                printf("%c", (*(((char *)run) + run->io.data_offset)));
                fflush(stdout);
            }
            else
            {
                printf("got: 0x%x\n", (uint8_t)(*(((char *)run) + run->io.data_offset)));
                // errx(1, "unhandled KVM_EXIT_IO: "
                // "direction = 0x%x, size = 0x%x, port = 0x%x, count = 0x%x\n",
                //  run->io.direction, run->io.size, run->io.port, run->io.count);
                // print_regs(vcpu);
                return;
            }
            break;
        case KVM_EXIT_MMIO:
            printf("KVM_EXIT_MMIO: is_write=%d len=%d phys_addr=0x%llx data=",
                   run->mmio.is_write, run->mmio.len, run->mmio.phys_addr);
            for (int i = 0; i < sizeof(run->mmio.data); i++)
            {
                printf("%02x", run->mmio.data[i]);
            }
            printf("\n");
            print_regs(vcpu);
            return;
            break;
        case KVM_EXIT_FAIL_ENTRY:
            errx(1, "KVM_EXIT_FAIL_ENTRY: hardware_entry_failure_reason = 0x%llx",
                 (unsigned long long)run->fail_entry.hardware_entry_failure_reason);
        case KVM_EXIT_INTERNAL_ERROR:
            // print_regs(vcpu);
            // errx(1, "KVM_EXIT_INTERNAL_ERROR: suberror = 0x%x", run->internal.suberror);
            printf("KVM_EXIT_INTERNAL_ERROR: suberror = 0x%x, ndata = 0x%x\n", run->internal.suberror, run->internal.ndata);
            for (int i = 0; i < run->internal.ndata; i++)
            {
                printf("data[%d] = 0x%llx\n", i, run->internal.data[i]);
            }
            return 1;
        default:
            errx(1, "exit_reason = 0x%x", run->exit_reason);
        }
    }

    free(buf);
    close(kvm);
    return 0;
}