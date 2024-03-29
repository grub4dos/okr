/*
 * Original copyright notice:
 * 
 * HMAC-SHA-224/256/384/512 implementation
 * Last update: 06/15/2005
 * Issue date:  06/15/2005
 *
 * Copyright (C) 2005 Olivier Gay <olivier.gay@a3.epfl.ch>
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

#include <string.h>

#include "hmac_sha3.h"

/* HMAC-SHA3-224 functions */

void myc_hmac_sha3_224_init(myc_hmac_sha3_224_ctx *ctx, const unsigned char *key,
                      unsigned int key_size)
{
    unsigned int fill;
    unsigned int num;

    const unsigned char *key_used;
    unsigned char key_temp[MYC_SHA3_224_DIGEST_LENGTH];
    int i;

    if (key_size == MYC_SHA3_224_BLOCK_LENGTH) {
        key_used = key;
        num = MYC_SHA3_224_BLOCK_LENGTH;
    } else {
        if (key_size > MYC_SHA3_224_BLOCK_LENGTH){
            num = MYC_SHA3_224_DIGEST_LENGTH;
            myc_sha3_224(key, key_size, key_temp);
            key_used = key_temp;
        } else { /* key_size > MYC_SHA3_224_BLOCK_LENGTH */
            key_used = key;
            num = key_size;
        }
        fill = MYC_SHA3_224_BLOCK_LENGTH - num;

        memset(ctx->block_ipad + num, 0x36, fill);
        memset(ctx->block_opad + num, 0x5c, fill);
    }

    for (i = 0; i < (int) num; i++) {
        ctx->block_ipad[i] = key_used[i] ^ 0x36;
        ctx->block_opad[i] = key_used[i] ^ 0x5c;
    }

    myc_sha3_224_Init(&ctx->ctx_inside);
    myc_sha3_Update(&ctx->ctx_inside, ctx->block_ipad, MYC_SHA3_224_BLOCK_LENGTH);

    myc_sha3_224_Init(&ctx->ctx_outside);
    myc_sha3_Update(&ctx->ctx_outside, ctx->block_opad,
                  MYC_SHA3_224_BLOCK_LENGTH);

    /* for hmac_reinit */
    memcpy(&ctx->ctx_inside_reinit, &ctx->ctx_inside,
           sizeof(MYC_SHA3_CTX));
    memcpy(&ctx->ctx_outside_reinit, &ctx->ctx_outside,
           sizeof(MYC_SHA3_CTX));
}

void myc_hmac_sha3_224_reinit(myc_hmac_sha3_224_ctx *ctx)
{
    memcpy(&ctx->ctx_inside, &ctx->ctx_inside_reinit,
           sizeof(MYC_SHA3_CTX));
    memcpy(&ctx->ctx_outside, &ctx->ctx_outside_reinit,
           sizeof(MYC_SHA3_CTX));
}

void myc_hmac_sha3_224_update(myc_hmac_sha3_224_ctx *ctx, const unsigned char *message,
                        unsigned int message_len)
{
    myc_sha3_Update(&ctx->ctx_inside, message, message_len);
}

void myc_hmac_sha3_224_final(myc_hmac_sha3_224_ctx *ctx, unsigned char *mac,
                       unsigned int mac_size)
{
    unsigned char digest_inside[MYC_SHA3_224_DIGEST_LENGTH];
    unsigned char mac_temp[MYC_SHA3_224_DIGEST_LENGTH];

    myc_sha3_Final(&ctx->ctx_inside, digest_inside);
    myc_sha3_Update(&ctx->ctx_outside, digest_inside, MYC_SHA3_224_DIGEST_LENGTH);
    myc_sha3_Final(&ctx->ctx_outside, mac_temp);
    memcpy(mac, mac_temp, mac_size);
}

void myc_hmac_sha3_224(const unsigned char *key, unsigned int key_size,
          const unsigned char *message, unsigned int message_len,
          unsigned char *mac, unsigned mac_size)
{
    myc_hmac_sha3_224_ctx ctx;

    myc_hmac_sha3_224_init(&ctx, key, key_size);
    myc_hmac_sha3_224_update(&ctx, message, message_len);
    myc_hmac_sha3_224_final(&ctx, mac, mac_size);
}

/* HMAC-SHA3-256 functions */

void myc_hmac_sha3_256_init(myc_hmac_sha3_256_ctx *ctx, const unsigned char *key,
                      unsigned int key_size)
{
    unsigned int fill;
    unsigned int num;

    const unsigned char *key_used;
    unsigned char key_temp[MYC_SHA3_256_DIGEST_LENGTH];
    int i;

    if (key_size == MYC_SHA3_256_BLOCK_LENGTH) {
        key_used = key;
        num = MYC_SHA3_256_BLOCK_LENGTH;
    } else {
        if (key_size > MYC_SHA3_256_BLOCK_LENGTH){
            num = MYC_SHA3_256_DIGEST_LENGTH;
            myc_sha3_256(key, key_size, key_temp);
            key_used = key_temp;
        } else { /* key_size > MYC_SHA3_256_BLOCK_LENGTH */
            key_used = key;
            num = key_size;
        }
        fill = MYC_SHA3_256_BLOCK_LENGTH - num;

        memset(ctx->block_ipad + num, 0x36, fill);
        memset(ctx->block_opad + num, 0x5c, fill);
    }

    for (i = 0; i < (int) num; i++) {
        ctx->block_ipad[i] = key_used[i] ^ 0x36;
        ctx->block_opad[i] = key_used[i] ^ 0x5c;
    }

    myc_sha3_256_Init(&ctx->ctx_inside);
    myc_sha3_Update(&ctx->ctx_inside, ctx->block_ipad, MYC_SHA3_256_BLOCK_LENGTH);

    myc_sha3_256_Init(&ctx->ctx_outside);
    myc_sha3_Update(&ctx->ctx_outside, ctx->block_opad,
                  MYC_SHA3_256_BLOCK_LENGTH);

    /* for hmac_reinit */
    memcpy(&ctx->ctx_inside_reinit, &ctx->ctx_inside,
           sizeof(MYC_SHA3_CTX));
    memcpy(&ctx->ctx_outside_reinit, &ctx->ctx_outside,
           sizeof(MYC_SHA3_CTX));
}

void myc_hmac_sha3_256_reinit(myc_hmac_sha3_256_ctx *ctx)
{
    memcpy(&ctx->ctx_inside, &ctx->ctx_inside_reinit,
           sizeof(MYC_SHA3_CTX));
    memcpy(&ctx->ctx_outside, &ctx->ctx_outside_reinit,
           sizeof(MYC_SHA3_CTX));
}

void myc_hmac_sha3_256_update(myc_hmac_sha3_256_ctx *ctx, const unsigned char *message,
                        unsigned int message_len)
{
    myc_sha3_Update(&ctx->ctx_inside, message, message_len);
}

void myc_hmac_sha3_256_final(myc_hmac_sha3_256_ctx *ctx, unsigned char *mac,
                       unsigned int mac_size)
{
    unsigned char digest_inside[MYC_SHA3_256_DIGEST_LENGTH];
    unsigned char mac_temp[MYC_SHA3_256_DIGEST_LENGTH];

    myc_sha3_Final(&ctx->ctx_inside, digest_inside);
    myc_sha3_Update(&ctx->ctx_outside, digest_inside, MYC_SHA3_256_DIGEST_LENGTH);
    myc_sha3_Final(&ctx->ctx_outside, mac_temp);
    memcpy(mac, mac_temp, mac_size);
}

void myc_hmac_sha3_256(const unsigned char *key, unsigned int key_size,
          const unsigned char *message, unsigned int message_len,
          unsigned char *mac, unsigned mac_size)
{
    myc_hmac_sha3_256_ctx ctx;

    myc_hmac_sha3_256_init(&ctx, key, key_size);
    myc_hmac_sha3_256_update(&ctx, message, message_len);
    myc_hmac_sha3_256_final(&ctx, mac, mac_size);
}

/* HMAC-SHA3-384 functions */

void myc_hmac_sha3_384_init(myc_hmac_sha3_384_ctx *ctx, const unsigned char *key,
                      unsigned int key_size)
{
    unsigned int fill;
    unsigned int num;

    const unsigned char *key_used;
    unsigned char key_temp[MYC_SHA3_384_DIGEST_LENGTH];
    int i;

    if (key_size == MYC_SHA3_384_BLOCK_LENGTH) {
        key_used = key;
        num = MYC_SHA3_384_BLOCK_LENGTH;
    } else {
        if (key_size > MYC_SHA3_384_BLOCK_LENGTH){
            num = MYC_SHA3_384_DIGEST_LENGTH;
            myc_sha3_384(key, key_size, key_temp);
            key_used = key_temp;
        } else { /* key_size > MYC_SHA3_384_BLOCK_LENGTH */
            key_used = key;
            num = key_size;
        }
        fill = MYC_SHA3_384_BLOCK_LENGTH - num;

        memset(ctx->block_ipad + num, 0x36, fill);
        memset(ctx->block_opad + num, 0x5c, fill);
    }

    for (i = 0; i < (int) num; i++) {
        ctx->block_ipad[i] = key_used[i] ^ 0x36;
        ctx->block_opad[i] = key_used[i] ^ 0x5c;
    }

    myc_sha3_384_Init(&ctx->ctx_inside);
    myc_sha3_Update(&ctx->ctx_inside, ctx->block_ipad, MYC_SHA3_384_BLOCK_LENGTH);

    myc_sha3_384_Init(&ctx->ctx_outside);
    myc_sha3_Update(&ctx->ctx_outside, ctx->block_opad,
                  MYC_SHA3_384_BLOCK_LENGTH);

    /* for hmac_reinit */
    memcpy(&ctx->ctx_inside_reinit, &ctx->ctx_inside,
           sizeof(MYC_SHA3_CTX));
    memcpy(&ctx->ctx_outside_reinit, &ctx->ctx_outside,
           sizeof(MYC_SHA3_CTX));
}

void myc_hmac_sha3_384_reinit(myc_hmac_sha3_384_ctx *ctx)
{
    memcpy(&ctx->ctx_inside, &ctx->ctx_inside_reinit,
           sizeof(MYC_SHA3_CTX));
    memcpy(&ctx->ctx_outside, &ctx->ctx_outside_reinit,
           sizeof(MYC_SHA3_CTX));
}

void myc_hmac_sha3_384_update(myc_hmac_sha3_384_ctx *ctx, const unsigned char *message,
                        unsigned int message_len)
{
    myc_sha3_Update(&ctx->ctx_inside, message, message_len);
}

void myc_hmac_sha3_384_final(myc_hmac_sha3_384_ctx *ctx, unsigned char *mac,
                       unsigned int mac_size)
{
    unsigned char digest_inside[MYC_SHA3_384_DIGEST_LENGTH];
    unsigned char mac_temp[MYC_SHA3_384_DIGEST_LENGTH];

    myc_sha3_Final(&ctx->ctx_inside, digest_inside);
    myc_sha3_Update(&ctx->ctx_outside, digest_inside, MYC_SHA3_384_DIGEST_LENGTH);
    myc_sha3_Final(&ctx->ctx_outside, mac_temp);
    memcpy(mac, mac_temp, mac_size);
}

void myc_hmac_sha3_384(const unsigned char *key, unsigned int key_size,
          const unsigned char *message, unsigned int message_len,
          unsigned char *mac, unsigned mac_size)
{
    myc_hmac_sha3_384_ctx ctx;

    myc_hmac_sha3_384_init(&ctx, key, key_size);
    myc_hmac_sha3_384_update(&ctx, message, message_len);
    myc_hmac_sha3_384_final(&ctx, mac, mac_size);
}

/* HMAC-SHA3-512 functions */

void myc_hmac_sha3_512_init(myc_hmac_sha3_512_ctx *ctx, const unsigned char *key,
                      unsigned int key_size)
{
    unsigned int fill;
    unsigned int num;

    const unsigned char *key_used;
    unsigned char key_temp[MYC_SHA3_512_DIGEST_LENGTH];
    int i;

    if (key_size == MYC_SHA3_512_BLOCK_LENGTH) {
        key_used = key;
        num = MYC_SHA3_512_BLOCK_LENGTH;
    } else {
        if (key_size > MYC_SHA3_512_BLOCK_LENGTH){
            num = MYC_SHA3_512_DIGEST_LENGTH;
            myc_sha3_512(key, key_size, key_temp);
            key_used = key_temp;
        } else { /* key_size > MYC_SHA3_512_BLOCK_LENGTH */
            key_used = key;
            num = key_size;
        }
        fill = MYC_SHA3_512_BLOCK_LENGTH - num;

        memset(ctx->block_ipad + num, 0x36, fill);
        memset(ctx->block_opad + num, 0x5c, fill);
    }

    for (i = 0; i < (int) num; i++) {
        ctx->block_ipad[i] = key_used[i] ^ 0x36;
        ctx->block_opad[i] = key_used[i] ^ 0x5c;
    }

    myc_sha3_512_Init(&ctx->ctx_inside);
    myc_sha3_Update(&ctx->ctx_inside, ctx->block_ipad, MYC_SHA3_512_BLOCK_LENGTH);

    myc_sha3_512_Init(&ctx->ctx_outside);
    myc_sha3_Update(&ctx->ctx_outside, ctx->block_opad,
                  MYC_SHA3_512_BLOCK_LENGTH);

    /* for hmac_reinit */
    memcpy(&ctx->ctx_inside_reinit, &ctx->ctx_inside,
           sizeof(MYC_SHA3_CTX));
    memcpy(&ctx->ctx_outside_reinit, &ctx->ctx_outside,
           sizeof(MYC_SHA3_CTX));
}

void myc_hmac_sha3_512_reinit(myc_hmac_sha3_512_ctx *ctx)
{
    memcpy(&ctx->ctx_inside, &ctx->ctx_inside_reinit,
           sizeof(MYC_SHA3_CTX));
    memcpy(&ctx->ctx_outside, &ctx->ctx_outside_reinit,
           sizeof(MYC_SHA3_CTX));
}

void myc_hmac_sha3_512_update(myc_hmac_sha3_512_ctx *ctx, const unsigned char *message,
                        unsigned int message_len)
{
    myc_sha3_Update(&ctx->ctx_inside, message, message_len);
}

void myc_hmac_sha3_512_final(myc_hmac_sha3_512_ctx *ctx, unsigned char *mac,
                       unsigned int mac_size)
{
    unsigned char digest_inside[MYC_SHA3_512_DIGEST_LENGTH];
    unsigned char mac_temp[MYC_SHA3_512_DIGEST_LENGTH];

    myc_sha3_Final(&ctx->ctx_inside, digest_inside);
    myc_sha3_Update(&ctx->ctx_outside, digest_inside, MYC_SHA3_512_DIGEST_LENGTH);
    myc_sha3_Final(&ctx->ctx_outside, mac_temp);
    memcpy(mac, mac_temp, mac_size);
}

void myc_hmac_sha3_512(const unsigned char *key, unsigned int key_size,
          const unsigned char *message, unsigned int message_len,
          unsigned char *mac, unsigned mac_size)
{
    myc_hmac_sha3_512_ctx ctx;

    myc_hmac_sha3_512_init(&ctx, key, key_size);
    myc_hmac_sha3_512_update(&ctx, message, message_len);
    myc_hmac_sha3_512_final(&ctx, mac, mac_size);
}