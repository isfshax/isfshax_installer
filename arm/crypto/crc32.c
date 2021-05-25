/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2021          rw-r-r-0644
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include "common/types.h"
#include "crypto/crc32.h"

static u32 crc32_table[0x100];

u32 crc32_compute(u8 *buf, u32 len)
{
    u32 c = 0xffffffff;
    int n;

    for (n = 0; n < len; n++) {
        c = crc32_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
    }

    return c ^ 0xffffffff;
}

void crc32_make_table(void)
{
    u32 n, c, k;

    for (n = 0; n < 0x100; n++) {
        c = n;
        for (k = 0; k < 8; k++) {
            c = ((c & 1) ? 0xedb88320 : 0) ^ (c >> 1);
        }
        crc32_table[n] = c;
    }
}
