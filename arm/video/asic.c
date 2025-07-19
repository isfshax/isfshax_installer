/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2023          Max Thomas <mtinc2@gmail.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include "asic.h"

#include "common/utils.h"
#include "system/latte.h"

u32 abif_gpu_read32(u32 offset)
{
    write32(LT_ABIF_OFFSET, (offset & 0xFFFF) | 0xC0000000);
    return read32(LT_ABIF_DATA);
}

void abif_gpu_write32(u32 offset, u32 value32)
{
    write32(LT_ABIF_OFFSET, (offset & 0xFFFF) | 0xC0000000);
    write32(LT_ABIF_DATA, value32);
}

void abif_gpu_mask32(u32 offset, u32 clear32, u32 set32)
{
    u32 val = abif_gpu_read32(offset);
    val &= ~clear32;
    val |= set32;
    abif_gpu_write32(offset, val);
}