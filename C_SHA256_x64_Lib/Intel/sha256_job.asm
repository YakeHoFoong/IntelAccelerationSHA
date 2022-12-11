; SPDX-FileCopyrightText: © 2021 Yake Ho Foong
; SPDX-FileCopyrightText: Copyright(c) 2011-2016 Intel Corporation All rights reserved.
; SPDX-License-Identifier: BSD-3-Clause

%include "datastruct.asm"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; Define constants
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%define STS_UNKNOWN		0
%define STS_BEING_PROCESSED	1
%define STS_COMPLETED		2

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; Threshold constants
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; if number of lanes in use <= threshold, using sb func
%define SHA256_SB_THRESHOLD_SSE		1
%define SHA256_SB_THRESHOLD_AVX		1
%define SHA256_SB_THRESHOLD_AVX2	1
%define SHA256_SB_THRESHOLD_AVX512	1
%define SHA256_NI_SB_THRESHOLD_SSE	4 ; shani is faster than sse sha256_mb
%define SHA256_NI_SB_THRESHOLD_AVX512	6

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; Define SHA256_JOB structure
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

START_FIELDS	; SHA256_JOB

;;;	name				size	align
FIELD	_buffer,			8,	8	; pointer to buffer
FIELD	_len,				8,	8	; length in bytes
FIELD	_result_digest,			8*4,	64	; Digest (output)
FIELD	_status,			4,	4
FIELD	_user_data,			8,	8

%assign _SHA256_JOB_size	_FIELD_OFFSET
%assign _SHA256_JOB_align	_STRUCT_ALIGN
