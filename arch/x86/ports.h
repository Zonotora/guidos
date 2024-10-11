#ifndef PORTS_H
#define PORTS_H

#include <stddef.h>
#include <stdint.h>

//  INS/INSB/INSW/INSD — Input from Port to String
//  INS m16, DX        Input word from I/O port specified in DX into memory location
//                     specified in ES:(E)DI or RDI.

//  REP/REPE/REPZ/REPNE/REPNZ — Repeat String Operation Prefix
//  REP INS m16, DX    Input (E)CX words from port DX into ES:[(E)DI]

// Inline assembler syntax
// !! Notice how the source and destination registers are switched from NASM
// !!
//
// '"=a" (result)'; set '=' the C variable '(result)' to the value of
// register e'a'x
// '"d" (port)': map the C variable '(port)' into e'd'x register
//
// Inputs and outputs are separated by colons
//

/**
 * Read a byte from the specified port
 */
static inline uint8_t inb(uint16_t port) {
  uint8_t result;

  __asm__("in %%dx, %%al" : "=a"(result) : "d"(port));
  return result;
}

static inline uint16_t inw(uint16_t port) {
  uint16_t result;
  __asm__("in %%dx, %%ax" : "=a"(result) : "d"(port));
  return result;
}

static inline uint32_t inl(uint16_t port) {
  uint32_t result;
  __asm__("in %%edx, %%eax" : "=a"(result) : "d"(port));
  return result;
}

static inline void insb(uint16_t port, void *addr, size_t cnt) {
  asm volatile("rep insb" : "+D"(addr), "+c"(cnt) : "d"(port) : "memory");
}

static inline void insw(uint16_t port, void *addr, size_t cnt) {
  asm volatile("rep insw" : "+D"(addr), "+c"(cnt) : "d"(port) : "memory");
}

static inline void insl(uint16_t port, void *addr, size_t cnt) {
  asm volatile("rep insl" : "+D"(addr), "+c"(cnt) : "d"(port) : "memory");
}

static inline void outb(uint16_t port, uint8_t data) { asm volatile("outb %%al, %%dx" : : "a"(data), "d"(port)); }

static inline void outw(uint16_t port, uint16_t data) { asm volatile("outw %%ax, %%dx" : : "a"(data), "d"(port)); }

static inline void outl(uint16_t port, uint32_t data) { asm volatile("outl %%eax, %%edx" : : "a"(data), "d"(port)); }

static inline void outsb(uint16_t port, void *addr, size_t cnt) {
  asm volatile("rep outsb" : "+S"(addr), "+c"(cnt) : "d"(port));
}

static inline void outsw(uint16_t port, void *addr, size_t cnt) {
  asm volatile("rep outsw" : "+S"(addr), "+c"(cnt) : "d"(port));
}

static inline void outsl(uint16_t port, void *addr, size_t cnt) {
  asm volatile("rep outsl" : "+S"(addr), "+c"(cnt) : "d"(port));
}

#endif