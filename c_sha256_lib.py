# SPDX-FileCopyrightText: © 2021 Yake Ho Foong
# SPDX-License-Identifier: BSD-3-Clause

"""Python wrapper for the C shared library for SHA256 mining."""
import os
import sys
import platform
import ctypes
import ctypes.util
from typing import Tuple, Union
from enum import Enum
import time


class PreferredAccelerationInC(Enum):
    """Enums to request the C function to use x64 extensions."""

    AVX512 = 0
    SHA = 1
    AVX2 = 2
    AVX = 3
    SSE = 4
    NO_ACCEL = 5


# current applicaiton folder
curr_app_folder = os.path.dirname(
    os.path.abspath(__file__))

# flag if system is 64-bit
is_64bit_system = sys.maxsize > 2**32

# if Windows, we need to use LoadLibraryEx because
# ctypes.CDLL cannot load DLL file in current folder
if platform.system() == 'Windows' and is_64bit_system:
    from ctypes import wintypes

    kernel32 = ctypes.WinDLL('kernel32', use_last_error=True)

    def check_bool(result, func, args):
        """Check for error."""
        if not result:
            raise ctypes.WinError(ctypes.get_last_error())
        return args

    kernel32.LoadLibraryExW.errcheck = check_bool
    kernel32.LoadLibraryExW.restype = wintypes.HMODULE
    kernel32.LoadLibraryExW.argtypes = (wintypes.LPCWSTR,
                                        wintypes.HANDLE,
                                        wintypes.DWORD)

    class CDLLEx(ctypes.CDLL):
        """Necessary for calling LoadLibraryEx.

        In Windows, LoadLibraryEx is necessary if DLL is in application
        folder rather than system-wide search paths.
        """

        def __init__(self, name, mode=0, handle=None,
                     use_errno=True, use_last_error=False):
            """Construct."""
            if os.name == 'nt' and handle is None:
                handle = kernel32.LoadLibraryExW(name, None, mode)
            super(CDLLEx, self).__init__(name, mode, handle,
                                         use_errno, use_last_error)

    LOAD_WITH_ALTERED_SEARCH_PATH = 0x00000008

    dll_path = os.path.join(
        curr_app_folder, "C_SHA256_x64\\x64\\Release\\C_SHA256_x64_Lib.dll")
    mylib = CDLLEx(dll_path, LOAD_WITH_ALTERED_SEARCH_PATH)
    is_library_supported = True

elif platform.system() == 'Linux' and is_64bit_system:

    so_path = os.path.join(
        curr_app_folder, "C_SHA256_x64/C_SHA256_x64_Lib/build/mine_xcoin.so")
    mylib = ctypes.cdll.LoadLibrary(so_path)
    is_library_supported = True

else:

    is_library_supported = False

mine_xcoin = mylib.mine_xcoin
mine_xcoin.argtypes = [
    ctypes.POINTER(ctypes.c_ubyte), ctypes.POINTER(ctypes.c_ubyte),
    ctypes.c_uint64, ctypes.c_ubyte,
    ctypes.POINTER(ctypes.c_ubyte), ctypes.POINTER(ctypes.c_uint64),
    ctypes.POINTER(ctypes.c_ubyte), ctypes.POINTER(ctypes.c_uint32)]
mine_xcoin.restype = ctypes.c_bool


def search_block_id_c(
    *,
    partial_bytes: bytes,
    difficulty: int,
    preferred_accel: PreferredAccelerationInC,
    cutoff_time: float) \
        -> Union[Tuple[bytes, int], Tuple[None, None]]:
    """Search for a valid `block_id` by trying different values of nonce.

    Only named arguments, no positional arguments, to be safe.

    Calls external C function using DLL (Windows) or dot o (Linux).

    Uses x64 instruction set extensions if requested and if available.

    Parameters
    ----------
    partial_bytes : bytes
        The bytes where append nonce and apply SHA-256 becomes block ID.
    difficulty : int
        The difficulty.
    preferred_accel : PreferredAccelerationInC
        The preferred acceleration, see enum class `PreferredAccelerationInC`.
    cutoff_time : float
        A floating point number returned by time(), seconds from UTC,
        to give up mining if still no success after cutoff_time.

    Returns
    ----------
    (bytes, int, float): block ID and nonce that satisfies difficulty,
                         and time used in seconds
    (None, None, None): if search failed
    """
    target = min(2 ** 256 // difficulty, 2**256 - 1)

    # inputs
    target_arr = \
        (ctypes.c_ubyte * 32)(*target.to_bytes
                              (32, byteorder='big', signed=False))
    msg = (ctypes.c_ubyte * len(partial_bytes))(*partial_bytes)
    # results
    results_arr = (ctypes.c_ubyte * 32)(0)
    nonce_arr = (ctypes.c_uint64 * 1)(0)
    accel_used = (ctypes.c_ubyte * 1)(0)
    num_threads_used = (ctypes.c_uint32 * 1)(0)

    tic = time.perf_counter()

    # special input
    remaining_time = cutoff_time - time.time()

    status = mylib.mine_xcoin(
        target_arr,
        msg,
        (ctypes.c_uint64)(len(partial_bytes)),
        (ctypes.c_ubyte)(preferred_accel.value),
        results_arr,
        nonce_arr,
        accel_used,
        num_threads_used,
        (ctypes.c_double)(remaining_time))

    toc = time.perf_counter()
    seconds_used = toc - tic

    if status:
        results = ctypes.string_at(results_arr, 32)
        nonce = int(nonce_arr[0])

        print(f"\n\nNumber of threads used: {int(num_threads_used[0])}\n")
        accel_method = ['AVX 512', 'SHA', 'AVX2', 'AVX', 'SSE 4.1',
                        'No x64 instruction set extension used']
        print(f"Acceleration used: {accel_method[int(accel_used[0])]}\n")

        return (results, nonce, seconds_used)
    else:
        return (None, None, None)
