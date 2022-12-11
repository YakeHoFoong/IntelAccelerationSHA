// SPDX-FileCopyrightText: © 2021 Yake Ho Foong
// SPDX-License-Identifier: BSD-3-Clause

#include "pch.h"

#define SHA256LEN  32

int __cdecl WinCalcSHA256(const BYTE rgbMsg[], uint64_t size_in_bytes, BYTE output_hash[])
{
    DWORD dwStatus = 0;
    BOOL bResult = FALSE;
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash, hHashClone;
    HCRYPTKEY pbHashValue[36];

    DWORD cbRead = 0;
    BYTE rgbHash[SHA256LEN];
    DWORD cbHash = 0;
    // Logic to check usage goes here.

    // Get handle to the crypto provider
    if (!CryptAcquireContext(&hProv,
        NULL,
        NULL,
        PROV_RSA_AES,
        CRYPT_VERIFYCONTEXT))
    {
        dwStatus = GetLastError();
        printf("CryptAcquireContext failed: %d\n", dwStatus);

        return dwStatus;
    }

    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash))
    {
        dwStatus = GetLastError();
        printf("CryptAcquireContext failed: %d\n", dwStatus);
        CryptReleaseContext(hProv, 0);

        return dwStatus;
    }

    if (!CryptHashData(hHash, rgbMsg, size_in_bytes, 0))
    {
        dwStatus = GetLastError();
        printf("CryptHashData failed: %d\n", dwStatus);
        CryptReleaseContext(hProv, 0);
        CryptDestroyHash(hHash);

        return 1;
    }

    DWORD hashlen;
    cbHash = sizeof(DWORD);
    if (CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)&hashlen, &cbHash, 0))
    {
        assert(hashlen == 32);
        CryptGetHashParam(hHash, HP_HASHVAL, output_hash, &hashlen, 0);
    }
    else
    {
        dwStatus = GetLastError();
        printf("CryptGetHashParam failed: %d\n", dwStatus);

    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    return 0;
}