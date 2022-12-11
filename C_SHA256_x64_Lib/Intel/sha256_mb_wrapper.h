/* SPDX-FileCopyrightText: © 2021 Yake Ho Foong
   SPDX-FileCopyrightText: Copyright(c) 2011-2016 Intel Corporation All rights reserved.
   SPDX-License-Identifier: BSD-3-Clause */

#include <stdint.h>
#include "sha256_mb.h"
#include "endian_helper.h"

/* SHA256_MB_ARGS_X16 is defined in sha256_mb.h */

typedef struct {
	uint32_t digest[2][8][8];
	uint8_t* data_ptr[SHA256_MAX_LANES];
} SHA256_MB_ARGS_X8;

typedef struct {
	uint32_t digest[4][8][4];
	uint8_t* data_ptr[SHA256_MAX_LANES];
} SHA256_MB_ARGS_X4;

#ifdef __cplusplus
extern "C" {
#endif
	extern void sha256_mb_x16_avx512_wrapper(SHA256_MB_ARGS_X16* args_struct, uint64_t size_in_blocks);
	extern void sha256_mb_x8_avx2_wrapper(SHA256_MB_ARGS_X8* args_struct, uint64_t size_in_blocks);
	extern void sha256_mb_x4_avx_wrapper(SHA256_MB_ARGS_X4* args_struct, uint64_t size_in_blocks);
	extern void sha256_mb_x4_sse_wrapper(SHA256_MB_ARGS_X4* args_struct, uint64_t size_in_blocks);
#ifdef __cplusplus
}
#endif