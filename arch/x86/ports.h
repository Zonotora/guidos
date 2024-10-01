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
static inline uint8_t port_byte_in(uint16_t port) {
  uint8_t result;

  __asm__("in %%dx, %%al" : "=a"(result) : "d"(port));
  return result;
}

static inline void port_byte_out(uint16_t port, uint16_t data) {
  /* Notice how here both registers are mapped to C variables and
   * nothing is returned, thus, no equals '=' in the asm syntax
   * However we see a comma since there are two variables in the input area
   * and none in the 'return' area
   */
  __asm__("out %%al, %%dx" : : "a"(data), "d"(port));
}

static inline uint16_t port_word_in(uint16_t port) {
  uint16_t result;
  __asm__("in %%dx, %%ax" : "=a"(result) : "d"(port));
  return result;
}

static inline void insw(uint16_t port, void *addr, size_t cnt) {
  asm volatile("rep insw" : "+D"(addr), "+c"(cnt) : "d"(port) : "memory");
}

static inline void port_word_out(uint16_t port, uint16_t data) { __asm__("out %%ax, %%dx" : : "a"(data), "d"(port)); }

#endif