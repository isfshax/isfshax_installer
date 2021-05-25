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
#include "memory.h"
#include "system/ancast.h"
#include "latte.h"
#include "common/utils.h"

#include <string.h>
#include <stdio.h>

void ppc_hang(void)
{
    clear32(LT_RESETS_COMPAT, 0x230);
    udelay(100);
}

void ppc_reset(void)
{
    ppc_hang();

    mask32(LT_COMPAT_MEMCTRL_STATE, 0xFFE0022F, 0x8100000);

    clear32(LT_60XE_CFG, 8);
    mask32(LT_60XE_CFG, 0x1C0000, 0x20000);
    set32(LT_60XE_CFG, 0xC000);

    set32(LT_RESETS_COMPAT, 0x200);
    clear32(LT_COMPAT_MEMCTRL_STATE, 0x20);

    set32(LT_RESETS_COMPAT, 0x30);
}

void* ppc_race(void)
{
    ppc_hang();

    u32* body = (void*) ancast_ppc_load("slc:/sys/title/00050010/1000400a/code/kernel.img");
    if(!body) {
        printf("PPC: failed to load signed image.\n");
        return NULL;
    }

    u32* rom_state = (u32*) 0x016FFFE0;
    memset(rom_state, 0, 0x20);
    dc_flushrange(rom_state, 0x20);

    u32 old = *body;
    ppc_reset();

    u8 exit_code = 0x00;
    do {
        dc_invalidaterange(body, sizeof(u32));
        dc_invalidaterange(rom_state, 0x20);
        exit_code = rom_state[7] >> 24;
    } while(old == *body && exit_code == 0x00);

    if(exit_code != 0x00 && old == *body) {
        printf("PPC: ROM failure: 0x%08lX.\n", rom_state[7]);
        return NULL;
    }

    return body;
}

void ppc_jump_stub(u32 location, u32 entry)
{
    size_t i = 0;

    // lis r3, entry@h
    write32(location + i, 0x3C600000 | entry >> 16); i += sizeof(u32);
    // ori r3, r3, entry@l
    write32(location + i, 0x60630000 | (entry & 0xFFFF)); i += sizeof(u32);
    // mtsrr0 r3
    write32(location + i, 0x7C7A03A6); i += sizeof(u32);
    // li r3, 0
    write32(location + i, 0x38600000); i += sizeof(u32);
    // mtsrr1 r3
    write32(location + i, 0x7C7B03A6); i += sizeof(u32);
    // rfi
    write32(location + i, 0x4C000064); i += sizeof(u32);

    dc_flushrange((void*)location, i);
}

void ppc_wait_stub(u32 location, u32 entry)
{
    size_t i = 0;

    // lis r3, entry@h
    write32(location + i, 0x3C600000 | entry >> 16); i += sizeof(u32);
    // ori r3, r3, entry@l
    write32(location + i, 0x60630000 | (entry & 0xFFFF)); i += sizeof(u32);

    // li r4, 0
    write32(location + i, 0x38800000); i += sizeof(u32);
    // stw r4, 0(r3)
    write32(location + i, 0x90830000); i += sizeof(u32);
    // dcbf r0, r3
    write32(location + i, 0x7C0018AC); i += sizeof(u32);
    // sync
    write32(location + i, 0x7C0004AC); i += sizeof(u32);

// _wait:
    // dcbi r0, r3
    write32(location + i, 0x7C001BAC); i += sizeof(u32);
    // sync
    write32(location + i, 0x7C0004AC); i += sizeof(u32);
    // lwz r4, 0(r3)
    write32(location + i, 0x80830000); i += sizeof(u32);
    // cmpwi cr7, r4, 0
    write32(location + i, 0x2F840000); i += sizeof(u32);
    // beq cr7, _wait
    write32(location + i, 0x419EFFF0); i += sizeof(u32);

    // mtsrr0 r4
    write32(location + i, 0x7C9A03A6); i += sizeof(u32);
    // li r4, 0
    write32(location + i, 0x38800000); i += sizeof(u32);
    // mtsrr1 r4
    write32(location + i, 0x7C9B03A6); i += sizeof(u32);
    // rfi
    write32(location + i, 0x4C000064); i += sizeof(u32);

    dc_flushrange((void*)location, i);
}

static bool ready = false;

void ppc_prepare(void)
{
    if(ready) {
        printf("PPC: PowerPC is already prepared!\n");
        return;
    }

    // Boot the PowerPC and race the ROM.
    u32 start = (u32) ppc_race();
    if(start == 0) return;

    // The wait stub clears this to zero before entering the wait loop.
    // Set a different value first, so we know when it enters the loop.
    write32(0x14000000, 0xFFFFFFFF);
    dc_flushrange((void*)0x14000000, sizeof(u32));

    // Copy the wait stub to MEM2. This waits for us to load further code.
    ppc_wait_stub(0x14000100, 0x14000000);
    // Copy the jump stub to the start of the ancast body. This jumps to the wait stub.
    ppc_jump_stub(start, 0x14000100);

    // Wait for the PowerPC to enter the wait loop.
    while(true) {
        dc_invalidaterange((void*)0x14000000, sizeof(u32));
        if(read32(0x14000000) == 0) break;
    }

    printf("PPC: PowerPC is waiting for entry!\n");
    ready = true;
}

void ppc_jump(u32 entry)
{
    if(!ready) {
        printf("PPC: Jump requested but PowerPC not ready!\n");
        return;
    }

    // Write the PowerPC entry point.
    write32(0x14000000, entry);
    dc_flushrange((void*)0x14000000, sizeof(u32));

    ready = false;
}
