; SPDX-FileCopyrightText: © 2021 Yake Ho Foong
; SPDX-FileCopyrightText: Copyright(c) 2011-2016 Intel Corporation All rights reserved.
; SPDX-License-Identifier: BSD-3-Clause

%include "datastruct.asm"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; Define SHA256 Out Of Order Data Structures
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

START_FIELDS    ; LANE_DATA
;;;     name            size    align
FIELD   _job_in_lane,   8,      8       ; pointer to job object
END_FIELDS

%assign _LANE_DATA_size _FIELD_OFFSET
%assign _LANE_DATA_align        _STRUCT_ALIGN

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

START_FIELDS    ; SHA256_ARGS_X16
;;;     name            size    align
FIELD   _digest,        4*8*16,  4       ; transposed digest
FIELD   _data_ptr,      8*16,    8       ; array of pointers to data
END_FIELDS

%assign _SHA256_ARGS_X4_size    _FIELD_OFFSET
%assign _SHA256_ARGS_X4_align   _STRUCT_ALIGN
%assign _SHA256_ARGS_X8_size	_FIELD_OFFSET
%assign _SHA256_ARGS_X8_align	_STRUCT_ALIGN
%assign _SHA256_ARGS_X16_size	_FIELD_OFFSET
%assign _SHA256_ARGS_X16_align	_STRUCT_ALIGN

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

START_FIELDS    ; MB_MGR
;;;     name            size    align
FIELD   _args,          _SHA256_ARGS_X4_size, _SHA256_ARGS_X4_align
FIELD   _lens,          4*16,    8
FIELD   _unused_lanes,  8,      8
FIELD   _ldata,         _LANE_DATA_size*16, _LANE_DATA_align
FIELD   _num_lanes_inuse, 4,    4
END_FIELDS

%assign _MB_MGR_size    _FIELD_OFFSET
%assign _MB_MGR_align   _STRUCT_ALIGN

_args_digest    equ     _args + _digest
_args_data_ptr  equ     _args + _data_ptr
