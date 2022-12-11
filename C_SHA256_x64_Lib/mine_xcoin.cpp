// SPDX-FileCopyrightText: © 2021 Yake Ho Foong
// SPDX-License-Identifier: BSD-3-Clause

#include "pch.h"

#include "mine_xcoin.h"

#if defined(_MSC_VER ) && defined(_WIN64)
#define ALLOC_ALIGNED(A, S) (_aligned_malloc(S, A))
#define FREE_ALIGNED(P) (_aligned_free(P))
#endif

#if defined(__GNUC__)
#define ALLOC_ALIGNED(A, S) (std::aligned_alloc(A, S))
#define FREE_ALIGNED(P) (std::free(P))
#endif

// function for each thread
void worker_mine(int thread_num, const uint8_t target[DIGEST_SIZE_BYTES], uint32_t state[DIGEST_NUM_WORDS], uint64_t nonce_result[1], uint32_t residual_message_len,
    const uint8_t tail_message[], uint64_t tail_message_len, uint64_t nonce_beg, uint64_t nonce_step, std::atomic<int>& winning_thread,
    SHA256_Acceleration use_acceleration, const std::chrono::duration<double>& timeout_seconds)
{
    auto start = std::chrono::steady_clock::now();

    int num_lanes = lane_counts[(uint8_t)use_acceleration];
    uint64_t num_blocks = tail_message_len / BLOCK_SIZE_BYTES; // block size 64 bytes (512 bits), should be either 1 or 2
    assert(num_blocks == 1 || num_blocks == 2);

    // convert target number to state - see below in the search loop
    uint32_t target_state[DIGEST_NUM_WORDS];
    std::memset(target_state, 0, DIGEST_SIZE_BYTES);
    uint32_t* target_ptr32 = (uint32_t*)target;
    for (int w = 0; w < DIGEST_NUM_WORDS; w++) // convert big endian 32-byte integer (stored in a byte array) into digest/state form
        target_state[w] = byteswap32(target_ptr32[w]);

    // copy start states or digests, and create test tail messages and point the nonces to their right location
    // do not use the new[] aligned, Visual C++ throws exception on delete[], no idea why
    // uint32_t* start_states = (uint32_t*) operator new[](sizeof(uint32_t) * (num_lanes* DIGEST_SIZE_BYTES), (std::align_val_t)(64)); // alignment just in case faster
    // uint8_t* test_tail_messages = (uint8_t*) operator new[](sizeof(uint8_t) * (num_lanes* tail_message_len), (std::align_val_t)(64));   // alignment just in case faster
    uint32_t* start_states = (uint32_t*)ALLOC_ALIGNED(64, sizeof(uint32_t) * num_lanes * DIGEST_SIZE_BYTES);  // alignment just in case faster
    uint8_t* test_tail_messages = (uint8_t*)ALLOC_ALIGNED(64, sizeof(uint8_t) * num_lanes * tail_message_len);    // alignment just in case faster

    for (int j = 0; j < num_lanes; j++) {
        for (int w = 0; w < DIGEST_NUM_WORDS; w++) start_states[w * num_lanes + j] = state[w];  // transposed form
        std::memcpy(test_tail_messages + j * tail_message_len, tail_message, tail_message_len); // this is NOT in transposed form, because these are pointed to
    }
    uint64_t* nonce_ptrs[SHA256_MAX_LANES];
    for (int j = 0; j < num_lanes; j++)
        nonce_ptrs[j] = (uint64_t*)&(test_tail_messages[residual_message_len + j * tail_message_len]);  // NOT transposed, see above

    // now search for the winning nonce!
    alignas(64) struct {
        uint32_t digest[DIGEST_NUM_WORDS * SHA256_MAX_LANES];
        uint8_t* data_ptr[SHA256_MAX_LANES];
    } args_generic; // all the vector functions use the same sized struct, see inside the file sha256_mb_wrapper.h
    for (int j = 0; j < num_lanes; j++)
        args_generic.data_ptr[j] = test_tail_messages + tail_message_len * j;
    uint64_t last_nonce = MAX_NONCE - ((MAX_NONCE - nonce_beg) % nonce_step);
    uint64_t next_check_nonce = 0;  // for timer
    for (uint64_t nonce = nonce_beg; nonce <= last_nonce; nonce+=nonce_step) {
        // check for timeout
        if (nonce_beg == 0 && nonce >= next_check_nonce) {   // save time, only check timer in 1 thread
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed = end - start;
            if (elapsed > timeout_seconds)  winning_thread = 0; // signal early exit
            if (nonce > nonce_step) {
                auto remaining = timeout_seconds - elapsed;
                double nonces_done = nonce - nonce_step;
                // estimate a jump ahead before checking the timer again
                next_check_nonce = nonce + uint64_t(nonces_done * (remaining / elapsed) * 0.5); // last factor is for conservatism, must be less than 1
            }
        }

        if (winning_thread >= 0) {   // early exit, another worker has won, or early abort signaled (zero)
            FREE_ALIGNED(start_states);
            FREE_ALIGNED(test_tail_messages);
            return;
        }

        // extra calc for final nonce when lanes > 1
        uint64_t j_max = num_lanes - 1ULL;
        if (nonce > MAX_NONCE - j_max)    // if nonce + j_max > MAX_NONCE
            j_max = MAX_NONCE - nonce;    // so that nonce + j_max = MAX_NONCE
        // copy state, start over
        std::memcpy(args_generic.digest, start_states, num_lanes * DIGEST_SIZE_BYTES);
        for (uint64_t j = 0; j <= j_max; j++)
            *nonce_ptrs[j] = nonce + j; // fill little endian, this will modify test_tail_messages

        // pick the SHA256 function to call
        uint8_t checking[128];
        std::memcpy(checking, args_generic.data_ptr[0], 128);
        switch (use_acceleration) {
            case SHA256_Acceleration::SHA:
                sha256_sha_sse41(args_generic.digest, args_generic.data_ptr[0], num_blocks);
                break;
            case SHA256_Acceleration::AVX512:
                sha256_mb_x16_avx512_wrapper((SHA256_MB_ARGS_X16*)&args_generic, num_blocks);
                break;
            case SHA256_Acceleration::AVX2:
                sha256_mb_x8_avx2_wrapper((SHA256_MB_ARGS_X8*)&args_generic, num_blocks);
                break;
            case SHA256_Acceleration::AVX:
                sha256_mb_x4_avx_wrapper((SHA256_MB_ARGS_X4*)&args_generic, num_blocks);
                break;
            case SHA256_Acceleration::SSE41:
                sha256_mb_x4_sse_wrapper((SHA256_MB_ARGS_X4*)&args_generic, num_blocks);
                break;
            default:    // plain C
                sha256_process(args_generic.digest, args_generic.data_ptr[0], (uint32_t)tail_message_len);
                break;
        }
        // the Intel vector functions increments the pointers because it has a reference to them through the args struct, so we need to undo that
        if (num_lanes > 1) {
            for (int j = 0; j < num_lanes; j++)
                args_generic.data_ptr[j] -= tail_message_len;
        }
        
        // check if any of the result(s) is a winner
        for (int j = 0; j <= j_max; j++) {
            bool found = false;
            for (int w = 0; w < DIGEST_NUM_WORDS; w++) {
                uint32_t test_val = args_generic.digest[w * num_lanes + j];  // this is in transposed form
                uint32_t target_val = target_state[w];
                if (test_val < target_val) {
                    found = true;
                    break;
                }
                if (test_val > target_val) {
                    found = false;
                    break;
                }
            }
            if (found) {
                winning_thread = thread_num;   // signal other workers to stop immediately
                for (int w = 0; w < DIGEST_NUM_WORDS; w++)  // copy result to output
                    state[w] = args_generic.digest[w * num_lanes + j]; // this is in transposed form
                nonce_result[0] = nonce + j; // copy result to output
                FREE_ALIGNED(start_states);
                FREE_ALIGNED(test_tail_messages);
                return;
            }
        }
    }

    // failed to find a winner after trying all nonces that this worker is responsible for
    FREE_ALIGNED(start_states);
    FREE_ALIGNED(test_tail_messages);
    return;

}


// return true if successful
bool mine_xcoin(const uint8_t target[DIGEST_SIZE_BYTES], const uint8_t message_ex_nonce[], uint64_t len_bytes, SHA256_Acceleration preferred_acceleration,
    uint8_t result_id[DIGEST_SIZE_BYTES], uint64_t result_nonce[1], SHA256_Acceleration acceleration_used[1], uint32_t num_threads_used[1], double timeout_seconds)
{
    // timeout timer
    auto start = std::chrono::steady_clock::now();

    // check the preferred acceleration method, and fallback to next best if not supported by CPU
    // if (preferred_acceleration < SHA256_Acceleration::AVX512)   preferred_acceleration = SHA256_Acceleration::AVX512;  // this line is not possible, no need to check
    if (preferred_acceleration > SHA256_Acceleration::NO_ACCEL) preferred_acceleration = SHA256_Acceleration::NO_ACCEL;
    SHA256_Acceleration use_acceleration = preferred_acceleration;
    // ignore the compiler warnings, the following 2 lines can never fail because the last element is always true
    while (!supported_accelerations[(uint8_t)use_acceleration])  use_acceleration = SHA256_Acceleration((uint8_t)use_acceleration + 1);
    int num_lanes = lane_counts[(uint8_t)use_acceleration];

    // set the number of threads to the number recommended by the standard library
    uint64_t num_threads = std::thread::hardware_concurrency();

    // return diagnostics
    acceleration_used[0] = use_acceleration;
    num_threads_used[0] = (uint32_t)num_threads;

    // state is 8 32-bit words i.e. 32 bytes
    // initial state
    uint32_t state[DIGEST_NUM_WORDS] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    // data is processed in 512-bit (64-byte) chunks (blocks)
    uint64_t preprocess_chunks_num = len_bytes / BLOCK_SIZE_BYTES;

    uint64_t preprocess_bytes_num = preprocess_chunks_num * BLOCK_SIZE_BYTES;
    const uint8_t* residual_message = message_ex_nonce + preprocess_bytes_num; // pointer to start of the incomplete chunk (block)
    uint64_t residual_message_len = len_bytes - preprocess_bytes_num;  // between 0~64 bytes

    // partial pre-processing
    if (preprocess_chunks_num > 0)
        sha256_process(state, message_ex_nonce, (uint32_t)preprocess_bytes_num);

    // residual_message is 0~63 bytes, nonce is 64 bit or 8 bytes, 1 byte for the 1 bit, 8 bytes for length
    // tail_message_len should be either 64 bytes (512 bits) or 128 bytes (512 bits x 2)
    uint64_t num_chunks_left = 1ULL;  // 1 or 2
    if (residual_message_len + NONCE_SIZE_BYTES > BLOCK_SIZE_BYTES - 8 - 1) num_chunks_left = 2ULL;
    uint64_t tail_message_len = BLOCK_SIZE_BYTES * num_chunks_left;
    uint8_t tail_message[BLOCK_SIZE_BYTES * 2];  // max 2 blocks
    std::memset(tail_message, 0x00, sizeof(tail_message));
    std::memcpy(tail_message, residual_message, residual_message_len);  // copy the residual message into tail message
    
    // the 1 bit in padding after nonce
    tail_message[residual_message_len + NONCE_SIZE_BYTES] = 0x80;
    // total length excluding padding is len_bytes + 8 (nonce), in bytes, below is in bits
    uint64_t total_bitlen_ex_pad = (len_bytes + NONCE_SIZE_BYTES) * 8ULL;   // times 8 to convert from bytes to bits
    // now fill into padding using big endian i.e. lowest memory index is most significant byte
    *((uint64_t*)(tail_message + tail_message_len - 8)) = byteswap64(total_bitlen_ex_pad);

    // parallel processing
    // copy the states
    // do not use the new[] aligned, Visual C++ throws exception on delete[], no idea why
    // uint32_t* states = (uint32_t*) operator new[](sizeof(uint32_t)* (DIGEST_NUM_WORDS * num_threads), (std::align_val_t)(64)); // alignment just in case faster
    // uint64_t* nonces = (uint64_t*) operator new[](sizeof(uint64_t)* (num_threads), (std::align_val_t)(64)); // alignment just in case faster
    uint32_t* states = (uint32_t*)ALLOC_ALIGNED(64, sizeof(uint32_t) * DIGEST_NUM_WORDS * num_threads); // alignment just in case faster
    uint64_t* nonces = (uint64_t*)ALLOC_ALIGNED(64, sizeof(uint64_t) * num_threads); // alignment just in case faster

    std::atomic<int> winning_thread = -1;    // this is how the threads let each other know when to stop i.e. once this is positive, then stop because we have a winner, or early abort (zero)
    std::vector<std::thread> threads;
    uint64_t nonce_step = num_threads * num_lanes;  // each thread would cover num_lanes in each iteration

    // timeout timer
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    auto timeout_secs = std::chrono::duration<double>(timeout_seconds) - elapsed;
    if (std::chrono::duration_cast<std::chrono::microseconds>(timeout_secs).count() <= 0)   return false;   // failed, already timed out
    // create calculation threads and run
    for (uint64_t i = 0; i < num_threads; i++) {
        std::memcpy(states + i * DIGEST_NUM_WORDS, state, DIGEST_SIZE_BYTES);
        threads.emplace_back(std::thread(worker_mine, (int)(i + 1), target, states + i * DIGEST_NUM_WORDS, nonces + i, (uint32_t)residual_message_len,
            tail_message, tail_message_len, i * num_lanes, nonce_step, std::ref(winning_thread), use_acceleration, timeout_secs));
    }
    // wait for them to complete
    for (auto& th : threads)
        th.join();

    // checking winning thread results and pass back
    int winning_i = winning_thread - 1;
    if (winning_i < 0) return false;   // all failed (winning_thread zero is early abort, -1 is initial value)
    // convert little-endian state into hash
    uint32_t* winning_state = states + winning_i * DIGEST_NUM_WORDS;
    uint32_t* result_id_ptr32 = (uint32_t*)result_id;
    // the return output result is bytes array in big endian
    for (int w = 0; w < DIGEST_NUM_WORDS; w++)
        result_id_ptr32[0 + w] = byteswap32(winning_state[0 + w]);
    result_nonce[0] = nonces[winning_i];

    // clean up
    FREE_ALIGNED(states);
    FREE_ALIGNED(nonces);

    // return
    return true;
}

