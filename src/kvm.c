#include "kvm.h"
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <err.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#pragma region KVM

int kvm_open()
{
    int kvm = open("/dev/kvm", O_RDWR | __O_CLOEXEC);
    if (kvm < 0)
    {
        err(1, "Failed to open the KVM handle");
    }
    return kvm;
}

void kvm_verify_version(int kvm)
{
    int ret = ioctl(kvm, KVM_GET_API_VERSION, 0);
    if (ret < 12)
    {
        err(1, "KVM_GET_API_VERSION");
        exit(1);
    }

    printf("KVM API version: %d\n", ret);
}

int kvm_create_vm(int kvm)
{
    int vm = ioctl(kvm, KVM_CREATE_VM, 0);
    if (vm < 0)
    {
        err(1, "KVM_CREATE_VM");
    }
    return vm;
}

#pragma endregion

#pragma region VM

int kvm_create_vcpu(int vm)
{
    int vcpu = ioctl(vm, KVM_CREATE_VCPU, 0);
    if (vcpu < 0)
    {
        err(1, "KVM_CREATE_VCPU");
    }
    return vcpu;
}

void kvm_set_userspace_memory_region(int vm, struct kvm_userspace_memory_region *memory_region)
{
    if (ioctl(vm, KVM_SET_USER_MEMORY_REGION, memory_region) < 0)
    {
        err(1, "Failed to set guest memory region");
    }
}

#pragma endregion

#pragma region VCPU

struct kvm_run *kvm_map_run(int kvm, int vcpu)
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

void kvm_get_regs(int vcpu, struct kvm_regs *regs)
{
    if(ioctl(vcpu, KVM_GET_REGS, regs) < 0)
    {
        err(1, "KVM_GET_REGS");
    }
}

void kvm_set_regs(int vcpu, struct kvm_regs *regs)
{
    if (ioctl(vcpu, KVM_GET_REGS, regs) < 0)
    {
        err(1, "KVM_GET_REGS");
    }
}

void kvm_get_sregs(int vcpu, struct kvm_sregs *sregs)
{
    if (ioctl(vcpu, KVM_GET_SREGS, sregs) == -1)
    {
        err(1, "KVM_GET_SREGS");
    }
}

void kvm_set_sregs(int vcpu, struct kvm_sregs *sregs)
{
    if (ioctl(vcpu, KVM_SET_SREGS, sregs) < 0)
    {
        err(1, "KVM_SET_SREGS");
    }
}

void kvm_run(int vcpu)
{
    if (ioctl(vcpu, KVM_RUN, 0) < 0)
    {
        err(1, "Failed to run");
    }
}

#pragma endregion