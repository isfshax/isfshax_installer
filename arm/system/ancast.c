/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include "common/types.h"
#include "video/gfx.h"
#include "common/utils.h"
#include "system/memory.h"

#include <stdio.h>
#include <string.h>
#include <sys/errno.h>

#include "crypto/sha.h"
#include "crypto/aes.h"
#include "crypto/crypto.h"
#include "system/smc.h"

#define ANCAST_MAGIC (0xEFA282D9l)
#define ANCAST_TARGET_IOP (0x02)
#define ANCAST_TARGET_PPC (0x01)

typedef struct {
    u16 unk1;
    u8 unk2;
    u8 unk3;
    u32 device;
    u32 type;
    u32 body_size;
    u32 body_hash[SHA_HASH_WORDS];
    u32 version;
    u8 padding[0x38];
} ancast_header;

typedef struct {
    ancast_header header;
    size_t header_size;
    FILE* file;
    const char* path;
    size_t size;
    void* load;
    void* body;
} ancast_ctx;

int ancast_fini(ancast_ctx* ctx);

int ancast_init(ancast_ctx* ctx, const char* path)
{
    if(!ctx || !path) return -1;
    memset(ctx, 0, sizeof(ancast_ctx));

    ctx->path = path;
    ctx->file = fopen(path, "rb");
    if(!ctx->file) {
        printf("ancast: failed to open %s (%d).\n", path, errno);
        return errno;
    }

    fseek(ctx->file, 0, SEEK_END);
    ctx->size = ftell(ctx->file);
    fseek(ctx->file, 0, SEEK_SET);

    u8 buffer[0x200] = {0};
    fread(buffer, min(sizeof(buffer), ctx->size), 1, ctx->file);
    fseek(ctx->file, 0, SEEK_SET);

    u32 magic = read32((u32) buffer);
    if(magic != ANCAST_MAGIC) {
        printf("ancast: %s is not an ancast image (magic is 0x%08lX, expected 0x%08lX).\n", path, magic, ANCAST_MAGIC);
        return -2;
    }

    u32 sig_offset = read32((u32) &buffer[0x08]);
    u32 sig_type = read32((u32) &buffer[sig_offset]);

    u32 header_offset = 0;
    switch(sig_type) {
        case 0x01:
            header_offset = 0xA0;
            break;
        case 0x02:
            header_offset = 0x1A0;
            break;
        default:
            printf("ancast: %s has unrecognized signature type 0x%02lX.\n", path, sig_type);
            return -3;
    }

    ctx->header_size = header_offset + sizeof(ancast_header);
    memcpy(&ctx->header, &buffer[header_offset], sizeof(ancast_header));

    return 0;
}

int ancast_load(ancast_ctx* ctx)
{
    if(!ctx) return -1;

    u8 target = ctx->header.device >> 4;
    u8 type = ctx->header.device & 0x0F;

    switch(target) {
        case ANCAST_TARGET_PPC:
            switch(type) {
                case 0x01:
                    ctx->load = (void*)0x08000000;
                    break;

                case 0x03:
                    ctx->load = (void*)0x01330000;
                    break;
            }
            break;

        case ANCAST_TARGET_IOP:
            ctx->load = (void*)0x01000000;
            break;

        default: break;
    }

    if(!ctx->load) {
        printf("ancast: unknown load address for %s (device 0x%02lX).\n", ctx->path, ctx->header.device);
        ancast_fini(ctx);
        return -2;
    }

    ctx->body = ctx->load + ctx->header_size;

    fseek(ctx->file, 0, SEEK_SET);
    int count = fread(ctx->load, ctx->header_size + ctx->header.body_size, 1, ctx->file);
    if(count != 1) {
        printf("ancast: failed to read %s (%d).\n", ctx->path, errno);
        ancast_fini(ctx);
        return errno;
    }

    u32 hash[SHA_HASH_WORDS] = {0};
    sha_hash(ctx->body, hash, ctx->header.body_size);

    u32* h1 = ctx->header.body_hash;
    u32* h2 = hash;
    if(memcmp(h1, h2, SHA_HASH_SIZE) != 0) {
        printf("ancast: body hash check failed.\n");
        printf("        expected:   %08lX%08lX%08lX%08lX%08lX\n", h1[0], h1[1], h1[2], h1[3], h1[4]);
        printf("        calculated: %08lX%08lX%08lX%08lX%08lX\n", h2[0], h2[1], h2[2], h2[3], h2[4]);
        return -3;
    }

    return 0;
}

int ancast_fini(ancast_ctx* ctx)
{
    int res = fclose(ctx->file);
    if(res) {
        printf("ancast: failed to close %s (%d).\n", ctx->path, res);
        return res;
    }

    memset(ctx, 0, sizeof(ancast_ctx));
    return 0;
}

typedef struct {
    u32 header_size;
    u32 loader_size;
    u32 elf_size;
    u32 ddr_init;
} ios_header;

u32 ancast_iop_load(const char* path)
{
    int res = 0;
    ancast_ctx ctx = {0};

    res = ancast_init(&ctx, path);
    if(res) return 0;

    u8 target = ctx.header.device >> 4;
    if(target != ANCAST_TARGET_IOP) {
        printf("ancast: %s is not an IOP image (target is 0x%02X, expected 0x%02X).\n", path, target, ANCAST_TARGET_IOP);
        ancast_fini(&ctx);
        return 0;
    }

    res = ancast_load(&ctx);
    if(res) return 0;

    if(!(ctx.header.unk1 & 0b1)) {
        aes_reset();
        aes_set_key(otp.fw_ancast_key);

        static const u8 iv[16] = {0x91, 0xC9, 0xD0, 0x08, 0x31, 0x28, 0x51, 0xEF,
                                  0x6B, 0x22, 0x8B, 0xF1, 0x4B, 0xAD, 0x43, 0x22};
        aes_set_iv((u8*)iv);

        printf("ancast: decrypting %s...\n", path);
        aes_decrypt(ctx.body, ctx.body, ctx.header.body_size / 0x10, 0);
    }

    ios_header* header = ctx.body;
    u32 vector = (u32) ctx.body + header->header_size;

    res = ancast_fini(&ctx);
    if(res) return 0;

    return vector;
}

u32 ancast_ppc_load(const char* path)
{
    int res = 0;
    ancast_ctx ctx = {0};

    res = ancast_init(&ctx, path);
    if(res) return 0;

    u8 target = ctx.header.device >> 4;
    if(target != ANCAST_TARGET_PPC) {
        printf("ancast: %s is not a PPC image (target is 0x%02X, expected 0x%02X).\n", path, target, ANCAST_TARGET_PPC);
        ancast_fini(&ctx);
        return 0;
    }

    res = ancast_load(&ctx);
    if(res) return 0;

    dc_flushrange(ctx.load, ctx.header_size + ctx.header.body_size);
    u32 vector = (u32) ctx.body;

    res = ancast_fini(&ctx);
    if(res) return 0;

    return vector;
}
