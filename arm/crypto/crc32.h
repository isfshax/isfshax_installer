/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2021          rw-r-r-0644
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef __CRC32_H__
#define __CRC32_H__

#include "common/types.h"

u32 crc32_compute(u8 *buf, u32 len);

void crc32_make_table(void);

#endif
