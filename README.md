SPDX-FileCopyrightText: © 2021 Yake Ho Foong
SPDX-FileCopyrightText: Copyright(c) 2011-2017 Intel Corporation All rights reserved.
SPDX-License-Identifier: BSD-3-Clause

Intel(R) Intelligent Storage Acceleration Library Crypto Version - Modified for Cryptocurrency Use  
==================================================================================================  

The reason for creating this fork is to create a library that is more suitable for cryptocurrency use cases.

To compile the C++ and assembly code:
* Windows x86-64: open the solution file in Visual Studio. Unit tests are included (need to have Visual Studio and NASM installed)
* Linux x86-64: run 'make -B' in a shell in the subfolder C_SHA256_x64/C_SHA256_x64_Lib (need to have GCC and NASM installed)
* Mac: not implemented

The ".asm" files are modified from Intel's files, except for "sha256_mb_xx_wrapper.asm" which was written by the author for this project. Once compiled by NASM, the functions below are available to the C code:
* sha256_mb_x16_avx512
* sha256_mb_x8_avx2
* sha256_mb_x4_avx
* sha256_mb_x4_sse
* sha256_sha_sse41

Note that "sha256_sha_sse41" is the SHA NI instructions i.e. the fastest extended instruction set to process SHA256.


As an example of usage, the library file mine_xcoin.cpp is provided. It takes a digest (an array of bytes) and a difficulty as inputs, and returns a nonce as output. The nonce is any value that satisfies SHA256(digest appended by nonce) <= difficulty. The size of the digest is fixed by the constants in the file mine_xcoin.h, DIGEST_NUM_WORDS and DIGEST_WORD_SIZE_BYTES. The compiled binary output file is a ".DLL" file in Windows and a ".so" file in Linux. This example C++ shared library is multithreaded.


The file "c_sha256_lib.py" allows the user to call the libraries above (both Windows and Linux) from Python. It also serves as an example of how to use this library.


As an illustration of how much these extended instruction sets can accelerate the calculations of SHA256 hashes in cryptocurrencies, refer to the graph below. Note that the Y-axis is in logarithmic scale.  

![Graph of hashing speed under various codes](https://github.com/YakeHoFoong/IntelAccelerationSHA/blob/master/Graph.svg) 


Below is the original Intel README.MD.
==================================================================================================  

ISA-L_crypto is a collection of optimized low-level functions targeting storage
applications.  ISA-L_crypto includes:

* Multi-buffer hashes - run multiple hash jobs together on one core for much
  better throughput than single-buffer versions.
  - SHA1, SHA256, SHA512, MD5, SM3

* Multi-hash - Get the performance of multi-buffer hashing with a single-buffer
  interface. Specification ref : [Multi-Hash white paper](https://www.intel.com/content/dam/www/public/us/en/documents/white-papers/multi-hash-paper.pdf)

* Multi-hash + murmur - run both together.

* AES - block ciphers
  - XTS, GCM, CBC

* Rolling hash - Hash input in a window which moves through the input

Also see:
* [ISA-L_crypto for updates](https://github.com/intel/isa-l_crypto).
* For non-crypto ISA-L see [isa-l on github](https://github.com/intel/isa-l).
* The [github wiki](https://github.com/intel/isa-l/wiki) covering isa-l and
  isa-l crypto.
* [Contributing](CONTRIBUTING.md).
* [Security Policy](SECURITY.md).

Building ISA-L
--------------

### Prerequisites

x86_64:
* Assembler: nasm v2.11.01 or later (nasm v2.13 or better suggested for building in AVX512 support)
  or yasm version 1.2.0 or later.
* Compiler: gcc, clang, icc or VC compiler.
* Make: GNU 'make' or 'nmake' (Windows).
* Optional: Building with autotools requires autoconf/automake packages.

aarch64:
* Assembler: gas v2.34 or later.
* Compiler: gcc v8 or later.
* For gas v2.24~v2.34, sve2 instructions are not supported. To workaround it, sve2 optimization should be disabled by
    * ./configure --disable-sve2
    * make -f Makefile.unx DEFINES+=-DNO_SVE2=1

### Autotools
To build and install the library with autotools it is usually sufficient to run:

    ./autogen.sh
    ./configure
    make
    sudo make install

### Makefile
To use a standard makefile run:

    make -f Makefile.unx

### Windows
On Windows use nmake to build dll and static lib:

    nmake -f Makefile.nmake

### Other make targets
Other targets include:
* `make check` : create and run tests
* `make tests` : create additional unit tests
* `make perfs` : create included performance tests
* `make ex`    : build examples
* `make doc`   : build API manual
