// SPDX-FileCopyrightText: © 2021 Yake Ho Foong
// SPDX-License-Identifier: BSD-3-Clause

// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include <atomic>
#include <cstring>
#include <thread>
#include <vector>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <climits>
#include <bitset>
#include <array>
#include <string>

#if defined(__GNUC__)
#include <cstdlib>
#endif
#if defined(_MSC_VER ) && defined(_WIN64)
#include <malloc.h>
#endif

#endif //PCH_H
