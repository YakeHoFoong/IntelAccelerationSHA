/* SPDX-FileCopyrightText: Copyright(c) 2011-2016 Intel Corporation All rights reserved.
   SPDX-License-Identifier: BSD-3-Clause */

#ifndef _MULTI_BUFFER_H_
#define _MULTI_BUFFER_H_

/**
 *  @file  multi_buffer.h
 *  @brief Multi-buffer common fields
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @enum JOB_STS
 *  @brief Job return codes
 */

typedef enum {STS_UNKNOWN = 0,	//!< STS_UNKNOWN
	STS_BEING_PROCESSED = 1,//!< STS_BEING_PROCESSED
	STS_COMPLETED = 2,	//!< STS_COMPLETED
	STS_INTERNAL_ERROR,	//!< STS_INTERNAL_ERROR
	STS_ERROR		//!< STS_ERROR
} JOB_STS;

#define HASH_MB_NO_FLAGS 0
#define HASH_MB_FIRST	1
#define HASH_MB_LAST	2

/* Common flags for the new API only
 *  */

/**
 *  @enum HASH_CTX_FLAG
 *  @brief CTX job type
 */
typedef enum {
	HASH_UPDATE	= 0x00, //!< HASH_UPDATE
	HASH_FIRST	= 0x01, //!< HASH_FIRST
	HASH_LAST	= 0x02, //!< HASH_LAST
	HASH_ENTIRE	= 0x03, //!< HASH_ENTIRE
} HASH_CTX_FLAG;

/**
 *  @enum HASH_CTX_STS
 *  @brief CTX status flags
 */
typedef enum {
	HASH_CTX_STS_IDLE	= 0x00, //!< HASH_CTX_STS_IDLE
	HASH_CTX_STS_PROCESSING	= 0x01, //!< HASH_CTX_STS_PROCESSING
	HASH_CTX_STS_LAST	= 0x02, //!< HASH_CTX_STS_LAST
	HASH_CTX_STS_COMPLETE	= 0x04, //!< HASH_CTX_STS_COMPLETE
} HASH_CTX_STS;

/**
 *  @enum HASH_CTX_ERROR
 *  @brief CTX error flags
 */
typedef enum {
	HASH_CTX_ERROR_NONE			=  0, //!< HASH_CTX_ERROR_NONE
	HASH_CTX_ERROR_INVALID_FLAGS		= -1, //!< HASH_CTX_ERROR_INVALID_FLAGS
	HASH_CTX_ERROR_ALREADY_PROCESSING	= -2, //!< HASH_CTX_ERROR_ALREADY_PROCESSING
	HASH_CTX_ERROR_ALREADY_COMPLETED	= -3, //!< HASH_CTX_ERROR_ALREADY_COMPLETED
} HASH_CTX_ERROR;


#define hash_ctx_user_data(ctx)  ((ctx)->user_data)
#define hash_ctx_digest(ctx)     ((ctx)->job.result_digest)
#define hash_ctx_processing(ctx) ((ctx)->status & HASH_CTX_STS_PROCESSING)
#define hash_ctx_complete(ctx)   ((ctx)->status == HASH_CTX_STS_COMPLETE)
#define hash_ctx_status(ctx)     ((ctx)->status)
#define hash_ctx_error(ctx)      ((ctx)->error)
#define hash_ctx_init(ctx) \
	do { \
		(ctx)->error = HASH_CTX_ERROR_NONE; \
		(ctx)->status = HASH_CTX_STS_COMPLETE; \
	} while(0)

#ifdef __cplusplus
}
#endif

#endif // _MULTI_BUFFER_H_
