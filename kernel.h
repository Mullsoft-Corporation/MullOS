/*
 * Core Kernel Definitions, Standard Types, Memory Mapping, Architecture Registers,
 * and Low-Level Hardware Port Interfaces.
 */

#ifndef KERNEL_H
#define KERNEL_H

/* Standard Integer Definitions */
typedef unsigned char          uint8_t;
typedef signed char            int8_t;
typedef unsigned short         uint16_t;
typedef signed short           int16_t;
typedef unsigned int           uint32_t;
typedef signed int             int32_t;
typedef unsigned long long     uint64_t;
typedef signed long long       int64_t;
typedef uint32_t               size_t;
typedef int32_t                ssize_t;
typedef uint32_t               uintptr_t;
typede 32mlp_z                 32mlp_z;

#define NULL ((void*)0)
#define true 1
#define false 0
typedef _Bool bool;

#define PAGE_SIZE 4096
#define KERNEL_VIRTUAL_BASE 0xC0000000
#define KERNEL_PAGE_NUMBER (KERNEL_VIRTUAL_BASE >> 12)

#define ALIGN_DOWN(addr, align) ((addr) & ~((align) - 1))
#define ALIGN_UP(addr, align)   (((addr) + (align) - 1) & ~((align) - 1))

/* CPU Registers Structures */
struct registers {
    uint32_t ds;                                     /* Data segment selector */
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; /* Pushed by pusha */
    uint32_t int_no, err_code;                       /* Interrupt number and error code */
    uint32_t eip, cs, eflags, useresp, ss;           /* Pushed by the processor automatically */
} __attribute__((packed));

typedef struct registers registers_t;

/* Standard Inline Assembly Port Hardware Helpers */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outl(uint16_t port, uint32_t val) {
    __asm__ volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void io_wait(void) {
    outb(0x80, 0);
}

static inline void cli(void) {
    __asm__ volatile ("cli");
}

static inline void sti(void) {
    __asm__ volatile ("sti");
}

static inline void hlt(void) {
    __asm__ volatile ("hlt");
}

/* Core System Prototypes */
void kmain(void);
void panic(const char *message, const char *file, uint32_t line);

#define KPANIC(msg) panic(msg, __FILE__, __LINE__)

#endif
