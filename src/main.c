#include <linux/kvm.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <err.h>
#include <elf.h>
#include "kvm.h"

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
           "r14 = 0x%llx, r15 = 0x%llx",
           regs.rip, regs.rsp, regs.rflags,
           regs.rax, regs.rbx, regs.rcx, regs.rdx,
           regs.rsi, regs.rdi, regs.r8, regs.r9,
           regs.r10, regs.r11, regs.r12, regs.r13,
           regs.r14, regs.r15);
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

    uint8_t* code;
    size_t size;
    read_file(argv[1], &code, &size);
    size_t size_aligned = (size + 4096 - 1) & ~(4096 - 1);
    
    void* mem = mmap(0, size_aligned, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED)
    {
        err(1, "Failed to map memory");
    }
    struct kvm_userspace_memory_region region = {
        .slot = 0,
        .flags = 0,
        .guest_phys_addr = 0,
        .memory_size = size_aligned,
        .userspace_addr = mem,
    };
    kvm_set_userspace_memory_region(vm, &region);


    vcpu = kvm_create_vcpu(vm);
    run = kvm_map_run(kvm, vcpu);


    

    // struct kvm_regs regs;
    // ret = ioctl(vcpu, KVM_GET_REGS, &regs);
    // if (ret < 0)
    // {
    //     err(1, "Failed to get registers");
    // }
    // regs.rip = 0x7c00;
    // // regs.rsp = 0x7c00;
    // regs.rflags = 2;
    // ret = ioctl(vcpu, KVM_SET_REGS, &regs);
    // if (ret < 0)
    // {
    //     err(1, "Failed to set registers");
    // }

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
    kvm_set_sregs(vcpu, &sregs);

    struct kvm_regs regs;
    kvm_get_regs(vcpu, &regs);
    regs.rip = 0x7c00;
    // regs.rsp = 0x7c00;
    regs.rflags = 2;
    kvm_set_regs(vcpu, &regs);
    

    


    while (1)
    {
        kvm_run(vcpu);

        switch (run->exit_reason)
        {
        case KVM_EXIT_HLT:
            puts("KVM_EXIT_HLT");
            // return 0;
            break;
        case KVM_EXIT_IO:
            if (run->io.direction == KVM_EXIT_IO_OUT && run->io.size == 1 && run->io.port == 0x3f8 && run->io.count == 1)
            {
                printf("%c", (*(((char *)run) + run->io.data_offset)));
                fflush(stdout);
            }
            else
            {
                errx(1, "unhandled KVM_EXIT_IO: "
                        "direction = 0x%x, size = 0x%x, port = 0x%x, count = 0x%x\n",
                     run->io.direction, run->io.size, run->io.port, run->io.count);
                print_regs(vcpu);
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
            errx(1, "KVM_EXIT_INTERNAL_ERROR: suberror = 0x%x", run->internal.suberror);
        default:
            errx(1, "exit_reason = 0x%x", run->exit_reason);
        }
    }

    close(kvm);
    return 0;
}