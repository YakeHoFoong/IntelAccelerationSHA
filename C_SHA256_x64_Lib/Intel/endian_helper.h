/* SPDX-FileCopyrightText: Copyright(c) 2011-2016 Intel Corporation All rights reserved.
   SPDX-License-Identifier: BSD-3-Clause */

#ifndef _ENDIAN_HELPER_H_
#define _ENDIAN_HELPER_H_

/**
 *  @file  endian_helper.h
 *  @brief Byte order helper routines
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#if defined (__ICC)
# define byteswap32(x) _bswap(x)
# define byteswap64(x) _bswap64(x)
#elif defined (__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))
# define byteswap32(x) __builtin_bswap32(x)
# define byteswap64(x) __builtin_bswap64(x)
#else
# define byteswap32(x) (  ((x) << 24) \
                        | (((x) & 0xff00) << 8) \
                        | (((x) & 0xff0000) >> 8) \
                        | ((x)>>24))
# define byteswap64(x) (  (((x) & (0xffull << 0)) << 56) \
                        | (((x) & (0xffull << 8)) << 40) \
                        | (((x) & (0xffull << 16)) << 24) \
                        | (((x) & (0xffull << 24)) << 8) \
                        | (((x) & (0xffull << 32)) >> 8) \
                        | (((x) & (0xffull << 40)) >> 24) \
                        | (((x) & (0xffull << 48)) >> 40) \
                        | (((x) & (0xffull << 56)) >> 56))
#endif

// This check works when using GCC (or LLVM).  Assume little-endian
// if any other compiler is being used.
#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) \
    && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define to_le32(x) byteswap32(x)
#define to_le64(x) byteswap64(x)
#define to_be32(x) (x)
#define to_be64(x) (x)
#else
#define to_le32(x) (x)
#define to_le64(x) (x)
#define to_be32(x) byteswap32(x)
#define to_be64(x) byteswap64(x)
#endif

#ifdef __cplusplus
}
#endif

#endif // _ISA_HELPER_H_
