/* SPDX-FileCopyrightText: © 2021 Yake Ho Foong
   SPDX-License-Identifier: BSD-3-Clause */

#ifdef __cplusplus
extern "C" {
#endif
	extern void sha256_sha_sse41(uint32_t state[8], const uint8_t data[], uint64_t size_in_blocks);
#ifdef __cplusplus
}
#endif