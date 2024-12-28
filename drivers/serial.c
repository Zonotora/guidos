#include "arch/x86/ports.h"
#include "libc/printf.h"

// COM1  0x3F8
// COM2  0x2F8
// COM3  0x3E8
// COM4  0x2E8
// COM5  0x5F8
// COM6  0x4F8
// COM7  0x5E8
// COM8  0x4E8
#define COM1 0x3f8

// IO Port Offset  Setting of DLAB  I/O Access  Register mapped to this port
// +0  0  Read  Receive buffer.
// +0  0  Write  Transmit buffer.
// +1  0  Read/Write  Interrupt Enable Register.
// +0  1  Read/Write  With DLAB set to 1, this is the least significant byte of the divisor value
//          for setting the baud rate.
// +1  1  Read/Write  With DLAB set to 1, this is the most significant byte of the divisor value.
// +2  -  Read  Interrupt Identification
// +2  -  Write  FIFO control registers
// +3  -  Read/Write  Line Control Register. The most significant bit of this register is the DLAB.
// +4  -  Read/Write  Modem Control Register.
// +5  -  Read  Line Status Register.
// +6  -  Read  Modem Status Register.
// +7  -  Read/Write  Scratch Register.
#define PORT_READ_WRITE(PORT) (PORT + 0)
#define PORT_DLAB_LO(PORT) (PORT + 0)
// To communicate with a serial port in interrupt mode, the interrupt-enable-register (see table above) must be set
// correctly. To determine which interrupts should be enabled, a value with the following bits (0 = disabled, 1 =
// enabled) must be written to the interrupt-enable-register:
// Bit 7-4   Bit 3         Bit 2                 Bit 1                               Bit 0
// Reserved  Modem Status  Receiver Line Status  Transmitter Holding Register Empty  Received Data Available
#define PORT_INTERRUPT_ENABLE(PORT) (PORT + 1)
#define PORT_DLAB_HI(PORT) (PORT + 1)
#define PORT_READ_INTERRUPT_ID(PORT) (PORT + 2)
// The First In / First Out Control Register (FCR) is for controlling the FIFO buffers. Access this register by writing
// to port offset +2.
//
// Bit 2 being set clears the Transmit FIFO buffer while Bit 1 being set clears the Receive FIFO buffer. Both bits will
// set themselves back to 0 after they are done being cleared.
//
// The Interrupt Trigger Level is used to configure how much data must be received in the FIFO Receive buffer before
// triggering a Received Data Available Interrupt.
//
// Bits 7-6                 Bits 5-4  Bit 3            Bit 2                Bit 1               Bit 0
// Interrupt Trigger Level  Reserved  DMA Mode Select  Clear Transmit FIFO  Clear Receive FIFO  Enable FIFO's
#define PORT_WRITE_FIFO_CTRL(PORT) (PORT + 2)
// Line Control Register
// Bit 7                     Bit 6             Bits 5-3     Bit 2      Bits 1-0
// Divisor Latch Access Bit  Break Enable Bit  Parity Bits  Stop Bits  Data Bits
//
// The number of bits in a character is variable. Having fewer bits is, of course, faster, but they store less
// information. If you are only sending ASCII text, you probably only need 7 bits.
//
// Set this value by writing to the two least significant bits of the Line Control Register [PORT + 3].
// Bit 1  Bit 0  Character Length (bits)
// 0      0       5
// 0      1       6
// 1      0       7
// 1      1       8
//
// The serial controller can be configured to send a number of bits after each character of data. These reliable bits
// can be used to by the controller to verify that the sending and receiving devices are in phase.
//
// If the character length is specifically 5 bits, the stop bits can only be set to 1 or 1.5. For other character
// lengths, the stop bits can only be set to 1 or 2.
//
// To set the number of stop bits, set bit 2 of the Line Control Register [PORT + 3].
// Bit 2  Stop bits
// 0      1
// 1      1.5 / 2 (depending on character length)
//
// The parity type can be NONE, EVEN, ODD, MARK or SPACE.
#define PORT_LINE_CTRL(PORT) (PORT + 3)
// The Modem Control Register is one half of the hardware handshaking registers. While most serial devices no longer use
// hardware handshaking, The lines are still included in all 16550 compatible UARTS. These can be used as general
// purpose output ports, or to actually perform handshaking. By writing to the Modem Control Register, it will set those
// lines active.
//
// Modem Control Register
// 0  Data Terminal Ready (DTR)  Controls the Data Terminal Ready Pin
// 1  Request to Send (RTS)      Controls the Request to Send Pin
// 2  Out 1                      Controls a hardware pin (OUT1) which is unused in PC implementations
// 3  Out 2                      Controls a hardware pin (OUT2) which is used to enable the IRQ in PC implementations
// 4  Loop                       Provides a local loopback feature for diagnostic testing of the UART
// 5  0                          Unused
// 6  0                          Unused
// 7  0                          Unused
#define PORT_MODEM_CTRL(PORT) (PORT + 4)
#define PORT_READ_LINE_STATUS(PORT) (PORT + 5)
// This register provides the current state of the control lines from a peripheral device. In addition to this
// current-state information, four bits of the MODEM Status Register provide change information. These bits are set to a
// logic 1 whenever a control input from the MODEM changes state. They are reset to logic 0 whenever the CPU reads the
//
// Modem Status Register
// 0  Delta Clear to Send (DCTS)              Indicates that CTS input has changed state since the last time it was read
// 1  Delta Data Set Ready (DDSR)             Indicates that DSR input has changed state since the last time it was read
// 2  Trailing Edge of Ring Indicator (TERI)  Indicates that RI input to the chip has changed from a low to a high state
// 3  Delta Data Carrier Detect (DDCD)        Indicates that DCD input has changed state since the last time it were
//                                            read
// 4  Clear to Send (CTS)                     Inverted CTS Signal
// 5  Data Set Ready (DSR)                    Inverted DSR Signal
// 6  Ring Indicator (RI)                     Inverted RI Signal 7  Data Carrier Detect (DCD)  Inverted DCD Signal
#define PORT_READ_MODEM_STATUS(PORT) (PORT + 6)
#define PORT_SCRATCH(PORT) (PORT + 7)

// Line Control Register
#define LINE_CTRL_DLAB 0x80

// Line Status Register
// 0  Data ready (DR)                            Set if there is data that can be read
// 1  Overrun error (OE)                         Set if there has been data lost
// 2  Parity error (PE)                          Set if there was an error in the transmission as detected by parity
// 3  Framing error (FE)                         Set if a stop bit was missing
// 4  Break indicator (BI)                       Set if there is a break in data input
// 5  Transmitter holding register empty (THRE)  Set if the transmission buffer is empty (i.e. data can be sent)
// 6  Transmitter empty (TEMT)                   Set if the transmitter is not doing anything
// 7  Impending Error                            Set if there is an error with a word in the input buffer
#define LINE_STATUS_DR 0x01
#define LINE_STATUS_OE 0x02
#define LINE_STATUS_PE 0x04
#define LINE_STATUS_FE 0x08
#define LINE_STATUS_BI 0x10
#define LINE_STATUS_THRE 0x20
#define LINE_STATUS_TEMP 0x40
#define LINE_STATUS_ERR 0x80

// The serial controller (UART) has an internal clock which runs at 115200 ticks per second and a clock divisor which is
// used to control the baud rate. This is exactly the same type of system used by the Programmable Interrupt Timer
// (PIT).
//
// In order to set the speed of the port, calculate the divisor required for the given baud rate and program that in to
// the divisor register. For example, a divisor of 1 will give 115200 baud, a divisor of 2 will give 57600 baud, 3 will
// give 38400 baud, etc.

// To set the divisor to the controller:
//  1. Set the most significant bit of the Line Control Register. This is
//  the DLAB bit, and allows access to the divisor registers.
//  2. Send the least significant byte of the divisor value to [PORT + 0].
//  3. Send the most significant byte of the divisor value to [PORT + 1].
//  4. Clear the most significant bit of the Line Control Register.

int serial_init() {
  outb(PORT_INTERRUPT_ENABLE(COM1), 0x00);    // Disable all interrupts
  outb(PORT_LINE_CTRL(COM1), LINE_CTRL_DLAB); // Enable DLAB (set baud rate divisor)
  outb(PORT_DLAB_LO(COM1), 0x03);             // Set divisor to 3 (lo byte) 38400 baud
  outb(PORT_DLAB_HI(COM1), 0x00);             //                  (hi byte)
  outb(PORT_LINE_CTRL(COM1), 0x03);           // 8 bits, no parity, one stop bit
  outb(PORT_WRITE_FIFO_CTRL(COM1), 0xc7);     // Enable FIFO, clear them, with 14-byte threshold
  outb(PORT_MODEM_CTRL(COM1), 0x0b);          // IRQs enabled, RTS/DSR set
  outb(PORT_MODEM_CTRL(COM1), 0x1e);          // Set in loopback mode, test the serial chip
  outb(PORT_READ_WRITE(COM1), 0xae);          // Test serial chip (send byte 0xAE and check if serial returns same byte)

  // Check if serial is faulty (i.e: not same byte as sent)
  if (inb(PORT_READ_WRITE(COM1)) != 0xae) {
    return 1;
  }

  // If serial is not faulty set it in normal operation mode
  // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
  outb(PORT_MODEM_CTRL(COM1), 0x0f);
  return 0;
}

int serial_received() { return inb(PORT_READ_LINE_STATUS(COM1)) & LINE_STATUS_DR; }

char serial_read_byte() {
  while (serial_received() == 0)
    ;

  return inb(PORT_READ_WRITE(COM1));
}

int is_transmit_empty() { return inb(PORT_READ_LINE_STATUS(COM1)) & LINE_STATUS_THRE; }

void serial_putchar(char c) {
  while (is_transmit_empty() == 0)
    ;

  outb(PORT_READ_WRITE(COM1), c);
}

void serial_printf(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vprintf(serial_putchar, format, args);
  va_end(args);
}