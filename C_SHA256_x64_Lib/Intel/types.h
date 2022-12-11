/* SPDX-FileCopyrightText: Copyright(c) 2011-2016 Intel Corporation All rights reserved.
   SPDX-License-Identifier: BSD-3-Clause */

/**
 *  @file  types.h
 *  @brief Defines common align and debug macros
 *
 */

#ifndef __TYPES_H
#define __TYPES_H

#ifdef __cplusplus
extern "C" {
#endif


#if defined  __unix__ || defined __APPLE__
# define DECLARE_ALIGNED(decl, alignval) decl __attribute__((aligned(alignval)))
# define __forceinline static inline
# define aligned_free(x) free(x)
#else
# ifdef __MINGW32__
#   define DECLARE_ALIGNED(decl, alignval) decl __attribute__((aligned(alignval)))
#   define posix_memalign(p, algn, len) (NULL == (*((char**)(p)) = (void*) _aligned_malloc(len, algn)))
#   define aligned_free(x) _aligned_free(x)
# else
#   define DECLARE_ALIGNED(decl, alignval) __declspec(align(alignval)) decl
#   define posix_memalign(p, algn, len) (NULL == (*((char**)(p)) = (void*) _aligned_malloc(len, algn)))
#   define aligned_free(x) _aligned_free(x)
# endif
#endif

#ifdef DEBUG
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif


#ifndef __has_feature
# define __has_feature(x) 0
#endif
#ifndef __has_extension
# define __has_extension __has_feature
#endif
#define ISAL_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

#if (defined(__ICC) || defined( __GNUC__ ) || defined(__clang__)) && !defined(ISAL_UNIT_TEST)
# if __has_extension(attribute_deprecated_with_message) \
	|| (ISAL_GCC_VERSION >= 40500) \
	|| (__INTEL_COMPILER >= 1100)
#   define ISAL_DEPRECATED(message) __attribute__(( deprecated( message )))
# else
#   define ISAL_DEPRECATED(message) __attribute__(( deprecated ))
# endif
#elif (defined( __ICL ) || defined(_MSC_VER))
# if (__INTEL_COMPILER >= 1100) || (_MSC_FULL_VER >= 140050727)
#   define ISAL_DEPRECATED(message) __declspec( deprecated ( message ))
# else
#   define ISAL_DEPRECATED(message) __declspec( deprecated )
# endif
#else
# define ISAL_DEPRECATED(message)
#endif

#define ISAL_EXPERIMENTAL(message) ISAL_DEPRECATED("Experimental: " message)

#ifdef __cplusplus
}
#endif

#endif  //__TYPES_H
