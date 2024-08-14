#ifndef KVM_H
#define KVM_H

#include <linux/kvm.h>

int kvm_open();
void kvm_verify_version(int kvm);
int kvm_create_vm(int kvm);
void kvm_set_userspace_memory_region(int vm, struct kvm_userspace_memory_region *memory_region);
int kvm_create_vcpu(int vm);
struct kvm_run* kvm_map_run(int kvm, int vcpu);
void kvm_get_regs(int vcpu, struct kvm_regs *regs);
void kvm_set_regs(int vcpu, struct kvm_regs *regs);
void kvm_get_sregs(int vcpu, struct kvm_sregs *sregs);
void kvm_set_sregs(int vcpu, struct kvm_sregs* sregs);
void kvm_run(int vcpu);

#endif