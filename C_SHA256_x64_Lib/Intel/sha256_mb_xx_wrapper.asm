; SPDX-FileCopyrightText: © 2021 Yake Ho Foong
; SPDX-License-Identifier: BSD-3-Clause

%include "reg_sizes.asm"

extern sha256_mb_x16_avx512
extern sha256_mb_x8_avx2
extern sha256_mb_x4_avx
extern sha256_mb_x4_sse

[bits 64]
default rel
section .text

;; Code to save registers and align stack before calling the inner functions.
;; rsp not saved to stack but calculated using add and sub.
;; Inner functions are the SHA256 functions by Intel, for SSE4, AVX, AVX2 and AVX512.

; CALLEE SAVED REGISTERS / NON-VOLATILE REGISTERS BY ABI (WINDOWS & LINUX)
; https://docs.microsoft.com/en-us/cpp/build/x64-calling-convention?view=msvc-160#callercallee-saved-registers
; The x64 ABI considers registers RBX, RBP, RDI, RSI, RSP, R12, R13, R14, R15, and XMM6-XMM15 nonvolatile.
; They must be saved and restored by a function that uses them.
; For Linux refer to this:
; https://en.wikipedia.org/wiki/X86_calling_conventions#System_V_AMD64_ABI

; Instead of relying on knowledge of the inner functions on which registers are clobbered and which are not,
;  we save all 8 GPR (excluding rsp) and 10 XMM onto the stack to be safe, per x64 ABI above.
; STACK_SPACE needs to be an odd multiple of 8 so that plus return pointer, it will be multiple of 16
_XMM_SAVE_SIZE  equ 10*16
_GPR_SAVE_SIZE  equ 8*8
_ALIGN_SIZE     equ 8

_XMM_SAVE       equ 0
_GPR_SAVE       equ _XMM_SAVE + _XMM_SAVE_SIZE
STACK_SPACE     equ _GPR_SAVE + _GPR_SAVE_SIZE + _ALIGN_SIZE

%imacro WRAP_FUNC 2

	mk_global %1_wrapper, function, internal
	global %1_wrapper
	align %2
	%1_wrapper:
		endbranch
		sub     rsp, STACK_SPACE
		mov     [rsp + _GPR_SAVE + 8*0], rbx
		mov     [rsp + _GPR_SAVE + 8*3], rbp
		mov     [rsp + _GPR_SAVE + 8*4], r12
		mov     [rsp + _GPR_SAVE + 8*5], r13
		mov     [rsp + _GPR_SAVE + 8*6], r14
		mov     [rsp + _GPR_SAVE + 8*7], r15
	%ifidn __OUTPUT_FORMAT__, win64
		mov     [rsp + _GPR_SAVE + 8*1], rsi
		mov     [rsp + _GPR_SAVE + 8*2], rdi
		vmovdqa  [rsp + _XMM_SAVE + 16*0], xmm6
		vmovdqa  [rsp + _XMM_SAVE + 16*1], xmm7
		vmovdqa  [rsp + _XMM_SAVE + 16*2], xmm8
		vmovdqa  [rsp + _XMM_SAVE + 16*3], xmm9
		vmovdqa  [rsp + _XMM_SAVE + 16*4], xmm10
		vmovdqa  [rsp + _XMM_SAVE + 16*5], xmm11
		vmovdqa  [rsp + _XMM_SAVE + 16*6], xmm12
		vmovdqa  [rsp + _XMM_SAVE + 16*7], xmm13
		vmovdqa  [rsp + _XMM_SAVE + 16*8], xmm14
		vmovdqa  [rsp + _XMM_SAVE + 16*9], xmm15
	%endif

	; Linux need position independent code but win64 won't accept plt
	%ifdef __NASM_VER__
		%ifidn __OUTPUT_FORMAT__, elf64
			call %1	wrt ..plt
		%elifidn __OUTPUT_FORMAT__, win64
			call %1
		%else
			call %1
		%endif
	%else
		call %1
	%endif

	%ifidn __OUTPUT_FORMAT__, win64
		vmovdqa  xmm6, [rsp + _XMM_SAVE + 16*0]
		vmovdqa  xmm7, [rsp + _XMM_SAVE + 16*1]
		vmovdqa  xmm8, [rsp + _XMM_SAVE + 16*2]
		vmovdqa  xmm9, [rsp + _XMM_SAVE + 16*3]
		vmovdqa  xmm10, [rsp + _XMM_SAVE + 16*4]
		vmovdqa  xmm11, [rsp + _XMM_SAVE + 16*5]
		vmovdqa  xmm12, [rsp + _XMM_SAVE + 16*6]
		vmovdqa  xmm13, [rsp + _XMM_SAVE + 16*7]
		vmovdqa  xmm14, [rsp + _XMM_SAVE + 16*8]
		vmovdqa  xmm15, [rsp + _XMM_SAVE + 16*9]
		mov     rsi, [rsp + _GPR_SAVE + 8*1]
		mov     rdi, [rsp + _GPR_SAVE + 8*2]
	%endif
		mov     rbx, [rsp + _GPR_SAVE + 8*0]
		mov     rbp, [rsp + _GPR_SAVE + 8*3]
		mov     r12, [rsp + _GPR_SAVE + 8*4]
		mov     r13, [rsp + _GPR_SAVE + 8*5]
		mov     r14, [rsp + _GPR_SAVE + 8*6]
		mov     r15, [rsp + _GPR_SAVE + 8*7]
		add     rsp, STACK_SPACE

		ret

%endmacro

WRAP_FUNC sha256_mb_x16_avx512, 64
WRAP_FUNC sha256_mb_x8_avx2, 64
WRAP_FUNC sha256_mb_x4_avx, 64
WRAP_FUNC sha256_mb_x4_sse, 64

