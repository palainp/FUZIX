# FUZIX for Texas Instrument 92 plus

## Setting up the toolchain

First you need to build a compatible toolchain, unfortunately, you cannot follow the instructions in README.68000.md of FUZIX as the compiler for Texas Instrument needs some patches. So far the patches for gcc are only for an old version of gcc (4.2.1) and binutils (2.16). You can get the installer for that toolchain at [gcc4ti].

If the build fails, if you use the linux version, you will need to add some symlinks in `gcc4ti/trunk/tigcc-linux/sources`:
```
a68k -> ../../tigcc/a68k
gcc -> ../../tigcc/gcc
ld-tigcc -> ../../tigcc/ld-tigcc
tprbuilder -> ../../tigcc/tprbuilder
```

Once done, you can run the install script `./scripts/Install`. It will ask you where you want to store the toolchain (that will be needed to be added to your PATH, e.g. `/home/user/toolchain-m68k/`), and CFLAGS value for building gcc4ti (regarding my version of gcc that adds some more error flags):
`-Os -s -fno-exceptions -fomit-frame-pointer -Wno-implicit-int -Wno-incompatible-pointer-types -Wno-implicit-function-declaration`

Finally you will also need some utilities to create the rom file:
- tib2xxu from [freeflash]
- [rabbitsign]
Building those tools is straighforward (you need to desactive the WINDOWS macro if you are compiling with linux):
```bash
cd fflash/tib2xxu && \
sed -i -Ee 's|^(\#define WINDOWS)|//\1|' tib2xxu.c  && \
gcc tib2xxu.c -o /home/user/toolchain-m68k/bin/tib2xxu
```
And for rabbitsign:
```bash
cd rabbitsign-2.1 && \
./configure --prefix=/home/user/toolchain-m68k/ && \
make -j 8 && make install
```

You will also need to copy the 0104 signing key from rabbitsign into `Kernel/platform-ti92p`.

##  Building the kernel and creating the rom file


## Testing the image

Before flashing anything on your real device, you should check that the image is working fine with [ti92p-emu] :)


[gcc4ti]: https://github.com/debrouxl/gcc4ti
[freeflash]: https://www.ticalc.org/archives/files/fileinfo/368/36829.html
[rabbitsign]: https://www.ticalc.org/archives/files/fileinfo/383/38392.html
[ti92p-emu]: https://tiplanet.org/pad_ti68k_emu/
