#ifndef EMULATORS_COMMON_H
#define EMULATORS_COMMON_H

#include <linux/kvm.h>
#include <stdint.h>

#define GET_BITS(value, start, length) (((value) >> (start)) & ((1U << (length)) - 1))

#endif