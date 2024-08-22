	.file	"asm-offsets.c"
	.code16gcc
	.text
.Ltext0:
	.file 0 "/home/byter/documents/kvm-new/seabios" "src/asm-offsets.c"
	.section	.text.foo,"ax",@progbits
	.globl	foo
	.type	foo, @function
foo:
.LFB51:
	.file 1 "src/asm-offsets.c"
	.loc 1 10 1 view -0
	.cfi_startproc
	.loc 1 11 5 view .LVU1
#APP
# 11 "src/asm-offsets.c" 1
	
->#BREGS
# 0 "" 2
	.loc 1 12 5 view .LVU2
# 12 "src/asm-offsets.c" 1
	
->BREGS_es $2 offsetof(struct bregs, es)
# 0 "" 2
	.loc 1 13 5 view .LVU3
# 13 "src/asm-offsets.c" 1
	
->BREGS_ds $0 offsetof(struct bregs, ds)
# 0 "" 2
	.loc 1 14 5 view .LVU4
# 14 "src/asm-offsets.c" 1
	
->BREGS_eax $28 offsetof(struct bregs, eax)
# 0 "" 2
	.loc 1 15 5 view .LVU5
# 15 "src/asm-offsets.c" 1
	
->BREGS_ebx $16 offsetof(struct bregs, ebx)
# 0 "" 2
	.loc 1 16 5 view .LVU6
# 16 "src/asm-offsets.c" 1
	
->BREGS_ecx $24 offsetof(struct bregs, ecx)
# 0 "" 2
	.loc 1 17 5 view .LVU7
# 17 "src/asm-offsets.c" 1
	
->BREGS_edx $20 offsetof(struct bregs, edx)
# 0 "" 2
	.loc 1 18 5 view .LVU8
# 18 "src/asm-offsets.c" 1
	
->BREGS_ebp $12 offsetof(struct bregs, ebp)
# 0 "" 2
	.loc 1 19 5 view .LVU9
# 19 "src/asm-offsets.c" 1
	
->BREGS_esi $8 offsetof(struct bregs, esi)
# 0 "" 2
	.loc 1 20 5 view .LVU10
# 20 "src/asm-offsets.c" 1
	
->BREGS_edi $4 offsetof(struct bregs, edi)
# 0 "" 2
	.loc 1 21 5 view .LVU11
# 21 "src/asm-offsets.c" 1
	
->BREGS_flags $36 offsetof(struct bregs, flags)
# 0 "" 2
	.loc 1 22 5 view .LVU12
# 22 "src/asm-offsets.c" 1
	
->BREGS_code $32 offsetof(struct bregs, code)
# 0 "" 2
	.loc 1 23 1 is_stmt 0 view .LVU13
#NO_APP
	ret
	.cfi_endproc
.LFE51:
	.size	foo, .-foo
	.text
.Letext0:
	.file 2 "src/types.h"
	.file 3 "src/bregs.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.long	0x4a6
	.value	0x5
	.byte	0x1
	.byte	0x4
	.long	.Ldebug_abbrev0
	.uleb128 0xb
	.long	.LASF43
	.byte	0x1d
	.long	.LASF0
	.long	.LASF1
	.long	.LLRL0
	.long	0
	.long	.Ldebug_line0
	.uleb128 0x9
	.string	"u8"
	.byte	0x9
	.byte	0x17
	.long	0x30
	.uleb128 0x7
	.byte	0x1
	.byte	0x8
	.long	.LASF2
	.uleb128 0x7
	.byte	0x1
	.byte	0x6
	.long	.LASF3
	.uleb128 0x9
	.string	"u16"
	.byte	0xb
	.byte	0x18
	.long	0x49
	.uleb128 0x7
	.byte	0x2
	.byte	0x7
	.long	.LASF4
	.uleb128 0x7
	.byte	0x2
	.byte	0x5
	.long	.LASF5
	.uleb128 0x9
	.string	"u32"
	.byte	0xd
	.byte	0x16
	.long	0x62
	.uleb128 0x7
	.byte	0x4
	.byte	0x7
	.long	.LASF6
	.uleb128 0xc
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x7
	.byte	0x8
	.byte	0x7
	.long	.LASF7
	.uleb128 0x7
	.byte	0x8
	.byte	0x5
	.long	.LASF8
	.uleb128 0xd
	.long	.LASF9
	.byte	0x2
	.byte	0x11
	.byte	0xd
	.long	0x57
	.uleb128 0x3
	.byte	0x2
	.byte	0x1b
	.byte	0x9
	.long	0xad
	.uleb128 0x1
	.long	.LASF10
	.byte	0x2
	.byte	0x1c
	.byte	0x11
	.long	0x3e
	.byte	0
	.uleb128 0x2
	.string	"seg"
	.byte	0x2
	.byte	0x1d
	.byte	0x11
	.long	0x3e
	.byte	0x2
	.byte	0
	.uleb128 0x5
	.byte	0x2
	.byte	0x1a
	.long	0xc6
	.uleb128 0x4
	.long	0x8a
	.uleb128 0xe
	.long	.LASF16
	.byte	0x2
	.byte	0x1f
	.byte	0xd
	.long	0x57
	.byte	0
	.uleb128 0xa
	.long	.LASF39
	.byte	0x4
	.byte	0x2
	.byte	0x19
	.long	0xd9
	.uleb128 0x6
	.long	0xad
	.byte	0
	.byte	0
	.uleb128 0x3
	.byte	0x3
	.byte	0x1a
	.byte	0x5
	.long	0xfb
	.uleb128 0x2
	.string	"di"
	.byte	0x3
	.byte	0x1a
	.byte	0x5
	.long	0x3e
	.byte	0
	.uleb128 0x1
	.long	.LASF11
	.byte	0x3
	.byte	0x1a
	.byte	0x5
	.long	0x3e
	.byte	0x2
	.byte	0
	.uleb128 0x3
	.byte	0x3
	.byte	0x1a
	.byte	0x5
	.long	0x138
	.uleb128 0x1
	.long	.LASF12
	.byte	0x3
	.byte	0x1a
	.byte	0x5
	.long	0x26
	.byte	0
	.uleb128 0x1
	.long	.LASF13
	.byte	0x3
	.byte	0x1a
	.byte	0x5
	.long	0x26
	.byte	0x1
	.uleb128 0x1
	.long	.LASF14
	.byte	0x3
	.byte	0x1a
	.byte	0x5
	.long	0x26
	.byte	0x2
	.uleb128 0x1
	.long	.LASF15
	.byte	0x3
	.byte	0x1a
	.byte	0x5
	.long	0x26
	.byte	0x3
	.byte	0
	.uleb128 0x5
	.byte	0x3
	.byte	0x1a
	.long	0x154
	.uleb128 0x8
	.string	"edi"
	.byte	0x1a
	.long	0x57
	.uleb128 0x4
	.long	0xd9
	.uleb128 0x4
	.long	0xfb
	.byte	0
	.uleb128 0x3
	.byte	0x3
	.byte	0x1b
	.byte	0x5
	.long	0x176
	.uleb128 0x2
	.string	"si"
	.byte	0x3
	.byte	0x1b
	.byte	0x5
	.long	0x3e
	.byte	0
	.uleb128 0x1
	.long	.LASF17
	.byte	0x3
	.byte	0x1b
	.byte	0x5
	.long	0x3e
	.byte	0x2
	.byte	0
	.uleb128 0x3
	.byte	0x3
	.byte	0x1b
	.byte	0x5
	.long	0x1b3
	.uleb128 0x1
	.long	.LASF18
	.byte	0x3
	.byte	0x1b
	.byte	0x5
	.long	0x26
	.byte	0
	.uleb128 0x1
	.long	.LASF19
	.byte	0x3
	.byte	0x1b
	.byte	0x5
	.long	0x26
	.byte	0x1
	.uleb128 0x1
	.long	.LASF20
	.byte	0x3
	.byte	0x1b
	.byte	0x5
	.long	0x26
	.byte	0x2
	.uleb128 0x1
	.long	.LASF21
	.byte	0x3
	.byte	0x1b
	.byte	0x5
	.long	0x26
	.byte	0x3
	.byte	0
	.uleb128 0x5
	.byte	0x3
	.byte	0x1b
	.long	0x1cf
	.uleb128 0x8
	.string	"esi"
	.byte	0x1b
	.long	0x57
	.uleb128 0x4
	.long	0x154
	.uleb128 0x4
	.long	0x176
	.byte	0
	.uleb128 0x3
	.byte	0x3
	.byte	0x1c
	.byte	0x5
	.long	0x1f1
	.uleb128 0x2
	.string	"bp"
	.byte	0x3
	.byte	0x1c
	.byte	0x5
	.long	0x3e
	.byte	0
	.uleb128 0x1
	.long	.LASF22
	.byte	0x3
	.byte	0x1c
	.byte	0x5
	.long	0x3e
	.byte	0x2
	.byte	0
	.uleb128 0x3
	.byte	0x3
	.byte	0x1c
	.byte	0x5
	.long	0x22e
	.uleb128 0x1
	.long	.LASF23
	.byte	0x3
	.byte	0x1c
	.byte	0x5
	.long	0x26
	.byte	0
	.uleb128 0x1
	.long	.LASF24
	.byte	0x3
	.byte	0x1c
	.byte	0x5
	.long	0x26
	.byte	0x1
	.uleb128 0x1
	.long	.LASF25
	.byte	0x3
	.byte	0x1c
	.byte	0x5
	.long	0x26
	.byte	0x2
	.uleb128 0x1
	.long	.LASF26
	.byte	0x3
	.byte	0x1c
	.byte	0x5
	.long	0x26
	.byte	0x3
	.byte	0
	.uleb128 0x5
	.byte	0x3
	.byte	0x1c
	.long	0x24a
	.uleb128 0x8
	.string	"ebp"
	.byte	0x1c
	.long	0x57
	.uleb128 0x4
	.long	0x1cf
	.uleb128 0x4
	.long	0x1f1
	.byte	0
	.uleb128 0x3
	.byte	0x3
	.byte	0x1d
	.byte	0x5
	.long	0x26c
	.uleb128 0x2
	.string	"bx"
	.byte	0x3
	.byte	0x1d
	.byte	0x5
	.long	0x3e
	.byte	0
	.uleb128 0x1
	.long	.LASF27
	.byte	0x3
	.byte	0x1d
	.byte	0x5
	.long	0x3e
	.byte	0x2
	.byte	0
	.uleb128 0x3
	.byte	0x3
	.byte	0x1d
	.byte	0x5
	.long	0x2a7
	.uleb128 0x2
	.string	"bl"
	.byte	0x3
	.byte	0x1d
	.byte	0x5
	.long	0x26
	.byte	0
	.uleb128 0x2
	.string	"bh"
	.byte	0x3
	.byte	0x1d
	.byte	0x5
	.long	0x26
	.byte	0x1
	.uleb128 0x1
	.long	.LASF28
	.byte	0x3
	.byte	0x1d
	.byte	0x5
	.long	0x26
	.byte	0x2
	.uleb128 0x1
	.long	.LASF29
	.byte	0x3
	.byte	0x1d
	.byte	0x5
	.long	0x26
	.byte	0x3
	.byte	0
	.uleb128 0x5
	.byte	0x3
	.byte	0x1d
	.long	0x2c3
	.uleb128 0x8
	.string	"ebx"
	.byte	0x1d
	.long	0x57
	.uleb128 0x4
	.long	0x24a
	.uleb128 0x4
	.long	0x26c
	.byte	0
	.uleb128 0x3
	.byte	0x3
	.byte	0x1e
	.byte	0x5
	.long	0x2e5
	.uleb128 0x2
	.string	"dx"
	.byte	0x3
	.byte	0x1e
	.byte	0x5
	.long	0x3e
	.byte	0
	.uleb128 0x1
	.long	.LASF30
	.byte	0x3
	.byte	0x1e
	.byte	0x5
	.long	0x3e
	.byte	0x2
	.byte	0
	.uleb128 0x3
	.byte	0x3
	.byte	0x1e
	.byte	0x5
	.long	0x320
	.uleb128 0x2
	.string	"dl"
	.byte	0x3
	.byte	0x1e
	.byte	0x5
	.long	0x26
	.byte	0
	.uleb128 0x2
	.string	"dh"
	.byte	0x3
	.byte	0x1e
	.byte	0x5
	.long	0x26
	.byte	0x1
	.uleb128 0x1
	.long	.LASF31
	.byte	0x3
	.byte	0x1e
	.byte	0x5
	.long	0x26
	.byte	0x2
	.uleb128 0x1
	.long	.LASF32
	.byte	0x3
	.byte	0x1e
	.byte	0x5
	.long	0x26
	.byte	0x3
	.byte	0
	.uleb128 0x5
	.byte	0x3
	.byte	0x1e
	.long	0x33c
	.uleb128 0x8
	.string	"edx"
	.byte	0x1e
	.long	0x57
	.uleb128 0x4
	.long	0x2c3
	.uleb128 0x4
	.long	0x2e5
	.byte	0
	.uleb128 0x3
	.byte	0x3
	.byte	0x1f
	.byte	0x5
	.long	0x35e
	.uleb128 0x2
	.string	"cx"
	.byte	0x3
	.byte	0x1f
	.byte	0x5
	.long	0x3e
	.byte	0
	.uleb128 0x1
	.long	.LASF33
	.byte	0x3
	.byte	0x1f
	.byte	0x5
	.long	0x3e
	.byte	0x2
	.byte	0
	.uleb128 0x3
	.byte	0x3
	.byte	0x1f
	.byte	0x5
	.long	0x399
	.uleb128 0x2
	.string	"cl"
	.byte	0x3
	.byte	0x1f
	.byte	0x5
	.long	0x26
	.byte	0
	.uleb128 0x2
	.string	"ch"
	.byte	0x3
	.byte	0x1f
	.byte	0x5
	.long	0x26
	.byte	0x1
	.uleb128 0x1
	.long	.LASF34
	.byte	0x3
	.byte	0x1f
	.byte	0x5
	.long	0x26
	.byte	0x2
	.uleb128 0x1
	.long	.LASF35
	.byte	0x3
	.byte	0x1f
	.byte	0x5
	.long	0x26
	.byte	0x3
	.byte	0
	.uleb128 0x5
	.byte	0x3
	.byte	0x1f
	.long	0x3b5
	.uleb128 0x8
	.string	"ecx"
	.byte	0x1f
	.long	0x57
	.uleb128 0x4
	.long	0x33c
	.uleb128 0x4
	.long	0x35e
	.byte	0
	.uleb128 0x3
	.byte	0x3
	.byte	0x20
	.byte	0x5
	.long	0x3d7
	.uleb128 0x2
	.string	"ax"
	.byte	0x3
	.byte	0x20
	.byte	0x5
	.long	0x3e
	.byte	0
	.uleb128 0x1
	.long	.LASF36
	.byte	0x3
	.byte	0x20
	.byte	0x5
	.long	0x3e
	.byte	0x2
	.byte	0
	.uleb128 0x3
	.byte	0x3
	.byte	0x20
	.byte	0x5
	.long	0x412
	.uleb128 0x2
	.string	"al"
	.byte	0x3
	.byte	0x20
	.byte	0x5
	.long	0x26
	.byte	0
	.uleb128 0x2
	.string	"ah"
	.byte	0x3
	.byte	0x20
	.byte	0x5
	.long	0x26
	.byte	0x1
	.uleb128 0x1
	.long	.LASF37
	.byte	0x3
	.byte	0x20
	.byte	0x5
	.long	0x26
	.byte	0x2
	.uleb128 0x1
	.long	.LASF38
	.byte	0x3
	.byte	0x20
	.byte	0x5
	.long	0x26
	.byte	0x3
	.byte	0
	.uleb128 0x5
	.byte	0x3
	.byte	0x20
	.long	0x42e
	.uleb128 0x8
	.string	"eax"
	.byte	0x20
	.long	0x57
	.uleb128 0x4
	.long	0x3b5
	.uleb128 0x4
	.long	0x3d7
	.byte	0
	.uleb128 0xa
	.long	.LASF40
	.byte	0x26
	.byte	0x3
	.byte	0x17
	.long	0x497
	.uleb128 0x2
	.string	"ds"
	.byte	0x3
	.byte	0x18
	.byte	0x9
	.long	0x3e
	.byte	0
	.uleb128 0x2
	.string	"es"
	.byte	0x3
	.byte	0x19
	.byte	0x9
	.long	0x3e
	.byte	0x2
	.uleb128 0x6
	.long	0x138
	.byte	0x4
	.uleb128 0x6
	.long	0x1b3
	.byte	0x8
	.uleb128 0x6
	.long	0x22e
	.byte	0xc
	.uleb128 0x6
	.long	0x2a7
	.byte	0x10
	.uleb128 0x6
	.long	0x320
	.byte	0x14
	.uleb128 0x6
	.long	0x399
	.byte	0x18
	.uleb128 0x6
	.long	0x412
	.byte	0x1c
	.uleb128 0x1
	.long	.LASF41
	.byte	0x3
	.byte	0x21
	.byte	0x15
	.long	0xc6
	.byte	0x20
	.uleb128 0x1
	.long	.LASF42
	.byte	0x3
	.byte	0x22
	.byte	0x9
	.long	0x3e
	.byte	0x24
	.byte	0
	.uleb128 0xf
	.string	"foo"
	.byte	0x1
	.byte	0x9
	.byte	0x6
	.long	.LFB51
	.long	.LFE51-.LFB51
	.uleb128 0x1
	.byte	0x9c
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x13
	.byte	0x1
	.uleb128 0xb
	.uleb128 0x21
	.sleb128 4
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0xd
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x17
	.byte	0x1
	.uleb128 0xb
	.uleb128 0x21
	.sleb128 4
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0x21
	.sleb128 5
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0xd
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0x21
	.sleb128 3
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0x21
	.sleb128 5
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0x21
	.sleb128 2
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0x21
	.sleb128 8
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x1f
	.uleb128 0x1b
	.uleb128 0x1f
	.uleb128 0x55
	.uleb128 0x17
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x7a
	.uleb128 0x19
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_aranges,"",@progbits
	.long	0x1c
	.value	0x2
	.long	.Ldebug_info0
	.byte	0x4
	.byte	0
	.value	0
	.value	0
	.long	.LFB51
	.long	.LFE51-.LFB51
	.long	0
	.long	0
	.section	.debug_rnglists,"",@progbits
.Ldebug_ranges0:
	.long	.Ldebug_ranges3-.Ldebug_ranges2
.Ldebug_ranges2:
	.value	0x5
	.byte	0x4
	.byte	0
	.long	0
.LLRL0:
	.byte	0x7
	.long	.LFB51
	.uleb128 .LFE51-.LFB51
	.byte	0
.Ldebug_ranges3:
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF20:
	.string	"si_hilo"
.LASF9:
	.string	"size_t"
.LASF16:
	.string	"segoff"
.LASF28:
	.string	"bx_hilo"
.LASF33:
	.string	"cx_hi"
.LASF32:
	.string	"dx_hihi"
.LASF40:
	.string	"bregs"
.LASF34:
	.string	"cx_hilo"
.LASF2:
	.string	"unsigned char"
.LASF39:
	.string	"segoff_s"
.LASF4:
	.string	"short unsigned int"
.LASF37:
	.string	"ax_hilo"
.LASF12:
	.string	"di8l"
.LASF35:
	.string	"cx_hihi"
.LASF41:
	.string	"code"
.LASF15:
	.string	"di_hihi"
.LASF18:
	.string	"si8l"
.LASF13:
	.string	"di8u"
.LASF31:
	.string	"dx_hilo"
.LASF6:
	.string	"unsigned int"
.LASF42:
	.string	"flags"
.LASF26:
	.string	"bp_hihi"
.LASF7:
	.string	"long long unsigned int"
.LASF30:
	.string	"dx_hi"
.LASF19:
	.string	"si8u"
.LASF22:
	.string	"bp_hi"
.LASF14:
	.string	"di_hilo"
.LASF8:
	.string	"long long int"
.LASF17:
	.string	"si_hi"
.LASF25:
	.string	"bp_hilo"
.LASF29:
	.string	"bx_hihi"
.LASF10:
	.string	"offset"
.LASF23:
	.string	"bp8l"
.LASF5:
	.string	"short int"
.LASF21:
	.string	"si_hihi"
.LASF11:
	.string	"di_hi"
.LASF27:
	.string	"bx_hi"
.LASF3:
	.string	"signed char"
.LASF24:
	.string	"bp8u"
.LASF38:
	.string	"ax_hihi"
.LASF43:
	.ascii	"GNU C17 11.4.0 -march=i386 -mregparm=3 -mpreferred-stack-bou"
	.ascii	"ndary=2 -minline-all-stringops -m16 -g -Os -fomit-frame-poin"
	.ascii	"ter -freg-struct-return -ffreestanding -fno-delete-null-poin"
	.ascii	"ter-checks -ffunction-sections -f"
	.string	"data-sections -fno-common -fno-merge-constants -fno-pie -fno-stack-protector -fstack-check=no -fcf-protection=none -fno-defer-pop -fno-jump-tables -fno-tree-switch-conversion --param=large-stack-frame=4 -fasynchronous-unwind-tables -fstack-clash-protection"
.LASF36:
	.string	"ax_hi"
	.section	.debug_line_str,"MS",@progbits,1
.LASF1:
	.string	"/home/byter/documents/kvm-new/seabios"
.LASF0:
	.string	"src/asm-offsets.c"
	.ident	"GCC: (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0"
	.section	.note.GNU-stack,"",@progbits
