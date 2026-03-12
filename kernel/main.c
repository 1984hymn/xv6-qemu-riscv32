#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

volatile static int started = 0;

// start() jumps here in supervisor mode on all CPUs.
void
main()
{
  
  volatile unsigned int *uart = (volatile unsigned int *)UART0;
  *uart = 'M'; // entered main
  *uart = 'a'; // entered main
  *uart = 'i'; // entered main
  *uart = 'n'; // entered main
  *uart = '\n'; // entered main

  *uart = (cpuid() + 48); //add ASCII code and check cpu id
  *uart = '\n';
  if(cpuid() == 0){
    consoleinit();
  
    printfinit();
    
    printf("\n");
    printf("xv6 kernel is booting\n");
    printf("\n");

    kinit();         // physical page allocator
    kvminit();       // create kernel page table
    kvminithart();   // turn on paging
    procinit();      // process table
    trapinit();      // trap vectors
    trapinithart();  // install kernel trap vector
    plicinit();      // set up interrupt controller
    plicinithart();  // ask PLIC for device interrupts
    binit();         // buffer cache
    iinit();         // inode table
    fileinit();      // file table
    virtio_disk_init();
    userinit();      // first user process
    __sync_synchronize();
    started = 1;
  } else {
    while(started == 0)
      ;
    __sync_synchronize();
    printf("hart %d starting\n", cpuid());
    kvminithart();    // turn on paging
    trapinithart();   // install kernel trap vector
    plicinithart();   // ask PLIC for device interrupts
  }

  scheduler();        
}
