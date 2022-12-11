// SPDX-FileCopyrightText: © 2021 Yake Ho Foong
// SPDX-License-Identifier: BSD-3-Clause

#include "Intel/sha256_mb_wrapper.h"
#include "Intel/sha256_sha_sse41.h"
#include "Microsoft/cpuid.cpp"

const uint64_t DIGEST_NUM_WORDS = 8;    // each WORD is a 32 bits
const uint64_t DIGEST_WORD_SIZE_BYTES = 4;
const uint64_t DIGEST_SIZE_BYTES = DIGEST_NUM_WORDS * DIGEST_WORD_SIZE_BYTES;
const uint64_t BLOCK_SIZE_BYTES = 64;
const uint64_t NONCE_SIZE_BYTES = 8; // 64 bit integer
const uint64_t MAX_NONCE = ULLONG_MAX; // 64 bit integer

// enum class to indicate the preferred acceleration method
enum class SHA256_Acceleration : uint8_t { AVX512 = 0, SHA = 1, AVX2 = 2, AVX = 3, SSE41 = 4, NO_ACCEL = 5 };
static bool supported_accelerations[] = { InstructionSet::AVX512F(), InstructionSet::SHA() && InstructionSet::SSE41(), InstructionSet::AVX2(),
    InstructionSet::AVX(), InstructionSet::SSE41(), true };
// vector instructions can handle multiple messages at the same time
static int lane_counts[] = { 16, 1, 8, 4, 4, 1 };

#if defined(_MSC_VER ) && defined(_WIN64)
#ifdef _EXPORTING
   #define CLASS_DECLSPEC    __declspec(dllexport)
#else
   #define CLASS_DECLSPEC    __declspec(dllimport)
#endif
#endif

#if defined(__GNUC__)
    #define CLASS_DECLSPEC
#endif

extern "C" {
	CLASS_DECLSPEC bool mine_xcoin(const uint8_t target[DIGEST_SIZE_BYTES], const uint8_t message_ex_nonce[], uint64_t len_bytes, SHA256_Acceleration preferred_acceleration,
        uint8_t result_id[DIGEST_SIZE_BYTES], uint64_t result_nonce[1], SHA256_Acceleration acceleration_used[1], uint32_t num_threads_used[1], double timeout_seconds);
    void sha256_process_x86(uint32_t state[DIGEST_NUM_WORDS], const uint8_t data[], uint32_t length);  // SHA version by Jeffrey Walton
    void sha256_process(uint32_t state[8], const uint8_t data[], uint32_t length);   // plain C version by Jeffrey Walton
}
