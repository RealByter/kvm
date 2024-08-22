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
#include "loaders/elf_loader.h"
#include "loaders/iso_loader.h"
#include "common.h"
#include "emulators/cmos.h"
#include "emulators/a20.h"
#include "emulators/pci.h"
#include "gui.h"

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

void print_sregs(int vcpu)
{
    struct kvm_sregs sregs;
    int ret = ioctl(vcpu, KVM_GET_SREGS, &sregs);
    if (ret == -1)
        err(1, "KVM_GET_SREGS");
    printf("sregs:\n");
    printf("  cs: 0x%x\n", sregs.cs.base);
    printf("  ds: 0x%x\n", sregs.ds.base);
    printf("  es: 0x%x\n", sregs.es.base);
    printf("  fs: 0x%x\n", sregs.fs.base);
    printf("  gs: 0x%x\n", sregs.gs.base);
    printf("  ss: 0x%x\n", sregs.ss.base);
    printf("  tr: 0x%x\n", sregs.tr.base);
}

uint8_t *init_kvm(int *kvm, int *vm, int *vcpu, struct kvm_run **run, char *file_name)
{
    *kvm = kvm_open();
    kvm_verify_version(*kvm);

    *vm = kvm_create_vm(*kvm);

    uint8_t *buf;
    size_t size;
    read_file(file_name, &buf, &size);
    printf("size: %d\n", size);
    // uint8_t *isa = mmap(0xf0000, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // if (isa == MAP_FAILED)
    // err(1, "Failed to map ISA");
    // memcpy(isa, buf + size - 0x10000, 0x10000);
    uint8_t *guest_memory = mmap(0, 0x100000, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (guest_memory == MAP_FAILED)
        err(1, "Failed to map guest memory");
    memcpy(guest_memory + 0x100000 - size, buf, size);

    // struct kvm_userspace_memory_region memory_region = {
    // .slot = 0,
    // .guest_phys_addr = 0xf0000,
    // .memory_size = 0x10000,
    // .userspace_addr = (uint64_t)isa,
    // .flags = 0};
    struct kvm_userspace_memory_region memory_region = {
        .slot = 0,
        .guest_phys_addr = 0,
        .memory_size = 0x100000,
        .userspace_addr = (uint64_t)guest_memory,
        .flags = 0};
    kvm_set_userspace_memory_region(*vm, &memory_region);

    *vcpu = kvm_create_vcpu(*vm);
    *run = kvm_map_run(*kvm, *vcpu);

    struct kvm_sregs sregs;
    kvm_get_sregs(*vcpu, &sregs);
    sregs.cs.base = 0xF0000;
    sregs.cs.selector = 0xF000;
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
    kvm_set_sregs(*vcpu, &sregs);

    struct kvm_regs regs;
    kvm_get_regs(*vcpu, &regs);
    regs.rip = 0xFFF0;
    regs.rflags = 0x2;
    kvm_set_regs(*vcpu, &regs);

    return buf;
}

void run_kvm(int vcpu, struct kvm_run *run)
{
    struct kvm_regs regs;
    FILE *log = fopen("log.txt", "w");
    if (log == NULL)
        err(1, "Failed to open log file");

    while (1)
    {
        kvm_run(vcpu);
        print_regs(vcpu);
        print_sregs(vcpu);
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
            if (run->io.port >= 0x70 && run->io.port <= 0x71)
            {
                cmos_handle(run->io.direction, run->io.size, run->io.port, run->io.count, (uint8_t *)run, run->io.data_offset);
            }
            else if (run->io.port == 0x92)
            {
                a20_handle(run->io.direction, run->io.size, run->io.port, run->io.count, (uint8_t *)run, run->io.data_offset);
            }
            else if (run->io.port == 0x402 && run->io.direction == KVM_EXIT_IO_OUT)
            {
                int wrote = fwrite((uint8_t *)(run) + run->io.data_offset, 1, run->io.count, log);
                if (wrote != run->io.count)
                    err(1, "Failed to write to log file");
            }
            else if(run->io.port == 0xcf8 || run->io.port == 0xcfc)
            {
                pci_handle(run->io.direction, run->io.size, run->io.port, run->io.count, (uint8_t *)run, run->io.data_offset);
            }
            else
            {
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
                    return 0;
                }
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
}

int main(int argc, char *argv[])
{
    gui_init();

    if (argc != 2)
    {
        errx(1, "Usage: %s <filename>", argv[0]);
    }

    int kvm, ret, vm, vcpu;
    struct kvm_run *run;

    uint8_t *buf = init_kvm(&kvm, &vm, &vcpu, &run, argv[1]);
    run_kvm(vcpu, run);

    free(buf);
    close(kvm);
    return 0;
}