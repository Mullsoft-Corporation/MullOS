; SPDX-License-Identifier: GPL-3.0-only
; MullOS:0.1 - Early Bootstrap for x86_64

MULTIBOOT_MAGIC    equ 0x1BADB002
MULTIBOOT_FLAGS    equ 0x00000003
MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

section .multiboot
align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384 ; Alokasi stack 16 KB
stack_top:

section .text
global _start
extern mullos_main

_start:
    mov esp, stack_top
    call mullos_main
    cli
.hang:
    hlt
    jmp .hang
