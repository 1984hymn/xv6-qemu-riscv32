typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int  uint32;
// typedef unsigned long uint64; //in riscv32, unsigned long is 32 bits

// even in riscv32, we still have register that is 64 bits, e.g. CLINT. 
// And somethings more ridiculous, the virtio_disk need to follow its rule, 
// it only has 64 bits implementation, 
// And the address to disk need to use 64bits: high 32bits stay 0, low 32bits is the address.
// Furthermore, the "disk request" need to use uint64 as part of the field in struct virtio_blk_req.
// Meantime, the CPU is still 32 bits, but it can load this xv6 os that contains some 64 bits variables. 
// So how does the CPU and compiler allow this to happen? Cuz according to C standard, the type unsigned long long set as 64 bits.
typedef unsigned long long uint64; 

typedef uint32 pde_t;
