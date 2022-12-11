# SPDX-FileCopyrightText: © 2021 Yake Ho Foong
# SPDX-License-Identifier: BSD-3-Clause

AS = nasm #Assembly compiler
CC = g++  # C compiler
LD = g++ -shared # linker, use gcc to call ld
ASFLAGS = -f elf64 -IIntel -DHAVE_AS_KNOWS_AVX512 -g
C_CPP_FLAGS = -Wall -Wextra -O3 -g -march=x86-64 -fPIC -pthread
CFLAGS = -c $(C_CPP_FLAGS) -std=c17 -lstdc # C flags, -c means compile only do not link
CPPCFLAGS = -c $(C_CPP_FLAGS) -std=c++17 -lstdc++ # C++ flags, -c means compile only do not link
LDFLAGS = $(C_CPP_FLAGS) -lstdc++ -z defs -Wl,--version-script=libcode.version # linking flags
RM = rm -f   # rm command

TARGET_LIB = build/mine_xcoin.so  # target lib

# Assembly files
SOURCES_A_RAW = sha256_mb_xx_wrapper.asm sha256_sha_sse41.asm \
	sha256_mb_x16_avx512.asm sha256_mb_x8_avx2.asm \
	sha256_mb_x4_avx.asm sha256_mb_x4_sse.asm
SOURCES_A = $(SOURCES_A_RAW:%.asm=Intel/%.asm)
OBJECTS_A = $(SOURCES_A_RAW:%.asm=build/%.o)

# C files excluding the XCoin file
SOURCES_C_RAW = sha256.c
SOURCES_C = $(SOURCES_C_RAW:%.c=JeffreyWalton/%.c)
OBJECTS_C = $(SOURCES_C_RAW:%.c=build/%.o)

# C file for XCoin
SRC_X = mine_xcoin.cpp
OBJ_X = $(SRC_X:%.cpp=build/%.o)

# All sources
SOURCES = $(SRC_X) $(SOURCES_C) $(SOURCES_A)
OBJECTS = $(OBJ_X) $(OBJECTS_C) $(OBJECTS_A)

#Check version, and also this is necessary to trigger the
# implicit looping mechanism in Make for the assembly line below
all: $(SOURCES) $(TARGET_LIB)

#Link
$(TARGET_LIB): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

# build XCoin
$(OBJ_X): $(SRC_X)
	$(CC) $(CPPCFLAGS) $< -o $@ 

# build other C files	
$(OBJECTS_C): $(SOURCES_C)
	$(CC) $(CFLAGS) $< -o $@


# build assembly files

build/sha256_mb_xx_wrapper.o: Intel/sha256_mb_xx_wrapper.asm
	$(AS) $(ASFLAGS) -o $@ $<

build/sha256_sha_sse41.o: Intel/sha256_sha_sse41.asm
	$(AS) $(ASFLAGS) -o $@ $<

build/sha256_mb_x16_avx512.o: Intel/sha256_mb_x16_avx512.asm
	$(AS) $(ASFLAGS) -o $@ $<

build/sha256_mb_x8_avx2.o: Intel/sha256_mb_x8_avx2.asm
	$(AS) $(ASFLAGS) -o $@ $<

build/sha256_mb_x4_avx.o: Intel/sha256_mb_x4_avx.asm
	$(AS) $(ASFLAGS) -o $@ $<

build/sha256_mb_x4_sse.o: Intel/sha256_mb_x4_sse.asm
	$(AS) $(ASFLAGS) -o $@ $<
