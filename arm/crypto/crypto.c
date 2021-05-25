/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  Copyright (C) 2008, 2009    Haxx Enterprises <bushing@gmail.com>
 *  Copyright (C) 2008, 2009    Sven Peter <svenpeter@gmail.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include "crypto/crypto.h"
#include "crypto/seeprom.h"
#include "crypto/aes.h"
#include "crypto/crc32.h"
#include "system/latte.h"
#include "common/utils.h"
#include "system/memory.h"
#include "system/irq.h"
#include "video/gfx.h"
#include "string.h"

otp_t otp;
seeprom_t seeprom;

void crypto_read_otp(void)
{
    u32 *otpd = (u32*)&otp;
    int word, bank;
    for (bank = 0; bank < 8; bank++)
    {
        for (word = 0; word < 0x20; word++)
        {
            write32(LT_OTPCMD, 0x80000000 | bank << 8 | word);
            *otpd++ = read32(LT_OTPDATA);
        }
    }
}

void crypto_read_seeprom(void)
{
    seeprom_read(&seeprom, 0, sizeof(seeprom) / 2);
}

void crypto_initialize(void)
{
    crypto_read_otp();
    crypto_read_seeprom();
    crc32_make_table();
}
