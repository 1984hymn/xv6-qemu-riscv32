#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

void main();
void timerinit();

// entry.S needs one stack per CPU.
__attribute__ ((aligned (16))) char stack0[4096 * NCPU];

// a scratch area per CPU for machine-mode timer interrupts.
uint32 timer_scratch[NCPU][6];

// assembly code in kernelvec.S for machine-mode timer interrupt.
extern void timervec();

// print before we have printf
static inline void uart_putc(char c) {
  volatile unsigned int *uart = (volatile unsigned int *)UART0;
  *uart = c;
}
// handy for checking register
static inline void uart_print_32bits_register(uint32 register32bits)  {
  for (int i = 7; i >= 0; --i) {
    int nib = (register32bits >> (i*4)) & 0xF;
    char ch = nib < 10 ? '0' + nib : 'A' + (nib - 10);
    uart_putc(ch);
  }
}

// entry.S jumps here in machine mode on stack0.
void
start()
{
  // set M Previous Privilege mode to Supervisor, for mret.
  /*
  #define MSTATUS_MPP_MASK (3L << 11) // previous mode.
  #define MSTATUS_MPP_M (3L << 11)
  #define MSTATUS_MPP_S (1L << 11)
  #define MSTATUS_MPP_U (0L << 11)
  #define MSTATUS_MIE (1L << 3)    // machine-mode interrupt enable.
  */
  unsigned long x = r_mstatus();
  x &= ~MSTATUS_MPP_MASK;
  x |= MSTATUS_MPP_S;
  uart_print_32bits_register(x);
  uart_putc('\n');
  w_mstatus(x);

  // set M Exception Program Counter to main, for mret.
  // requires gcc -mcmodel=medany
  uart_print_32bits_register((uint32)main);
  uart_putc('\n');
  w_mepc((uint32)main);

  // disable paging for now.
  w_satp(0);

  // delegate all interrupts and exceptions to supervisor mode.
  w_medeleg(0xffff);
  w_mideleg(0xffff);
  w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);

  // configure Physical Memory Protection to give supervisor mode
  // access to all of physical memory.
  // these 2 lines are essential for project running on qemu-system-riscv. Without these, we can't enter supervisor mode.
  w_pmpaddr0(0x3fffffff);
  w_pmpcfg0(0xf);

  // ask for clock interrupts.
  timerinit();

  // keep each CPU's hartid in its tp register, for cpuid().
  int id = r_mhartid();
  w_tp(id);
  

  // optionally write mstatus value as hex nibbles
  uint32 ms = r_mstatus();
  // write 8 hex chars of ms
  uart_print_32bits_register(ms);
  uart_putc('\n');

  // switch to supervisor mode and jump to main().
  asm volatile("mret");
}



// arrange to receive timer interrupts.
// they will arrive in machine mode at
// at timervec in kernelvec.S,
// which turns them into software interrupts for
// devintr() in trap.c.
void
timerinit()
{
  // each CPU has a separate source of timer interrupts.
  int id = r_mhartid();

  // ask the CLINT for a timer interrupt.
  int interval = 3200000; // cycles; about 1/10th second in qemu.
  *(uint64*)CLINT_MTIMECMP(id) = *(uint64*)CLINT_MTIME + interval; //this is corrected! both in riscv32 and riscv64, this register should be 64bits. It is according to riscv standard
/*
  // safe write of 64-bit mtimecmp on rv32                    //this is bullshit of ai
  volatile uint32 *mtimecmp = (volatile uint32 *)CLINT_MTIMECMP(id);
  volatile uint32 *mtime    = (volatile uint32 *)CLINT_MTIME;
  uint64 now;
  uint32 low, high;

  // read 64-bit mtime safely (loop until high stable)
  while (1) {
    uint32 hi1 = mtime[1];
    uint32 lo  = mtime[0];
    uint32 hi2 = mtime[1];
    if (hi1 == hi2) {
      now = ((uint64)hi1 << 32) | lo;
      break;
    }
  }

  // compute target
  uint64 target = now + (uint64)interval;
  low  = (uint32)target;
  high = (uint32)(target >> 32);

  // write high = 0xFFFFFFFF (or a large value), then low, then high = actual
  mtimecmp[1] = 0xFFFFFFFF;
  mtimecmp[0] = low;
  mtimecmp[1] = high;
*/

  // prepare information in scratch[] for timervec.
  // scratch[0..2] : space for timervec to save registers.
  // scratch[3] : address of CLINT MTIMECMP register.
  // scratch[4] : desired interval (in cycles) between timer interrupts.
  uint32 *scratch = &timer_scratch[id][0];
  scratch[4] = CLINT_MTIMECMP(id);
  scratch[5] = interval;
  w_mscratch((uint32)scratch);

  // set the machine-mode trap handler.
  w_mtvec((uint32)timervec);


  // enable machine-mode interrupts.
  w_mstatus(r_mstatus() | MSTATUS_MIE);

  // enable machine-mode timer interrupts.
  w_mie(r_mie() | MIE_MTIE);

}

