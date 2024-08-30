#include "kvm.h"
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <err.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "gui.h"
#include "io_manager.h"

int kvm, vm, vcpu;
struct kvm_run *run;

#pragma region KVM

void kvm_open()
{
    kvm = open("/dev/kvm", O_RDWR | __O_CLOEXEC);
    if (kvm < 0)
    {
        err(1, "Failed to open the KVM handle");
    }
}

void kvm_verify_version()
{
    int ret = ioctl(kvm, KVM_GET_API_VERSION, 0);
    if (ret < 12)
    {
        err(1, "KVM_GET_API_VERSION");
        exit(1);
    }

    printf("KVM API version: %d\n", ret);
}

void kvm_create_vm()
{
    vm = ioctl(kvm, KVM_CREATE_VM, 0);
    if (vm < 0)
    {
        err(1, "KVM_CREATE_VM");
    }
}

#pragma endregion

#pragma region VM

void kvm_create_vcpu()
{
    vcpu = ioctl(vm, KVM_CREATE_VCPU, 0);
    if (vcpu < 0)
    {
        err(1, "KVM_CREATE_VCPU");
    }
}

void kvm_set_userspace_memory_region(struct kvm_userspace_memory_region *memory_region)
{
    if (ioctl(vm, KVM_SET_USER_MEMORY_REGION, memory_region) < 0)
    {
        err(1, "KVM_SET_USER_MEMORY_REGION");
    }
}

#pragma endregion

#pragma region VCPU

struct kvm_run *kvm_map_run()
{
    int ret = ioctl(kvm, KVM_GET_VCPU_MMAP_SIZE, 0);
    if (ret < 0)
    {
        err(1, "KVM_GET_VCPU_MMAP_SIZE");
    }

    struct kvm_run *run = mmap(NULL, ret, PROT_READ | PROT_WRITE, MAP_SHARED, vcpu, 0);
    if (run == MAP_FAILED)
    {
        err(1, "Failed to map run");
    }
    return run;
}

void kvm_get_regs(struct kvm_regs *regs)
{
    if (ioctl(vcpu, KVM_GET_REGS, regs) < 0)
    {
        err(1, "KVM_GET_REGS");
    }
}

void kvm_set_regs(struct kvm_regs *regs)
{
    if (ioctl(vcpu, KVM_SET_REGS, regs) < 0)
    {
        err(1, "KVM_GET_REGS");
    }
}

void kvm_get_sregs(struct kvm_sregs *sregs)
{
    if (ioctl(vcpu, KVM_GET_SREGS, sregs) == -1)
    {
        err(1, "KVM_GET_SREGS");
    }
}

void kvm_set_sregs(struct kvm_sregs *sregs)
{
    if (ioctl(vcpu, KVM_SET_SREGS, sregs) < 0)
    {
        err(1, "KVM_SET_SREGS");
    }
}

void print_regs()
{
    struct kvm_regs regs;
    kvm_get_regs(&regs);
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

void print_sregs()
{
    struct kvm_sregs sregs;
    kvm_get_sregs(&sregs);
    printf("sregs:\n");
    printf("  cs: 0x%x\n", sregs.cs.base);
    printf("  ds: 0x%x\n", sregs.ds.base);
    printf("  es: 0x%x\n", sregs.es.base);
    printf("  fs: 0x%x\n", sregs.fs.base);
    printf("  gs: 0x%x\n", sregs.gs.base);
    printf("  ss: 0x%x\n", sregs.ss.base);
    printf("  tr: 0x%x\n", sregs.tr.base);
}

void kvm_run()
{
    struct kvm_regs regs;
    int i = 0;
    char *sig = "QEMO";
    while (1)
    {
        if (ioctl(vcpu, KVM_RUN, 0) < 0)
        {
            err(1, "Failed to run");
        }
        // sleep(3);

        // print_regs();
        // print_sregs();
        kvm_get_regs(&regs);
        printf("0x%llx: ", regs.rip);
        switch (run->exit_reason)
        {
        case KVM_EXIT_HLT:
            puts("KVM_EXIT_HLT");
            // return 0;
            break;
        case KVM_EXIT_IO:
            printf("KVM_EXIT_IO: direction = 0x%x, size = 0x%x, port = 0x%x, count = 0x%x, data = 0x%x\n", run->io.direction, run->io.size, run->io.port, run->io.count, ((uint8_t*)run)[run->io.data_offset]);
            if (0)
            {
                printf("------------------0x%02x:%x------------------\n", run->io.port, ((uint8_t*)run)[run->io.data_offset]);
            }
            else
            {
                io_manager_handle(&run->io, (uint8_t *)run);
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

        gui_render();
    }
}

#pragma endregion

void kvm_init(char *file_name)
{
    kvm_open();
    kvm_verify_version();

    kvm_create_vm();

    uint8_t *buf;
    size_t size;
    read_file(file_name, &buf, &size);

    uint8_t *guest_memory_low = mmap(0, 0x100000, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (guest_memory_low == MAP_FAILED)
        err(1, "Failed to map guest memory");
    memcpy(guest_memory_low + 0x100000 - size, buf, size);

    struct kvm_userspace_memory_region memory_region = {
        .slot = 0,
        .guest_phys_addr = 0,
        .memory_size = 0x100000,
        .userspace_addr = (uint64_t)guest_memory_low,
        .flags = 0};
    kvm_set_userspace_memory_region(&memory_region);

    uint8_t *bios_high = mmap(0xfff00000, 0x100000, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (bios_high == MAP_FAILED)
        err(1, "Failed to map guest memory");
    memcpy(bios_high + 0x100000 - size, buf, size);

    struct kvm_userspace_memory_region bios_region = {
        .slot = 1,
        .guest_phys_addr = 0xfff00000,
        .memory_size = 0x100000,
        .userspace_addr = (uint64_t)bios_high,
        .flags = 0};
    kvm_set_userspace_memory_region(&bios_region);

    free(buf);

    kvm_create_vcpu();
    run = kvm_map_run();

    struct kvm_sregs sregs;
    kvm_get_sregs(&sregs);
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
    kvm_set_sregs(&sregs);

    struct kvm_regs regs;
    kvm_get_regs(&regs);
    regs.rip = 0xFFF0;
    regs.rflags = 0x2;
    kvm_set_regs(&regs);
}

void kvm_deinit()
{
    munmap(run, sizeof(struct kvm_run));
    close(vcpu);
    close(vm);
    close(kvm);
}

void kvm_pause_vcpu()
{
    struct kvm_debugregs debugregs;
    if (ioctl(vcpu, KVM_GET_DEBUGREGS, &debugregs) < 0)
    {
        err(1, "KVM_GET_DEBUGREGS");
    }
    debugregs.db[0] = 0x0;
    debugregs.dr7 |= 0x1;
    if (ioctl(vcpu, KVM_SET_DEBUGREGS, &debugregs) < 0)
    {
        err(1, "KVM_SET_DEBUGREGS");
    }
}