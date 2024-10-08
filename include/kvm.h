#ifndef KVM_H
#define KVM_H

#include <linux/kvm.h>
#include <stdint.h>
#include <stdbool.h>

// int kvm_open();
// void kvm_verify_version(int kvm);
// int kvm_create_vm(int kvm);
void kvm_set_userspace_memory_region(struct kvm_userspace_memory_region *memory_region);
// int kvm_create_vcpu();
struct kvm_run* kvm_map_run();
void kvm_print_regs();
void kvm_get_regs(struct kvm_regs *regs);
void kvm_set_regs(struct kvm_regs *regs);
void kvm_get_sregs(struct kvm_sregs *sregs);
void kvm_set_sregs(struct kvm_sregs* sregs);
void kvm_init(char *file_name);
void kvm_run();
void kvm_deinit();
void kvm_pause_vcpu();
void kvm_interrupt(uint32_t vector);
bool kvm_is_interrupts_enabled();

#endif