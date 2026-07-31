; ==============================================================================
; boot.asm - Entry point for MullOS (x86 32-bit)
; ==============================================================================

BITS 32

; Multiboot constants
MB_ALIGN     equ 1 << 0
MB_MEMINFO   equ 1 << 1
MB_FLAGS     equ MB_ALIGN | MB_MEMINFO
MB_MAGIC     equ 0x1BADB002
MB_CHECKSUM  equ -(MB_MAGIC + MB_FLAGS)

section .multiboot
align 4
    dd MB_MAGIC
    dd MB_FLAGS
    dd MB_CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384                  ; 16 KB stack space
stack_top:

section .text
global _start
extern kmain

_start:
    ; Setup stack
    mov esp, stack_top

    ; Clear flags
    push 0
    popf

    ; Pass multiboot parameters (EBX = info struct, EAX = magic)
    push ebx
    push eax

    call kmain

    ; If kmain returns, halt CPU
    cli
.loop:
    hlt
    jmp .loop
; 67 Aku Nak Mie Ayam😋
