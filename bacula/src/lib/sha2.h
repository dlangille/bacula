/*
 * FIPS 180-2 SHA-224/256/384/512 implementation
 * Last update: 02/02/2007
 * Issue date:  04/30/2005
 *
 * Copyright (C) 2005, 2007 Olivier Gay <olivier.gay@a3.epfl.ch>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef SHA2_H
#define SHA2_H

#include "bacula.h"

#ifndef HAVE_SHA2

#define SHA224_DIGEST_SIZE ( 224 / 8)
#define SHA256_DIGEST_SIZE ( 256 / 8)
#define SHA384_DIGEST_SIZE ( 384 / 8)
#define SHA512_DIGEST_SIZE ( 512 / 8)

#define SHA256_BLOCK_SIZE  ( 512 / 8)
#define SHA512_BLOCK_SIZE  (1024 / 8)
#define SHA384_BLOCK_SIZE  SHA512_BLOCK_SIZE
#define SHA224_BLOCK_SIZE  SHA256_BLOCK_SIZE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned int tot_len;
    unsigned int len;
    unsigned char block[2 * SHA256_BLOCK_SIZE];
    uint32_t h[8];
} sha256_ctx;

typedef struct {
    unsigned int tot_len;
    unsigned int len;
    unsigned char block[2 * SHA512_BLOCK_SIZE];
    uint64_t h[8];
} sha512_ctx;

typedef sha512_ctx sha384_ctx;
typedef sha256_ctx sha224_ctx;

void sha224_init(sha224_ctx *ctx);
void sha224_update(sha224_ctx *ctx, const unsigned char *message,
                   unsigned int len);
void sha224_final(sha224_ctx *ctx, unsigned char *digest);
void sha224(const unsigned char *message, unsigned int len,
            unsigned char *digest);

void sha256_init(sha256_ctx * ctx);
void sha256_update(sha256_ctx *ctx, const unsigned char *message,
                   unsigned int len);
void sha256_final(sha256_ctx *ctx, unsigned char *digest);
void sha256(const unsigned char *message, unsigned int len,
            unsigned char *digest);

void sha384_init(sha384_ctx *ctx);
void sha384_update(sha384_ctx *ctx, const unsigned char *message,
                   unsigned int len);
void sha384_final(sha384_ctx *ctx, unsigned char *digest);
void sha384(const unsigned char *message, unsigned int len,
            unsigned char *digest);

void sha512_init(sha512_ctx *ctx);
void sha512_update(sha512_ctx *ctx, const unsigned char *message,
                   unsigned int len);
void sha512_final(sha512_ctx *ctx, unsigned char *digest);
void sha512(const unsigned char *message, unsigned int len,
            unsigned char *digest);

#ifdef __cplusplus
}
#endif

/* implement the openssl fall back */
typedef sha256_ctx SHA256_CTX;
#define SHA256_Init(ctx) sha256_init(ctx)
#define SHA256_Update(ctx, msg, len) sha256_update(ctx, (const unsigned char *)(msg), len)
#define SHA256_Final(digest, ctx) sha256_final(ctx, digest)

typedef sha512_ctx SHA512_CTX;
#define SHA512_Init(ctx) sha512_init(ctx)
#define SHA512_Update(ctx, msg, len) sha512_update(ctx, (const unsigned char *)(msg), len)
#define SHA512_Final(digest, ctx) sha512_final(ctx, digest)

#endif /* HAVE_SHA2 */

#endif /* !SHA2_H */

