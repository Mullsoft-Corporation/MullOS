; SPDX-License-Identifier: GPL-3.0-only
; MullOS Bootloader Entry Point
; Owner: Athaya Alfarizki

.set ALIGN,    1<<0             /* Align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* Provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* Multiboot flag combination */
.set MAGIC,    0x1BADB002       /* Multiboot magic number */
.set CHECKSUM, -(MAGIC + FLAGS) /* Checksum required by multiboot */

/* Multiboot Header for Bootloaders (GRUB/QEMU) */
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/* Allocate Stack Space for MullOS */
.section .bss
.align 16
mullos_stack_bottom:
.skip 16384 # 16 KiB Stack
mullos_stack_top:

/* Kernel Entry Point */
.section .text
.global _mullos_start
.type _mullos_start, @function

_mullos_start:
	/* Setup Stack Pointer */
	mov $mullos_stack_top, %esp

	/* Call MullOS Main Core Function */
	call mullos_main

	/* Infinite Halt Loop if Kernel Exits */
	cli
1:	hlt
	jmp 1b

.size _mullos_start, . - _mullos_start
