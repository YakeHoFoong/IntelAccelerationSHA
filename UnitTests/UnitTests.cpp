// SPDX-FileCopyrightText: © 2021 Yake Ho Foong
// SPDX-License-Identifier: BSD-3-Clause

#include "pch.h"
#include "CppUnitTest.h"

#include "..\C_SHA256_x64_Lib\mine_xcoin.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

extern int __cdecl WinCalcSHA256(const BYTE rgbMsg[], uint64_t size_in_bytes, BYTE output_hash[]);

// test inputs
static const uint8_t target[DIGEST_SIZE_BYTES] =
{ 0x0, 0x0, 0x10, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
static const uint8_t message_ex_nonce[] =
{ 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0xe5, 0x2b, 0xf4, 0xcd, 0x99, 0x98, 0xa4, 0xdb, 0x17, 0x6a, 0xcd, 0x8f, 0x91, 0x1b, 0xd5, 0x5d, 0x5, 0x80, 0x76, 0xde, 0xa4, 0x88, 0x5e,
    0x83, 0xb3, 0x25, 0xeb, 0x21, 0x25, 0xd1, 0xdf, 0xaf, 0x0, 0xbc, 0x63, 0x6b, 0x56, 0x6d, 0x34, 0xa3, 0x2d, 0x54, 0x4b, 0x38, 0xb6, 0xe9, 0xea, 0x34,
    0xef, 0x90, 0x91, 0xcf, 0xaa, 0x74, 0x2c, 0x6f, 0xd, 0x72, 0xc5, 0xfb, 0x79, 0xeb, 0xd4, 0x9d, 0xe2, 0xd3, 0x74, 0x35, 0xa1, 0xb9, 0x1f, 0x8a, 0x1f,
    0x20, 0x99, 0xde, 0x5c, 0x7, 0xb5, 0x84, 0x0, 0x33, 0xce, 0x85, 0xf7, 0xe9, 0x2, 0x61, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x10, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

namespace UnitTests
{
	TEST_CLASS(UnitTests)
	{
    private:
        void test_sha256(SHA256_Acceleration accel)
        {
            uint8_t result_id[DIGEST_SIZE_BYTES];
            uint64_t result_nonce[1];
            SHA256_Acceleration acceleration_used[1];   // diagnostics not used
            uint32_t num_threads_used[1];   // diagnostics not used

            bool result = mine_xcoin(target, message_ex_nonce, sizeof(message_ex_nonce), accel,
                    result_id, result_nonce, acceleration_used, num_threads_used, 10.0);   // timeout of 10 seconds

            // check function return status
            Assert::IsTrue(
                // Actual value:
                result,
                // Message:
                L"SHA256 test failed, function returned status false",
                // Line number - used if there is no PDB file:
                LINE_INFO());

            // check nonce
            bool id_not_more_than_target = false;
            for (int i = 0; i < DIGEST_SIZE_BYTES; i++) {
                if (result_id[i] < target[i]) {
                    id_not_more_than_target = true;
                    break;
                }
                else if (result_id[i] > target[i]) break;
            }
            Assert::IsTrue(
                // Actual value:
                id_not_more_than_target,
                // Message:
                L"SHA256 test failed, ID is greater than target",
                // Line number - used if there is no PDB file:
                LINE_INFO());

            // check the digest
            uint8_t message[sizeof(message_ex_nonce) + 8];
            std::memcpy(message, message_ex_nonce, sizeof(message_ex_nonce));
            *((uint64_t*)(message + sizeof(message_ex_nonce))) = result_nonce[0];
            uint8_t expected_digest[32];
            WinCalcSHA256(message, sizeof(message), expected_digest);
            std::vector<uint8_t> expected_id(std::begin(expected_digest), std::end(expected_digest));
            std::vector<uint8_t> actual_id(std::begin(result_id), std::end(result_id));
            Assert::IsTrue(
                // Actual value:
                actual_id == expected_id,
                // Message:
                L"SHA256 test failed, hash or digest is not the expected value",
                // Line number - used if there is no PDB file:
                LINE_INFO());
        }
	public:
        TEST_METHOD(TestMethodAVX512)
        {
            test_sha256(SHA256_Acceleration::AVX512);
        }
        TEST_METHOD(TestMethodSHA)
        {
            test_sha256(SHA256_Acceleration::SHA);
        }
        TEST_METHOD(TestMethodAVX2)
        {
            test_sha256(SHA256_Acceleration::AVX2);
        }
        TEST_METHOD(TestMethodAVX)
        {
            test_sha256(SHA256_Acceleration::AVX);
        }
        TEST_METHOD(TestMethodSSE41)
        {
            test_sha256(SHA256_Acceleration::SSE41);
        }
        TEST_METHOD(TestMethodNO_ACCEL)
        {
            test_sha256(SHA256_Acceleration::NO_ACCEL);
        }
    };
}
