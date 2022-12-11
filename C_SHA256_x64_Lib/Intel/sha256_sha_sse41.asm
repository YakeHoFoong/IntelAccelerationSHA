; SPDX-FileCopyrightText: © 2021 Yake Ho Foong
; SPDX-FileCopyrightText: Copyright(c) 2011-2017 Intel Corporation All rights reserved.
; SPDX-License-Identifier: BSD-3-Clause

%include "sha256_mb_mgr_datastruct.asm"
%include "reg_sizes.asm"

[bits 64]
default rel
section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%ifidn __OUTPUT_FORMAT__, elf64
 ; Linux
 %define arg0  rdi
 %define arg1  rsi
 %define arg2  rdx
%else
 ; Windows
 %define arg0   rcx
 %define arg1   rdx
 %define arg2   r8
%endif

%define MSG     	xmm0
%define STATE0  	xmm1
%define STATE1  	xmm2
%define MSGTMP0 	xmm3
%define MSGTMP1 	xmm4
%define MSGTMP2 	xmm5
%define MSGTMP3 	xmm6
%define MSGTMP4 	xmm7

%define SHUF_MASK       xmm8

%define ABEF_SAVE       xmm9
%define CDGH_SAVE       xmm10

; arg index is start from 0 while MGR_flush/submit is from 1
%define DIGEST_ARG     arg0
%define DATA_ARG    arg1
%define NBLK    arg2

%define DPTR    r11     ; local variable -- input buffer pointer
%define TMP     xmm0      ; local variable -- assistant to address digest
%define TBL     rax

_XMM_SAVE_SIZE  equ 10*16
_GPR_SAVE_SIZE  equ 0
_ALIGN_SIZE     equ 8

_XMM_SAVE       equ 0
_GPR_SAVE       equ _XMM_SAVE + _XMM_SAVE_SIZE
STACK_SPACE     equ _GPR_SAVE + _GPR_SAVE_SIZE + _ALIGN_SIZE

align 32

; void sha256_sha_sse41(uint32_t state[8], const uint8_t data[], uint32_t size_in_blocks)
; arg 0 : DIGEST_ARG : pointer to digest
; arg 1 : DIGEST_ARG : pointer to data
; arg 2 : NBLK : size (in blocks) ;; assumed to be >= 1
;
; Clobbers registers: rax, r9~r11, xmm0-xmm10
;
mk_global sha256_sha_sse41, function, internal
sha256_sha_sse41:
	endbranch
%ifidn __OUTPUT_FORMAT__, win64
	sub     rsp, STACK_SPACE
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
	
	shl     NBLK, 6 	; transform blk amount into bytes
	jz      backto_MGR

	;; Initialize digest
	;; digests -> ABEF(state0), CDGH(state1)
    ; TMP = _mm_loadu_si128((const __m128i*) &state[0]);
	movdqu	TMP, [DIGEST_ARG]			; ABCD
    ; STATE1 = _mm_loadu_si128((const __m128i*) &state[4]);
	movdqu	STATE1, [DIGEST_ARG + 16]	; EFGH
    ; TMP = _mm_shuffle_epi32(TMP, 0xB1);          /* CDAB */
	pshufd	TMP, TMP, 0xB1	; 0xB1 order operand: X2, X3, X0, X1 -> B, A, D, C
    ; STATE1 = _mm_shuffle_epi32(STATE1, 0x1B);    /* EFGH */
	pshufd	STATE1, STATE1, 0x1B	; 0x1B order operand: X0, X1, X2, X3 -> H, G, F, E
    ; STATE0 = _mm_alignr_epi8(TMP, STATE1, 8);    /* ABEF */
	movdqu STATE0, TMP
	palignr	STATE0, STATE1, 8	; concat STATE0 & STATE1, shift right 8 byte, put right 16 bytes into dest -> F, E, B, A
    ; STATE1 = _mm_blend_epi16(STATE1, TMP, 0xF0); /* CDGH */
	pblendw STATE1, TMP, 0xF0	; conditional overwrite
	
	;; Load table constants masks
	movdqa  SHUF_MASK, [PSHUFFLE_SHANI_MASK]
	lea     TBL, [TABLE]

	;; Load input pointers
	mov     DPTR, DATA_ARG
	;; nblk is used to indicate data end
	add     NBLK, DPTR

lloop:
	; /* Save hash values for addition after rounds */
	movdqa  	ABEF_SAVE, STATE0
	movdqa  	CDGH_SAVE, STATE1

	; /* Rounds 0-3 */
	movdqu  	MSG, [DPTR + 0*16]
	pshufb  	MSG, SHUF_MASK
	movdqa  	MSGTMP0, MSG
		paddd   	MSG, [TBL + 0*16]
		sha256rnds2     STATE1, STATE0, MSG
		pshufd  	MSG, MSG, 0x0E
		sha256rnds2     STATE0, STATE1, MSG

	; /* Rounds 4-7 */
	movdqu  	MSG, [DPTR + 1*16]
	pshufb  	MSG, SHUF_MASK
	movdqa  	MSGTMP1, MSG
		paddd   	MSG, [TBL + 1*16]
		sha256rnds2     STATE1, STATE0, MSG
		pshufd  	MSG, MSG, 0x0E
		sha256rnds2     STATE0, STATE1, MSG
	sha256msg1      MSGTMP0, MSGTMP1

	; /* Rounds 8-11 */
	movdqu  	MSG, [DPTR + 2*16]
	pshufb  	MSG, SHUF_MASK
	movdqa  	MSGTMP2, MSG
		paddd   	MSG, [TBL + 2*16]
		sha256rnds2     STATE1, STATE0, MSG
		pshufd  	MSG, MSG, 0x0E
		sha256rnds2     STATE0, STATE1, MSG
	sha256msg1      MSGTMP1, MSGTMP2

	; /* Rounds 12-15 */
	movdqu  	MSG, [DPTR + 3*16]
	pshufb  	MSG, SHUF_MASK
	movdqa  	MSGTMP3, MSG
		paddd   	MSG, [TBL + 3*16]
		sha256rnds2     STATE1, STATE0, MSG
	movdqa  	MSGTMP4, MSGTMP3
	palignr 	MSGTMP4, MSGTMP2, 4
	paddd   	MSGTMP0, MSGTMP4
	sha256msg2      MSGTMP0, MSGTMP3
		pshufd  	MSG, MSG, 0x0E
		sha256rnds2     STATE0, STATE1, MSG
	sha256msg1      MSGTMP2, MSGTMP3

	; /* Rounds 16-19 */
	movdqa  	MSG, MSGTMP0
		paddd   	MSG, [TBL + 4*16]
		sha256rnds2     STATE1, STATE0, MSG
	movdqa  	MSGTMP4, MSGTMP0
	palignr 	MSGTMP4, MSGTMP3, 4
	paddd   	MSGTMP1, MSGTMP4
	sha256msg2      MSGTMP1, MSGTMP0
		pshufd  	MSG, MSG, 0x0E
		sha256rnds2     STATE0, STATE1, MSG
	sha256msg1      MSGTMP3, MSGTMP0

	; /* Rounds 20-23 */
	movdqa  	MSG, MSGTMP1
		paddd   	MSG, [TBL + 5*16]
		sha256rnds2     STATE1, STATE0, MSG
	movdqa  	MSGTMP4, MSGTMP1
	palignr 	MSGTMP4, MSGTMP0, 4
	paddd   	MSGTMP2, MSGTMP4
	sha256msg2      MSGTMP2, MSGTMP1
		pshufd  	MSG, MSG, 0x0E
		sha256rnds2     STATE0, STATE1, MSG
	sha256msg1      MSGTMP0, MSGTMP1

	; /* Rounds 24-27 */
	movdqa  	MSG, MSGTMP2
		paddd   	MSG, [TBL + 6*16]
		sha256rnds2     STATE1, STATE0, MSG
	movdqa  	MSGTMP4, MSGTMP2
	palignr 	MSGTMP4, MSGTMP1, 4
	paddd   	MSGTMP3, MSGTMP4
	sha256msg2      MSGTMP3, MSGTMP2
		pshufd  	MSG, MSG, 0x0E
		sha256rnds2     STATE0, STATE1, MSG
	sha256msg1      MSGTMP1, MSGTMP2

	; /* Rounds 28-31 */
	movdqa  	MSG, MSGTMP3
		paddd   	MSG, [TBL + 7*16]
		sha256rnds2     STATE1, STATE0, MSG
	movdqa  	MSGTMP4, MSGTMP3
	palignr 	MSGTMP4, MSGTMP2, 4
	paddd   	MSGTMP0, MSGTMP4
	sha256msg2      MSGTMP0, MSGTMP3
		pshufd  	MSG, MSG, 0x0E
		sha256rnds2     STATE0, STATE1, MSG
	sha256msg1      MSGTMP2, MSGTMP3

	; /* Rounds 32-35 */
	movdqa  	MSG, MSGTMP0
		paddd   	MSG, [TBL + 8*16]
		sha256rnds2     STATE1, STATE0, MSG
	movdqa  	MSGTMP4, MSGTMP0
	palignr 	MSGTMP4, MSGTMP3, 4
	paddd   	MSGTMP1, MSGTMP4
	sha256msg2      MSGTMP1, MSGTMP0
		pshufd  	MSG, MSG, 0x0E
		sha256rnds2     STATE0, STATE1, MSG
	sha256msg1      MSGTMP3, MSGTMP0

	; /* Rounds 36-39 */
	movdqa  	MSG, MSGTMP1
		paddd   	MSG, [TBL + 9*16]
		sha256rnds2     STATE1, STATE0, MSG
	movdqa  	MSGTMP4, MSGTMP1
	palignr 	MSGTMP4, MSGTMP0, 4
	paddd   	MSGTMP2, MSGTMP4
	sha256msg2      MSGTMP2, MSGTMP1
		pshufd  	MSG, MSG, 0x0E
		sha256rnds2     STATE0, STATE1, MSG
	sha256msg1      MSGTMP0, MSGTMP1

	; /* Rounds 40-43 */
	movdqa  	MSG, MSGTMP2
		paddd   	MSG, [TBL + 10*16]
		sha256rnds2     STATE1, STATE0, MSG
	movdqa  	MSGTMP4, MSGTMP2
	palignr 	MSGTMP4, MSGTMP1, 4
	paddd   	MSGTMP3, MSGTMP4
	sha256msg2      MSGTMP3, MSGTMP2
		pshufd  	MSG, MSG, 0x0E
		sha256rnds2     STATE0, STATE1, MSG
	sha256msg1      MSGTMP1, MSGTMP2

	; /* Rounds 44-47 */
	movdqa  	MSG, MSGTMP3
		paddd   	MSG, [TBL + 11*16]
		sha256rnds2     STATE1, STATE0, MSG
	movdqa  	MSGTMP4, MSGTMP3
	palignr 	MSGTMP4, MSGTMP2, 4
	paddd   	MSGTMP0, MSGTMP4
	sha256msg2      MSGTMP0, MSGTMP3
		pshufd  	MSG, MSG, 0x0E
		sha256rnds2     STATE0, STATE1, MSG
	sha256msg1      MSGTMP2, MSGTMP3

	; /* Rounds 48-51 */
	movdqa  	MSG, MSGTMP0
		paddd   	MSG, [TBL + 12*16]
		sha256rnds2     STATE1, STATE0, MSG
	movdqa  	MSGTMP4, MSGTMP0
	palignr 	MSGTMP4, MSGTMP3, 4
	paddd   	MSGTMP1, MSGTMP4
	sha256msg2      MSGTMP1, MSGTMP0
		pshufd  	MSG, MSG, 0x0E
		sha256rnds2     STATE0, STATE1, MSG
	sha256msg1      MSGTMP3, MSGTMP0

	; /* Rounds 52-55 */
	movdqa  	MSG, MSGTMP1
		paddd   	MSG, [TBL + 13*16]
		sha256rnds2     STATE1, STATE0, MSG
	movdqa  	MSGTMP4, MSGTMP1
	palignr 	MSGTMP4, MSGTMP0, 4
	paddd   	MSGTMP2, MSGTMP4
	sha256msg2      MSGTMP2, MSGTMP1
		pshufd  	MSG, MSG, 0x0E
		sha256rnds2     STATE0, STATE1, MSG

	; /* Rounds 56-59 */
	movdqa  	MSG, MSGTMP2
		paddd   	MSG, [TBL + 14*16]
		sha256rnds2     STATE1, STATE0, MSG
	movdqa  	MSGTMP4, MSGTMP2
	palignr 	MSGTMP4, MSGTMP1, 4
	paddd   	MSGTMP3, MSGTMP4
	sha256msg2      MSGTMP3, MSGTMP2
		pshufd  	MSG, MSG, 0x0E
		sha256rnds2     STATE0, STATE1, MSG

	; /* Rounds 60-63 */
	movdqa  	MSG, MSGTMP3
		paddd   	MSG, [TBL + 15*16]
		sha256rnds2     STATE1, STATE0, MSG
		pshufd  	MSG, MSG, 0x0E
		sha256rnds2     STATE0, STATE1, MSG

	; /* Add current hash values with previously saved */
	paddd   	STATE0, ABEF_SAVE
	paddd   	STATE1, CDGH_SAVE

	; Increment data pointer and loop if more to process
	add     	DPTR, 64
	cmp     	DPTR, NBLK
	jne     	lloop

	;; write out digests
	;; ABEF(state0), CDGH(state1) -> digests
    ; TMP = _mm_shuffle_epi32(STATE0, 0x1B);       /* FEBA */
	pshufd TMP, STATE0, 0x1B
    ; STATE1 = _mm_shuffle_epi32(STATE1, 0xB1);    /* DCHG */
	pshufd STATE1, STATE1, 0xB1
    ; STATE0 = _mm_blend_epi16(TMP, STATE1, 0xF0); /* DCBA */
	movdqu STATE0, TMP
	pblendw STATE0, STATE1, 0xF0
    ; STATE1 = _mm_alignr_epi8(STATE1, TMP, 8);    /* ABEF */
	palignr STATE1, TMP, 8

    ; _mm_storeu_si128((__m128i*) &state[0], STATE0);
	movdqu	[DIGEST_ARG], STATE0			; ABCD
    ; _mm_storeu_si128((__m128i*) &state[4], STATE1);
	movdqu	[DIGEST_ARG + 16], STATE1	; EFGH

backto_MGR:
	;;;;;;;;;;;;;;;;
	;; Postamble
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
	add     rsp, STACK_SPACE
%endif

	ret


section .data align=16
PSHUFFLE_SHANI_MASK:    dq 0x0405060700010203, 0x0c0d0e0f08090a0b
TABLE:	dd	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5
	dd      0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5
	dd      0xd807aa98,0x12835b01,0x243185be,0x550c7dc3
	dd      0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174
	dd      0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc
	dd      0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da
	dd      0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7
	dd      0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967
	dd      0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13
	dd      0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85
	dd      0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3
	dd      0xd192e819,0xd6990624,0xf40e3585,0x106aa070
	dd      0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5
	dd      0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3
	dd      0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208
	dd      0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
