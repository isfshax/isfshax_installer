/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2021          rw-r-r-0644 <rwrr0644@gmail.com>
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  Copyright (C) 2008, 2009    Haxx Enterprises <bushing@gmail.com>
 *  Copyright (C) 2008, 2009    Sven Peter <svenpeter@gmail.com>
 *  Copyright (C) 2008, 2009    Hector Martin "marcan" <marcan@marcansoft.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include "common/types.h"
#include "crypto/crypto.h"
#include "crypto/crc32.h"
#include "crypto/aes.h"
#include "boot1.h"

typedef struct
{
    struct PACKED
    {
        u16 version;
        u16 sector;
        u8 empty[8];
    } params;
    u32 checksum;;
} PACKED boot1_params_t;

_Static_assert(sizeof(boot1_params_t) == 0x10, "boot1_params_t size must be 0x10!");

int boot1_get_version(void)
{
    static boot1_params_t boot1_params[2] ALIGNED(0x10);

    /* decrypt boot1 params */
    aes_reset();
    aes_set_key(otp.seeprom_key);
    aes_empty_iv();
    aes_decrypt((u8 *)&seeprom.boot1_params, (u8 *)boot1_params, 2, 0);

    /* find newest installed boot1 version */
    int version = -1;
    for (int i = 0; i < 2; i++) {
        if (boot1_params[i].checksum != crc32_compute((u8 *)&boot1_params[i].params, sizeof(boot1_params[i].params)))
            continue;

        if (version < boot1_params[i].params.version)
            version = boot1_params[i].params.version;
    }

    return version;
}
