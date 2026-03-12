This is a riscv32 version of xv6 running on qemu-system-riscv32.
And it should work properly with qemu in recent version, qemu 7.2 or later...

# before build
You can follow the instruction of MIT to install the qemu.
## toolchain
If you haven't compiled or used riscv toolchain before, 
I highly recommend you to download and install the pre-build toolchain from [Embecosm](https://www.embecosm.com/resources/tool-chain-downloads/#riscv-stable), which was once mentioned in the releases of [pico-sdk-tools](https://github.com/raspberrypi/pico-sdk-tools/releases/tag/v2.0.0-1)

Don't forget the file path where you put the toolchain. Then you should edit the path in the Makefile
```makefile
TOOLPREFIX = /opt/riscv/riscv-toolchain-15/bin/riscv32-unknown-elf-
```
# build xv6
I add a target called xv6_result...it just compile the ELF file kernel/kernel and fs.img without running qemu.
```bash
make clean
make xv6_result
```
Or you can head to the target qemu directly...
```bash
make qemu
```

